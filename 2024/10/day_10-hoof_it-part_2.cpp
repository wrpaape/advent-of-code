#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

auto getInput()
{
    std::vector<std::string> grid;
    for (std::ifstream stream("day_10-hoof_it-input.txt");
         std::getline(stream, grid.emplace_back()); )
        continue;

    assert(!grid.empty());
    grid.pop_back();
    assert(!grid.empty());
    return grid;
}

std::int64_t score(std::vector<std::vector<std::int64_t>>& numPaths,
                   const std::vector<std::string>&         grid,
                   int                                     i,
                   int                                     j)
{
    if (numPaths[i][j] >= 0)
        return numPaths[i][j];

    const char height = grid[i][j];
    if (height == '9') {
        numPaths[i][j] = 1;
        return 1;
    }
    std::int64_t total = 0;
    constexpr int k_Dir[] = { 0, +1, 0, -1, 0 };
    for (int k = 0; k < 4; ++k)
        if (const int nI = i+k_Dir[k], nJ = j+k_Dir[k+1];
            (nI >= 0) && (nI < grid.size()) &&
            (nJ >= 0) && (nJ < grid[nI].size()) &&
            (grid[nI][nJ] == (height + 1)))
            total += score(numPaths, grid, nI, nJ);

    numPaths[i][j] = total;
    return total;
}

int main()
{
    std::vector<std::string> grid = getInput();
    const int k_NumRows = grid.size(), k_NumCols = grid.empty() ? 0 : grid[0].size();
    std::vector<std::vector<std::int64_t>> numPaths(
                                      k_NumRows,
                                      std::vector<std::int64_t>(k_NumCols, -1));
    std::int64_t total = 0;
    for (int i = 0; i < k_NumRows; ++i)
        for (int j = 0; j < k_NumCols; ++j)
            if (grid[i][j] == '0')
                total += score(numPaths, grid, i, j);

    std::cout << total << std::endl;
    return 0;
}
