#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

auto getInput()
{
    std::vector<std::string> grid;
    for (std::ifstream stream("day_12-garden_groups-input.txt");
         std::getline(stream, grid.emplace_back()); )
        continue;

    assert(!grid.empty());
    grid.pop_back();
    assert(!grid.empty());
    return grid;
}

void visitGarden(std::uint64_t&                  area,
                 std::uint64_t&                  perimeter,
                 std::vector<std::vector<int>>&  gardens,
                 int                             garden,
                 const std::vector<std::string>& grid,
                 int                             i,
                 int                             j)
{
    gardens[i][j] = garden;
    ++area;
    constexpr int k_Dir[] = { 0, +1, 0, -1, 0 };
    const int m = grid.size(), n = grid[0].size();
    for (int k = 0; k < 4; ++k) {
        if (const int nI = i+k_Dir[k], nJ = j+k_Dir[k+1];
            (nI >= 0) && (nI < m) &&
            (nJ >= 0) && (nJ < n) &&
            (grid[nI][nJ] == grid[i][j])) {
            if (gardens[nI][nJ] < 0)
                visitGarden(area, perimeter, gardens, garden, grid, nI, nJ);
        }
        else {
            ++perimeter;
        }
    }
}

int main()
{
    const std::vector<std::string> grid = getInput();
    const int m = grid.size(), n = grid[0].size();
    std::vector<std::vector<int>> gardens(m, std::vector<int>(n, -1));
    std::uint64_t total = 0;
    for (int i = 0, garden = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (gardens[i][j] < 0) {
                std::uint64_t area = 0, perimeter = 0;
                visitGarden(area, perimeter, gardens, garden++, grid, i, j);
                total += (area * perimeter);
            }
        }
    }
    std::cout << total << std::endl;
    return 0;
}
