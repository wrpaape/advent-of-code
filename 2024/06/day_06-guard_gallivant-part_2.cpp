#include <algorithm>
#include <cassert>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <bitset>

std::vector<std::string> getInput()
{
    std::vector<std::string> grid;
    for (std::ifstream input("day_06-guard_gallivant-input.txt");
         std::getline(input, grid.emplace_back()); )
        continue;

    if (!grid.empty())
        grid.pop_back();

    return grid;
}

enum Dir {
    e_Up,
    e_Right,
    e_Down,
    e_Left,
    e_NumDirs
};

// 0 1 2 3 4
// 1
// 2 ^
// 3

std::pair<int, int> step(int x, int y, int dir)
{
    constexpr int k_Step[] = { 0, 1, 0, -1, 0 };
    return std::make_pair(x + k_Step[dir], y - k_Step[dir+1]);
}

int turn(int dir)
{
    return (dir + 1) % 4;
}

typedef std::bitset<e_NumDirs> DirSet;

bool haveCycle(const std::vector<std::string>&    grid,
               std::vector<std::vector<DirSet>>& visited,
               int x, int y)
{
    const int k_NumRows = grid.size(),
              k_NumCols = grid.empty() ? 0 : grid[0].size();

    for (auto& row : visited)
        for (auto& cell : row)
            cell.reset();

    for (int dir = e_Up; !visited[y][x].test(dir); ) {
        visited[y][x].set(dir);
        const auto [nextX, nextY] = step(x, y, dir);
        if ((nextX < 0) || (nextX >= k_NumRows) ||
            (nextY < 0) || (nextY >= k_NumCols))
            return false;

        if (grid[nextY][nextX] == '#')
            dir = turn(dir);
        else
            x = nextX, y = nextY;
    }
    return true;
}

int main()
{
    std::vector<std::string> grid = getInput();
    const int k_NumRows = grid.size(),
              k_NumCols = grid.empty() ? 0 : grid[0].size();
    int x = -1, y = -1;
    for (int i = 0; i < k_NumRows; ++i) {
        for (int j = 0; j < k_NumCols; ++j) {
            if (grid[i][j] == '^') {
                y = i;
                x = j;
                goto found;
            }
        }
    }
found:
    assert(x >= 0);
    assert(y >= 0);
    std::vector<std::vector<DirSet>> visited(k_NumRows,
                                             std::vector<DirSet>(k_NumCols));

    int total = 0;
    for (int i = 0; i < k_NumRows; ++i) {
        for (int j = 0; j < k_NumCols; ++j) {
            if (grid[i][j] == '.') {
                grid[i][j] = '#';
                total += haveCycle(grid, visited, x, y);
                grid[i][j] = '.';
            }
        }
    }
    std::cout << total << std::endl;
    return 0;
}
