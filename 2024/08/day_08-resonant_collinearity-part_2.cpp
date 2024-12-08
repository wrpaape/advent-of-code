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
    const int k_NumRows = poles.size(), k_NumCols = poles[0].size();
    const auto isInGrid = [=](int i, int j) {
        return ((i >= 0) && (i < k_NumRows) && (j >= 0) && (j < k_NumCols));
    };
    const auto [l, g]   = std::minmax(a1, a2);
    const auto [iL, jL] = l;
    const auto [iG, jG] = g;
    const int dI = iG - iL, dJ = jG - jL,
              dGcd = std::gcd(dI, dJ),
              stepI = dI / dGcd, stepJ = dJ / dGcd;
    for (int i = iL, j = jL; isInGrid(i, j); i -= stepI, j -= stepJ)
        poles[i][j] = true;

    for (int i = iL, j = jL; isInGrid(i, j); i += stepI, j += stepJ)
        poles[i][j] = true;
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
