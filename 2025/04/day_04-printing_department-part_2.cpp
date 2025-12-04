#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
#include <queue>
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
    std::ifstream stream("day_04-printing_department-input.txt");
    return getInput(stream);
}

auto getInput(std::string_view input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::string joinLines(std::span<const std::string> lines)
{
    std::string result;
    for (const std::string& line : lines)
        result += line, result += '\n';
    return result;
}

std::pair<std::string, std::int64_t> solve(std::span<const std::string> grid)
{
    const std::int64_t m = grid.size(), n = grid.empty() ? 0 : grid[0].size();
    std::vector<std::string> marked(grid.begin(), grid.end()),
                             counts(m, std::string(n, '\0'));
    std::queue<std::array<std::int64_t, 2>> frontier;
    for (std::int64_t ii = 0; ii < m; ++ii) {
        for (std::int64_t jj = 0; jj < n; ++jj) {
            if (grid[ii][jj] == '@') {
                std::int64_t cnt = 0;
                for (std::int64_t
                     iMin = std::max(0L, ii-1), iMax = std::min(n-1, ii+1),
                     jMin = std::max(0L, jj-1), jMax = std::min(n-1, jj+1),
                     i = iMin; i <= iMax; ++i)
                    for (std::int64_t j = jMin; j <= jMax; ++j)
                        if (((i != ii) || (j != jj)) && (grid[i][j] == '@'))
                            ++cnt;
                counts[ii][jj] = cnt;
                if (cnt < 4)
                    frontier.push({ii, jj});
            }
        }
    }
    std::int64_t numRolls = 0;
    while (!frontier.empty()) {
        const auto [ii, jj] = frontier.front();
        frontier.pop();
        marked[ii][jj] = '.';
        --counts[ii][jj];
        ++numRolls;
        for (std::int64_t
             iMin = std::max(0L, ii-1), iMax = std::min(n-1, ii+1),
             jMin = std::max(0L, jj-1), jMax = std::min(n-1, jj+1),
             i = iMin; i <= iMax; ++i)
            for (std::int64_t j = jMin; j <= jMax; ++j)
                if (((i != ii) || (j != jj)) && (grid[i][j] == '@') &&
                    (counts[i][j]-- == 4))
                    frontier.push({i, j});
    }
    return { joinLines(marked), numRolls };
}

void check(int              lineNumber,
           std::string_view expectedMarks,
           std::int64_t     expectedRolls,
           std::string_view source)
{
    const auto [marks, numRolls] = solve(getInput(source));
    if (marks != expectedMarks)
        std::cerr
            << std::format("failure({}):\n> expected:\n{}\n> actual:\n{}\n",
                           lineNumber, expectedMarks, marks)
            << std::endl;
    if (numRolls != expectedRolls)
        std::cerr
            << std::format("failure({}):\n> expected: {}, \n> actual:   {}\n",
                           lineNumber, expectedRolls, numRolls)
            << std::endl;
}

void runTests()
{
    check(
        __LINE__,
        "..........\n"
        "..........\n"
        "..........\n"
        "....@@....\n"
        "...@@@@...\n"
        "...@@@@@..\n"
        "...@.@.@@.\n"
        "...@@.@@@.\n"
        "...@@@@@..\n"
        "....@@@...\n",
        43,
        "..@@.@@@@.\n"
        "@@@.@.@.@@\n"
        "@@@@@.@.@@\n"
        "@.@@@@..@.\n"
        "@@.@@@@.@@\n"
        ".@@@@@@@.@\n"
        ".@.@.@.@@@\n"
        "@.@@@.@@@@\n"
        ".@@@@@@@@.\n"
        "@.@.@@@.@.\n"
    );
}

int main()
{
    runTests();

    const auto [_marks, numRolls] = solve(getInput());
    std::cout << numRolls << std::endl;
    return 0;
}
