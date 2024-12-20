#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
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

std::int64_t solve(std::int64_t                    threshold,
                   const std::vector<std::string>& grid)
{
    const std::int64_t m = grid.size(), n = grid.empty() ? 0 : grid[0].size();
    const auto start = findToken(grid, 'S'), end = findToken(grid, 'E');
    const auto fromStart = bfs(grid, start), fromEnd = bfs(grid, end);
    assert(fromStart[end[0]][end[1]] == fromEnd[start[0]][start[1]]);
    const std::int64_t cutoff = fromStart[end[0]][end[1]] - threshold;
    std::int64_t total = 0;
    for (std::int64_t i = 0; i < m; ++i) {
        for (std::int64_t j = 0; j < n; ++j) {
            if (grid[i][j] != '#')
                continue;

            for (int d = 0; d < 2; ++d) {
                const auto i1 = i+Dir[d], j1 = j+Dir[d+1],
                           i2 = i-Dir[d], j2 = j-Dir[d+1];
                if ((i1 < 0) || (i1 >= m) || (j1 < 0) || (j1 >= n) ||
                    (i2 < 0) || (i2 >= m) || (j2 < 0) || (j2 >= n) ||
                    (grid[i1][j1] == '#') || (grid[i2][j2] == '#'))
                    continue;

                const auto distA = fromStart[i1][j1] + 1 + fromEnd[i2][j2],
                           distB = fromStart[i2][j2] + 1 + fromEnd[i1][j1];
                total += (distA <= cutoff);
                total += (distB <= cutoff);
            }
        }
    }
    return total;
}

void check(int                     lineNumber,
           std::int64_t            expectedNumCheats,
           std::int64_t            threshold,
           const std::string_view& input)
{
    const std::int64_t numCheats = solve(threshold, getInput(input));
    if (numCheats != expectedNumCheats) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedNumCheats
                  << "\n> actual:   " << numCheats
                  << std::endl;
    }
}

void runTests()
{
    const std::string_view grid =
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
    constexpr int threshold[] = {  2,  4,  6,  8, 10, 12, 20, 36, 38, 40, 64 };
    constexpr int numCheats[] = { 14, 14,  2,  4,  2,  3,  1,  1,  1,  1,  1 };
    for (int total = 0, i = int(std::size(numCheats))-1; i >= 0; --i) {
        total += numCheats[i];
        check(__LINE__, total, threshold[i], grid);
    }
}

int main()
{
    runTests();

    const auto numCheats = solve(100, getInput());
    std::cout << numCheats << std::endl;
    return 0;
}
