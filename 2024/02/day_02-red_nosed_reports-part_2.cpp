#include <algorithm>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

std::vector<std::vector<int>> getInput()
{
    std::vector<std::vector<int>> grid;
    std::string                   line;
    for (std::ifstream input("day_02-red_nosed_reports-input.txt");
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

std::pair<int, int> findUnsafe(const std::vector<int>& levels, int drop)
{
    assert(levels.size() >= 2);

    int l = 0;
    l += (l == drop);
    int r = l + 1;
    r += (r == drop);
    int minDiff = 1, maxDiff = 3;
    if (levels[l] > levels[r])
        minDiff = -3, maxDiff = -1;
    do {
        const int diff = levels[r] - levels[l];
        if ((diff < minDiff) || (diff > maxDiff))
            return std::make_pair(l, r);

        l = r++;
        r += (r == drop);
    } while (r < levels.size());
    return std::make_pair(-1, -1);
}

bool isSafe(const std::vector<int>& levels)
{
    assert(levels.size() >= 2);

    const auto [bad1, bad2] = findUnsafe(levels, -1);
    return ((bad1 < 0)                           ||
            (findUnsafe(levels, bad1).first < 0) ||
            (findUnsafe(levels, bad2).first < 0)) ||
           ((bad1 != 0)                       &&
            (findUnsafe(levels, 0).first < 0) ||
            ((bad1 != 1) && (findUnsafe(levels, 1).first < 0)));
}

int main()
{
    const auto grid    = getInput();
    const auto numSafe = std::count_if(grid.begin(), grid.end(), &isSafe);
    std::cout << numSafe << std::endl;
    return 0;
}
