#include <algorithm>
#include <bits/ranges_algo.h>
#include <cassert>
#include <charconv>
#include <coin/CoinFinite.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <span>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include <CbcModel.hpp>
#include <CoinModel.hpp>
#include <OsiClpSolverInterface.hpp>

void apply(std::span<std::int64_t>       joltages,
           std::span<const std::int64_t> buttons,
           std::int64_t                  delta)
{
  for (const auto b : buttons)
    joltages[b] += delta;
}

bool isOver(std::span<const std::int64_t> joltages,
            std::span<const std::int64_t> limits)
{
  assert(limits.size() == joltages.size());

  for (std::size_t i = 0; i < limits.size(); ++i)
    if (joltages[i] > limits[i])
      return true;
  return false;
}

std::ostream &operator<<(std::ostream &stream,
                         const std::vector<std::int64_t> &joltages) {
  const char *delim = "";
  for (const auto j : joltages)
    stream << delim, stream << j, delim = ",";
  return stream;
}

namespace std {
  template<>
  struct hash<std::vector<std::int64_t>> {
    std::size_t operator()(const std::vector<std::int64_t>& joltages) const {
      const char *const j = reinterpret_cast<const char *>(joltages.data()),
                        *const jEnd = j + joltages.size();
      const std::string_view str(j, jEnd);
      return hash<std::string_view>{}(str);
    }
  };
};

struct Machine {
  std::string                            d_indicators;
  std::vector<std::vector<std::int64_t>> d_buttons;
  std::vector<std::int64_t>              d_joltages;
};

void getMachine(Machine &m, std::string_view line) {
  const auto iOpen = line.find('[', 0);
  assert(iOpen != std::string::npos);
  const auto iClose = line.find(']', iOpen + 1);
  assert(iClose != std::string::npos);
  m.d_indicators = std::string(line.data() + iOpen + 1, line.data() + iClose);
  const auto parseInts = [](std::vector<std::int64_t> &ints,
                            std::string_view input) {
    for (const char *p = input.data(), *const pEnd = p + input.size();
         p != pEnd;) {
      auto [nP, err] = std::from_chars(p, pEnd, ints.emplace_back());
      assert(err == std::errc());
      p = nP + (nP != pEnd);
    }
  };
  const auto jClose = line.rfind('}', std::string::npos);
  assert(jClose != std::string::npos);
  assert(jClose != 0);
  const auto jOpen = line.rfind('{', jClose - 1);
  assert(jOpen != std::string::npos);
  const std::string_view joltages(line.data() + jOpen + 1,
                                  line.data() + jClose);
  parseInts(m.d_joltages, joltages);
  for (std::size_t i = iClose;; ++i) {
    i = line.find('(', i + 1);
    if (i >= jOpen)
      break;
    const auto j = line.find(')', i + 1);
    assert(j != std::string::npos);
    const std::string_view buttons(line.data() + i + 1, line.data() + j);
    parseInts(m.d_buttons.emplace_back(), buttons);
    std::ranges::sort(m.d_buttons.back());
    i = j;
  }
  assert(m.d_indicators.size() == m.d_joltages.size());
}

auto getInput(std::istream &stream) {
  std::vector<Machine> input;
  for (std::string line; std::getline(stream, line);)
    getMachine(input.emplace_back(), line);
  return input;
}

auto getInput() {
  for (const auto &entry : std::filesystem::directory_iterator(".")) {
    if (std::filesystem::is_regular_file(entry.path()) &&
        entry.path().string().ends_with("-input.txt")) {
      std::ifstream stream(entry.path().string());
      return getInput(stream);
    }
  }
  std::cerr << "Failed to find input file in the current directory"
            << std::endl;
  std::abort();
}

auto getInput(std::string_view input) {
  std::istringstream stream{std::string(input)};
  return getInput(stream);
}

std::int64_t configure(const Machine &m) {

// [.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}

// 0001*b1 + 0101*b2 + 0010*b3 + 0011*b4 + 1010*b5 + 1100*b6 = 3547
// minimize: b1 + b2 + b3 + b4 + b5 + b6
//
// subject to: 0*b1 + 0*b2 + 0*b3 + 0*b4 + 1*b5 + 1*b6 = 3
//             0*b1 + 1*b2 + 0*b3 + 0*b4 + 0*b5 + 1*b6 = 5
//             0*b1 + 0*b2 + 1*b3 + 1*b4 + 1*b5 + 0*b6 = 4
//             1*b1 + 1*b2 + 0*b3 + 1*b4 + 0*b5 + 0*b6 = 7
//             b1-b5 >= 0

    // Create model
    CoinModel builder;
    // 1.0*b1 + 1.0*b2 + ...

    // Number of rows (constraints) and columns (variables)
    const int numRows = m.d_joltages.size();
    const int numCols = m.d_buttons.size();
    for (int i = 0; i < numRows; ++i)
      builder.addRow(0, nullptr, nullptr, m.d_joltages[i], m.d_joltages[i]);
    for (int j = 0; j < numCols; ++j) {
      std::vector<int> indices(numRows);
      std::ranges::iota(indices, 0);
      std::vector<double> values(numRows);
      for (int i = 0; i < numRows; ++i)
        values[i] = std::ranges::binary_search(m.d_buttons[j], i) ? 1.0 : 0.0;
      builder.addColumn(numRows, indices.data(), values.data(), 0.0, COIN_DBL_MAX, 1.0, NULL, true);
    }

    OsiClpSolverInterface solver;
    solver.loadFromCoinModel(builder);
    CbcModel model(solver);
    model.branchAndBound();

    const double* sol = model.bestSolution();
    std::int64_t sum = 0;
    std::cout << "    ";
    for (int j = 0; j < numCols; ++j) {
      sum += std::round(sol[j]);
    }
    return sum;
}

std::int64_t solve(std::span<const Machine> input) {
  std::int64_t sum = 0;
  for (const auto &m : input) {
    sum += configure(m);
  }
  return sum;
}

void check(int lineNumber, std::int64_t expectedPresses, const Machine &m) {
  if (const auto numPresses = configure(m); numPresses != expectedPresses) {
    std::cerr << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                             lineNumber, expectedPresses, numPresses)
              << std::endl;
  }
}

void check(int lineNumber, std::int64_t expectedSum, std::string_view source) {
  if (const auto sum = solve(getInput(source)); sum != expectedSum) {
    std::cerr << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                             lineNumber, expectedSum, sum)
              << std::endl;
  }
}

void runTests() {
  const std::string_view example =
R"([.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}
[...#.] (0,2,3,4) (2,3) (0,4) (0,1,2) (1,2,3,4) {7,5,12,7,2}
[.###.#] (0,1,2,3,4) (0,3,4) (0,1,2,4,5) (1,2) {10,11,11,5,10,5}
)";
  check(__LINE__, 10, {".##.", {{3},{1,3},{2},{2,3},{0,2},{0,1}}, {3,5,4,7}});
  check(__LINE__, 12, {"...#.", {{0,2,3,4},{2,3},{0,4},{0,1,2},{1,2,3,4}}, {7,5,12,7,2}});
  check(__LINE__, 11, {".###.#", {{0,1,2,3,4},{0,3,4},{0,1,2,4,5},{1,2}}, {10,11,11,5,10,5}});
  check(__LINE__, 33, example);
}

int main() {
  runTests();

  const auto sum = solve(getInput());
  std::cout << "sum: " << sum << std::endl;
  return 0;
}
