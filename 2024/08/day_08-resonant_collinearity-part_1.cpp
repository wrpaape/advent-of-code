#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

auto getInput()
{
    std::vector<std::string> grid;
    for (std::ifstream stream("day_08-resonant_collinearity-input.txt");
         std::getline(stream, grid.emplace_back()); )
        continue;

    assert(!grid.empty());
    grid.pop_back();
    return grid;
}

void placePoles(std::vector<std::vector<bool>>& poles,
                const std::pair<int, int>&      a1,
                const std::pair<int, int>&      a2)
{
    const auto place = [&](int i, int j) {
        if ((i >= 0) && (i < poles.size()) &&
            (j >= 0) && (j < poles[i].size()))
            poles[i][j] = true;
    };
    const auto [l, g]   = std::minmax(a1, a2);
    const auto [iL, jL] = l;
    const auto [iG, jG] = g;
    const int dI        = iG - iL;
    if (jG < jL) {
        // L
        //     G
        const int dJ = jL - jG;
        place(iL-dI, jL+dJ);
        place(iG+dI, jG-dJ);
    }
    else {
        //     G
        // L
        const int dJ = jG - jL;
        place(iL-dI, jL-dJ);
        place(iG+dI, jG+dJ);
    }
}

int main()
{
    const auto grid = getInput();
    const int k_NumRows = grid.size(),
              k_NumCols = grid.empty() ? 0 : grid[0].size();
    std::array<std::vector<std::pair<int, int>>,
               (std::numeric_limits<char>::max()+1)> antennas;
    for (int i = 0; i < k_NumRows; ++i)
        for (int j = 0; j < k_NumCols; ++j)
            if (grid[i][j] != '.')
                antennas[grid[i][j]].emplace_back(i, j);

    std::vector<std::vector<bool>> poles(k_NumRows,
                                         std::vector<bool>(k_NumCols));
    for (const auto& ants : antennas)
        for (int i = 0; i < ants.size(); ++i)
            for (int j = i+1; j < ants.size(); ++j)
                placePoles(poles, ants[i], ants[j]);

    std::cout <<
        std::transform_reduce(poles.begin(), poles.end(), 0, std::plus{},
                                [](const auto& row)
                                {
                                    return std::count(row.begin(), row.end(), true);
                                })
        << std::endl;
    return 0;
}
