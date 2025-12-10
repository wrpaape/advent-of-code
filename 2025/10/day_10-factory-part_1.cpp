#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <queue>
#include <span>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

struct Machine {
  std::string d_indicators;
  std::vector<std::vector<std::int64_t>> d_buttons;
  std::vector<std::int64_t> d_joltages;
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

std::int64_t configure(const Machine& m) {
    std::queue<std::string> frontier;
    std::unordered_set<std::string> visited;
    visited.insert(frontier.emplace(m.d_indicators.size(), '.'));
    for (std::int64_t numPresses = 0; ; ++numPresses) {
        assert(!frontier.empty());
        for (auto rem = frontier.size(); rem > 0; --rem) {
            const auto inds = std::move(frontier.front());
            frontier.pop();
            if (inds == m.d_indicators)
                return numPresses;
            std::string nInds;
            for (const auto& buts : m.d_buttons) {
                nInds = inds;
                for (const auto b : buts)
                    nInds[b] = (nInds[b] == '.') ? '#' : '.';
                if (visited.insert(nInds).second)
                    frontier.push(nInds);
            }
        }
    }
}

std::int64_t solve(std::span<const Machine> input) {
    std::int64_t sum = 0;
    for (const auto& m : input)
        sum += configure(m);
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
  check(__LINE__, 2, {".##.", {{3},{1,3},{2},{2,3},{0,2},{0,1}}, {3,5,4,7}});
  check(__LINE__, 3, {"...#.", {{0,2,3,4},{2,3},{0,4},{0,1,2},{1,2,3,4}}, {7,5,12,7,2}});
  check(__LINE__, 2, {".###.#", {{0,1,2,3,4},{0,3,4},{0,1,2,4,5},{1,2}}, {10,11,11,5,10,5}});
  check(__LINE__, 7, example);
}

int main() {
  runTests();

  const auto sum = solve(getInput());
  std::cout << sum << std::endl;
  return 0;
}
