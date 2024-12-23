#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

constexpr int Dir[] = { 0, +1, 0, -1, 0 };

auto getLines(std::istream& stream)
{
    std::vector<std::string> lines;
    for (std::string line; std::getline(stream, line); )
        lines.push_back(std::move(line));

    return lines;
}

auto getInput(std::istream& stream)
{
    const auto grid = getLines(stream);
    assert(!grid.empty());
    const auto numCols = grid.empty() ? 0 : grid[0].size();
    const bool isGrid = std::all_of(grid.begin(), grid.end(),
                                    [=](const std::string& row) {
                                        return row.size() == numCols;
                                    });
    assert(isGrid);
    return grid;
}

auto getInput()
{
    std::ifstream stream("day_20-race_condition-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::array<std::int64_t, 2> findToken(const std::vector<std::string>& grid,
                                      char                            token)
{
    const std::int64_t m = grid.size(), n = grid.empty() ? 0 : grid[0].size();
    for (std::int64_t i = 0; i < m; ++i)
        for (std::int64_t j = 0; j < n; ++j)
            if (grid[i][j] == token)
                return { i, j };

    assert(!"invalid input");
    std::unreachable();
}

std::vector<std::vector<std::int64_t>> bfs(
                                      const std::vector<std::string>&    grid,
                                      const std::array<std::int64_t, 2>& start)
{
    const std::int64_t m = grid.size(), n = grid.empty() ? 0 : grid[0].size();
    std::vector<std::vector<std::int64_t>> dist(
                                            m,
                                            std::vector<std::int64_t>(n, m*n));
    dist[start[0]][start[1]] = 0;
    std::queue<std::array<std::int64_t, 2>> frontier;
    frontier.push(start);
    for (std::int64_t numSteps = 1; !frontier.empty(); ++numSteps) {
        for (std::int64_t remSize = frontier.size(); remSize > 0; --remSize) {
            const auto [i, j] = frontier.front();
            frontier.pop();
            for (int d = 0; d < 4; ++d) {
                const auto nI = i+Dir[d], nJ = j+Dir[d+1];
                if ((nI >= 0) && (nI < m) && (nJ >= 0) && (nJ < n) &&
                    (grid[nI][nJ] != '#') && (dist[nI][nJ] > numSteps)) {
                    dist[nI][nJ] = numSteps;
                    frontier.push({ nI, nJ });
                }
            }
        }
    }
    return dist;
}

std::vector<std::array<std::int64_t, 2>> makeWindow(std::int64_t stride)
{
    std::vector<std::array<std::int64_t, 2>> pFrontier, nFrontier, window;
    nFrontier.push_back({ 0, 0 });
    std::vector<std::vector<bool>> visited(2*stride+1,
                                           std::vector<bool>(2*stride+1));
    visited[stride][stride] = true;
    for (std::int64_t numSteps = 1; numSteps <= stride; ++numSteps) {
        nFrontier.swap(pFrontier);
        nFrontier.clear();
        for (const auto [pI, pJ] : pFrontier) {
            for (int d = 0; d < 4; ++d) {
                if (const auto nI = pI+Dir[d], nJ = pJ+Dir[d+1]; 
                    !visited[stride+nI][stride+nJ]) {
                    visited[stride+nI][stride+nJ] = true;
                    nFrontier.push_back({ nI, nJ });
                }
            }
        }
        window.insert(window.end(), nFrontier.begin(), nFrontier.end());
    }
    return window;
}


std::int64_t solve(std::int64_t                    threshold,
                   std::int64_t                    cheatStride,
                   const std::vector<std::string>& grid,
                   bool                            debug = false)
{
    const std::int64_t m = grid.size(), n = grid.empty() ? 0 : grid[0].size();
    const auto start = findToken(grid, 'S'), end = findToken(grid, 'E');
    const auto fromStart = bfs(grid, start), fromEnd = bfs(grid, end);
    assert(fromStart[end[0]][end[1]] == fromEnd[start[0]][start[1]]);
    const std::int64_t cutoff = fromStart[end[0]][end[1]] - threshold;
    const auto window = makeWindow(cheatStride);
    std::int64_t total = 0;
    for (std::int64_t i = 0; i < m; ++i) {
        for (std::int64_t j = 0; j < n; ++j) {
            if (grid[i][j] == '#')
                continue;

            if (debug)
                std::cout << '(' << i << ',' << j << ')' << std::endl;

            for (const auto [di, dj] : window) {
                const auto nI = i+di, nJ = j+dj;
                if ((nI < 0) || (nI >= m) || (nJ < 0) || (nJ >= n) ||
                    (grid[nI][nJ] == '#'))
                    continue;

                const auto numSteps = std::abs(di) + std::abs(dj);
                const auto dist = fromStart[i][j] + numSteps + fromEnd[nI][nJ];
                total += (dist <= cutoff);
            }
        }
    }
    return total;
}

void check(int                     lineNumber,
           std::int64_t            expectedNumCheats,
           std::int64_t            threshold,
           std::int64_t            cheatStride,
           const std::string_view& input)
{
    const std::int64_t numCheats = solve(threshold,
                                         cheatStride,
                                         getInput(input));
    if (numCheats != expectedNumCheats) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedNumCheats
                  << "\n> actual:   " << numCheats
                  << std::endl;
    }
}

void runTests()
{
    constexpr std::string_view grid =
        "###############\n"
        "#...#...#.....#\n"
        "#.#.#.#.#.###.#\n"
        "#S#...#.#.#...#\n"
        "#######.#.#.###\n"
        "#######.#.#...#\n"
        "#######.#.###.#\n"
        "###..E#...#...#\n"
        "###.#######.###\n"
        "#...###...#...#\n"
        "#.#####.#.###.#\n"
        "#.#...#.#.#...#\n"
        "#.#.#.#.#.#.###\n"
        "#...#...#...###\n"
        "###############\n";
    constexpr std::int64_t thresholds[] = {
        50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76
    };
    std::int64_t numCheats[] = {
        32, 31, 29, 39, 25, 23, 20, 19, 12, 14, 12, 22,  4,  3
    };
    std::partial_sum(std::rbegin(numCheats), std::rend(numCheats),
                     std::rbegin(numCheats));
    for (const auto [t, c] : std::views::zip(thresholds, numCheats))
        check(__LINE__, c, t, 20, grid);
}

int main()
{
    runTests();

    const auto numCheats = solve(100, 20, getInput());
    std::cout << numCheats << std::endl;
    return 0;
}
