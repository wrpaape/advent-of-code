#include <algorithm>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::vector<int>> getInput()
{
    std::vector<std::vector<int>> grid;
    std::string                   line;
    for (std::ifstream input("day_02-red_noded_reports-input.txt");
         std::getline(input, line); ) {
        grid.emplace_back();
        for (const char *cursor  = line.c_str(),
                  *const lineEnd = cursor + line.size(); cursor != lineEnd; ) {
            const auto [ptr, errc] = std::from_chars(cursor, lineEnd,
                                                     grid.back().emplace_back());
            assert(errc == std::errc());
            for (cursor = ptr; isspace(*cursor); ++cursor)
                continue;
        }
        assert(grid.back().size() >= 2);
    }
    return grid;
}

bool isSafe(const std::vector<int>& levels)
{
    assert(levels.size() >= 2);

    int minDiff = 1, maxDiff = 3;
    if (levels[1] < levels[0])
        minDiff = -3, maxDiff = -1;
    return std::adjacent_find(levels.begin(), levels.end(),
        [=](int lhs, int rhs) {
            const int diff = rhs - lhs;
            return (diff < minDiff) || (diff > maxDiff);
        }) == levels.end();
}

int main()
{
    const auto grid    = getInput();
    const auto numSafe = std::count_if(grid.begin(), grid.end(), &isSafe);
    std::cout << numSafe << std::endl;
    return 0;
}
