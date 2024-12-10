#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
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

const std::vector<std::pair<int, int>>&
score(std::vector<std::vector<bool>>&                             visited,
      std::vector<std::vector<std::vector<std::pair<int, int>>>>& peaks,
      const std::vector<std::string>&                             grid,
      int                                                         i,
      int                                                         j)
{
    auto& currPeaks = peaks[i][j];
    if (visited[i][j])
        return currPeaks;

    visited[i][j] = true;
    if (grid[i][j] == '9') {
        currPeaks.emplace_back(i, j);
        return currPeaks;
    }
    std::vector<std::pair<int, int>> prevPeaks;
    constexpr int k_Dir[] = { 0, +1, 0, -1, 0 };
    for (int k = 0; k < 4; ++k) {
        if (const int nI = i+k_Dir[k], nJ = j+k_Dir[k+1];
            (nI >= 0) && (nI < grid.size()) &&
            (nJ >= 0) && (nJ < grid[nI].size()) &&
            (grid[nI][nJ] == (grid[i][j]+1))) {
            const auto& nextPeaks = score(visited, peaks, grid, nI, nJ);
            std::swap(currPeaks, prevPeaks);
            currPeaks.clear();
            std::merge(prevPeaks.begin(), prevPeaks.end(),
                       nextPeaks.begin(), nextPeaks.end(),
                       std::back_inserter(currPeaks));
        }
    }
    currPeaks.erase(std::unique(currPeaks.begin(), currPeaks.end()),
                    currPeaks.end());
    return currPeaks;
}

int main()
{
    std::vector<std::string> grid = getInput();
    const int k_NumRows = grid.size(), k_NumCols = grid.empty() ? 0 : grid[0].size();
    std::vector<std::vector<std::vector<std::pair<int, int>>>> peaks(
                     k_NumRows,
                     std::vector<std::vector<std::pair<int, int>>>(k_NumCols));
    std::vector<std::vector<bool>> visited(k_NumRows,
                                           std::vector<bool>(k_NumCols));
    std::int64_t total = 0;
    for (int i = 0; i < k_NumRows; ++i)
        for (int j = 0; j < k_NumCols; ++j)
            if (grid[i][j] == '0')
                total += score(visited, peaks, grid, i, j).size();

    std::cout << total << std::endl;
    return 0;
}
