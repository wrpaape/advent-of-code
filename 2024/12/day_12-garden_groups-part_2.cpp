#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unordered_map>
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
// | X | Y - Y |
// +   |   +---+
// | X | Y | Z |
//
constexpr int k_Dir[] = { 0, +1, 0, -1, 0 };

void visitGarden(std::uint64_t&                  area,
                 std::uint64_t&                  numCorners,
                 std::vector<std::vector<bool>>& visited,
                 const std::vector<std::string>& grid,
                 int                             i,
                 int                             j)
{
    visited[i][j] = true;
    ++area;
    const int m = grid.size(), n = grid[0].size();
    const auto isMatch = [&](int nI, int nJ) {
        return (nI >= 0) && (nI < m) &&
               (nJ >= 0) && (nJ < n) &&
               (grid[nI][nJ] == grid[i][j]);
    };
    bool haveObs[5] = { false }, haveMatch[5] = { false };
    for (int d = 0; d < 4; ++d) {
        if (const int nI = i+k_Dir[d], nJ = j+k_Dir[d+1]; isMatch(nI, nJ)) {
            haveMatch[d] = true;
            if (!visited[nI][nJ])
                visitGarden(area, numCorners, visited, grid, nI, nJ);
        }
        else {
            haveObs[d] = true;
        }
    }
    haveObs[  4] = haveObs[  0];
    haveMatch[4] = haveMatch[0];
    for (int d = 0; d < 4; ++d) {
        numCorners += (haveObs[d] & haveObs[d+1]);
        if (haveMatch[d] && haveMatch[d+1]) {
            const int dX = (d <= 1) ? +1 : -1,
                      dY = ((d == 0) || (d == 3)) ? +1 : -1;
            const int dI = i + dX, dJ = j + dY;
            numCorners += !isMatch(dI, dJ);
        }
    }
}
// 0 R => U,D
// 1 D => L,R
// 2 L => U,D
// 3 U => L,R
//
// d = 0 => (D,R) => (+1, +1)
// d = 1 => (D,L) => (+1, -1)
// d = 2 => (U,L) => (-1, -1)
// d = 3 => (U,R) => (-1, +1)
//
// +--p1--  p1
// |  
// p2   
// |
// 
// //
//
// +-x-+-x-+-x-+
// x o x o x o x
// +-x-+-x-+-x-+
// x o x o x o x
// +-x-+-x-+-x-+
//
// R R R
// RRRRR
//
//  P   P
// P3P3P3P
// P22122P
//  PPPPP
//   o
//   o
// o14oo
//   1 o
//   ooo


// 2---2
// |   2--2
// 2------2

int main()
{
    const std::vector<std::string> grid = getInput();
    const int m = grid.size(), n = grid[0].size();
    std::vector<std::vector<bool>> visited(m, std::vector<bool>(n));
    std::uint64_t total = 0;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (!visited[i][j]) {
                std::uint64_t area = 0, numCorners = 0;
                visitGarden(area, numCorners, visited, grid, i, j);
                const auto numSides = numCorners;
                total += (area * numSides);
            }
        }
    }
    std::cout << total << std::endl;
    return 0;
}
