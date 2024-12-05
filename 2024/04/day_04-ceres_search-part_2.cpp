#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> getInput()
{
    std::vector<std::string> grid;
    for (std::ifstream input("day_04-ceres_search-input.txt");
         std::getline(input, grid.emplace_back()); )
        continue;

    assert(!grid.empty());
    grid.pop_back();
    assert(!grid.empty());
    return grid;
}

bool isMatch(const std::vector<std::string>&  grid,
             const char                      *word,
             int i, int j, int x, int y)
{
    while ((i >= 0) && (i < grid.size()) &&
           (j >= 0) && (j < grid[i].size()) &&
           (*word == grid[i][j])) {
        ++word;
        i += x;
        j += y;
    }
    return *word == '\0';
}

int main()
{
    const std::vector<std::string> grid = getInput();
    const int k_NumRows = grid.size(), k_NumCols = grid[0].size();
    std::vector<std::vector<char>> mids(k_NumRows,
                                        std::vector<char>(k_NumCols, 0));
    for (int i = 0; i < k_NumRows; ++i)
        for (int j = 0; j < k_NumCols; ++j)
            for (int x = -1; x <= +1; x += 2)
                for (int y = -1; y <= +1; y += 2)
                    if (isMatch(grid, "MAS", i, j, x, y))
                        ++mids[i+x][j+y];

    int total = 0;
    for (int i = 0; i < k_NumRows; ++i)
        for (int j = 0; j < k_NumCols; ++j)
            total += (mids[i][j] == 2);

    std::cout << total << std::endl;
    return 0;
}
