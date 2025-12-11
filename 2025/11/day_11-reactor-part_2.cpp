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
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <vector>

struct Counts {
  std::int64_t d_noDacNoFft = -1;
  std::int64_t d_dacNoFft = -1;
  std::int64_t d_fftNoDac = -1;
  std::int64_t d_dacAndFft = -1;
};

auto getInput(std::istream &stream) {
  std::unordered_map<std::string, std::vector<std::string>> input;
  for (std::string line; std::getline(stream, line);) {
    const auto colon = line.find(':');
    assert(colon != std::string::npos);
    auto& edges = input[line.substr(0, colon)];
    for (std::size_t i = colon + 1; i < line.size(); ++i) {
      i = line.find_first_not_of(' ', i);
      assert(i != std::string::npos);
      const auto j = std::min(line.size(), line.find(' ', i+1));
      edges.push_back(line.substr(i, j-i));
      i = j;
    }
  }
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

std::int64_t countPaths(
    const std::unordered_map<std::string, std::vector<std::string>> &graph,
    const std::string &src, const std::string &dst) {
  std::unordered_map<std::string, Counts> counts;
  const bool dstIsDac = (dst == "dac"), dstIsFft = (dst == "fft");
  const Counts dstCnts = { !dstIsDac & !dstIsFft, dstIsDac, dstIsFft, false };
  const auto dfs = [&](this auto &&dfs, const std::string &u) -> Counts {
    if (u == dst)
      return dstCnts;
    if (const auto& cnts = counts[u]; cnts.d_noDacNoFft >= 0)
      return cnts;
    const bool uIsDac = (u == "dac"), uIsFft = (u == "fft");
    Counts cnts = {0, 0, 0, 0};
    if (const auto edges = graph.find(u); edges != graph.end()) {
      for (const std::string& v : edges->second) {
        const auto nCnts = dfs(v);
        if (!uIsDac & !uIsFft)
          cnts.d_noDacNoFft += nCnts.d_noDacNoFft;
        if (!uIsFft) {
          cnts.d_dacNoFft += nCnts.d_dacNoFft;
          if (uIsDac)
            cnts.d_dacNoFft += nCnts.d_noDacNoFft;
        }
        if (!uIsDac) {
          cnts.d_fftNoDac += nCnts.d_fftNoDac;
          if (uIsFft)
            cnts.d_fftNoDac += nCnts.d_noDacNoFft;
        }
        cnts.d_dacAndFft += nCnts.d_dacAndFft;
        if (uIsDac)
          cnts.d_dacAndFft += nCnts.d_fftNoDac;
        if (uIsFft)
          cnts.d_dacAndFft += nCnts.d_dacNoFft;
      }
    }
    return counts[u] = cnts;
  };
  return dfs(src).d_dacAndFft;
}

std::int64_t
solve(const std::unordered_map<std::string, std::vector<std::string>> &input) {
  return countPaths(input, "svr", "out");
}

void check(int lineNumber, std::int64_t expectMaxArea,
           std::string_view source) {
  if (const auto maxArea = solve(getInput(source)); maxArea != expectMaxArea) {
    std::cerr << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                             lineNumber, expectMaxArea, maxArea)
              << std::endl;
  }
}

void runTests() {
  const std::string_view example =
      R"(svr: aaa bbb
aaa: fft
fft: ccc
bbb: tty
tty: ccc
ccc: ddd eee
ddd: hub
hub: fff
eee: dac
dac: fff
fff: ggg hhh
ggg: out
hhh: out
)";
  check(__LINE__, 2, example);
}

int main() {
  runTests();

  const auto result = solve(getInput());
  std::cout << result << std::endl;
  return 0;
}
