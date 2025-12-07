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
#include <numeric>
#include <ranges>
#include <span>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

auto getInput(std::istream& stream)
{
    std::vector<std::string> input;
    for (std::string line; std::getline(stream, line); )
        input.push_back(std::move(line));
    return input;
}

auto getInput()
{
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
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

auto getInput(std::string_view input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::int64_t solve(std::span<const std::string> input)
{
    assert(!input.empty());
    const auto s = input[0].find('S');
    assert(s != std::string::npos);
    const std::int64_t m = input.size(), n = input[0].size();
    std::vector<std::int64_t> pVisited(n), nVisited(n);
    nVisited[s] = 1;
    for (std::int64_t i = 1; i < m; ++i) {
        assert(input[i].size() == std::size_t(n));
        std::swap(pVisited, nVisited);
        std::ranges::fill(nVisited, 0);
        for (std::int64_t j = 0; j < n; ++j) if (pVisited[j]) {
            if (input[i][j] == '^') {
                nVisited[j-1] += pVisited[j];
                nVisited[j+1] += pVisited[j];
            }
            else {
                nVisited[j] += pVisited[j];
            }
        }
    }
    return std::reduce(nVisited.begin(), nVisited.end());
}

void check(int lineNumber, std::int64_t expectedTotal, std::string_view source)

{
    if (const auto total = solve(getInput(source)); total != expectedTotal) {
        std::cerr
            << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                           lineNumber, expectedTotal, total)
            << std::endl;
    }
}

void runTests()
{
    const std::string_view example =
R"(.......S.......
...............
.......^.......
...............
......^.^......
...............
.....^.^.^.....
...............
....^.^...^....
...............
...^.^...^.^...
...............
..^...^.....^..
...............
.^.^.^.^.^...^.
...............
)";
    check(__LINE__, 40, example);
}

int main()
{
    runTests();

    const auto total = solve(getInput());
    std::cout << total << std::endl;
    return 0;
}
