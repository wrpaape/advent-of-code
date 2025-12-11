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
  std::unordered_map<std::string, std::int64_t> numPaths;
  const auto dfs = [&](this auto&& dfs, const std::string& u) -> std::int64_t {
    if (u == dst)
      return 1;
    const auto [it, newEntry] = numPaths.emplace(u, -1);
    if (!newEntry) {
      assert(it->second >= 0);
      return it->second;
    }
    std::int64_t numP = 0;
    if (const auto edges = graph.find(u); edges != graph.end())
      for (const std::string& v : edges->second)
        numP += dfs(v);
    return numPaths[u] = numP;
  };
  return dfs(src);
}

std::int64_t
solve(const std::unordered_map<std::string, std::vector<std::string>> &input) {
  return countPaths(input, "you", "out");
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
      R"(aaa: you hhh
you: bbb ccc
bbb: ddd eee
ccc: ddd eee fff
ddd: ggg
eee: out
fff: out
ggg: out
hhh: ccc fff iii
iii: out
)";
  check(__LINE__, 5, example);
}

int main() {
  runTests();

  const auto result = solve(getInput());
  std::cout << result << std::endl;
  return 0;
}
