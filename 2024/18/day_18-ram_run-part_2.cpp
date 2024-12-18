#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <queue>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

auto getLines(std::istream& stream)
{
    std::vector<std::string> lines;
    for (std::string line; std::getline(stream, line); )
        lines.push_back(std::move(line));

    return lines;
}

auto getInput(std::istream& stream)
{
    const std::vector<std::string>           lines(getLines(stream));
    std::vector<std::array<std::int64_t, 2>> bytePositions(lines.size());
#if 0
    for (auto& [line, bytePos] : std::views::zip(lines, bytePositions)) {
        const char *c = line.c_str(), *const cEnd = c + line.size();
        for (auto& coord : bytePos) {
            const auto [ptr, ec] = std::from_chars(c, cEnd, coord);
            c = ptr + (ptr != cEnd);
            assert(ec == std::errc());
        }
    }

#else
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const char *c = lines[i].c_str(), *const cEnd = c + lines[i].size();
        for (std::size_t j = 0; j < 2; ++j) {
            const auto [ptr, ec] = std::from_chars(c, cEnd, bytePositions[i][j]);
            c = ptr + (ptr != cEnd);
            assert(ec == std::errc());
        }
    }
#endif
    return bytePositions;
}

auto getInput()
{
    std::ifstream stream("day_18-ram_run-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

bool tryPath(std::int64_t                                    gridSize,
             std::int64_t                                    numBytes,
             const std::vector<std::array<std::int64_t, 2>>& bytes)
{
    assert(gridSize > 0);
    assert(numBytes >= 0);
    assert(numBytes <= bytes.size());

    std::vector<std::vector<bool>> grid(gridSize,
                                        std::vector<bool>(gridSize, true));
    for (std::int64_t i = 0; i < numBytes; ++i)
        grid[bytes[i][1]][bytes[i][0]] = false;

    const std::array<std::int64_t, 2> k_Exit = { gridSize-1, gridSize-1 };
    std::queue<std::array<std::int64_t, 2>> frontier;
    frontier.push({ 0, 0 });
    assert(grid[0][0]);
    grid[0][0] = false;
    for (std::int64_t numSteps = 0; !frontier.empty(); ++numSteps) {
        for (auto rem = frontier.size(); rem > 0; --rem) {
            const auto bytePos = frontier.front();
            frontier.pop();
            if (bytePos == k_Exit)
                return true;

            const auto [i, j] = bytePos;
            for (int d = 0; d < 4; ++d) {
                constexpr int k_Dir[] = { 0, +1, 0, -1, 0 };
                if (const auto nI = i+k_Dir[d], nJ = j+k_Dir[d+1];
                    (nI >= 0) && (nI < gridSize) &&
                    (nJ >= 0) && (nJ < gridSize) &&
                    grid[nI][nJ]) {
                    grid[nI][nJ] = false;
                    frontier.push({ nI, nJ });
                }
            }
        }
    }
    return false;
}

std::array<std::int64_t, 2>
runSim(std::int64_t                                    gridSize,
       const std::vector<std::array<std::int64_t, 2>>& bytes)
{
    std::int64_t numBytes;
    for (numBytes = 0; tryPath(gridSize, numBytes, bytes); ++numBytes)
        continue;

    return bytes[numBytes-1];
}

void check(int                                lineNumber,
           const std::array<std::int64_t, 2>& expectedByte,
           std::int64_t                       gridSize,
           const std::string_view&            input)
{
    const auto byte = runSim(gridSize, getInput(input));
    if (byte != expectedByte) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected:\n" << '(' << expectedByte[0] << ',' << expectedByte[1] << ')'
                  << "\n> actual:\n" <<  '(' << byte[0] << ',' << byte[1] << ')'
                  << std::endl;
    }
}

void runTests()
{
    check(
        __LINE__,
        { 6, 1 },
        7,
        "5,4\n"
        "4,2\n"
        "4,5\n"
        "3,0\n"
        "2,1\n"
        "6,3\n"
        "2,4\n"
        "1,5\n"
        "0,6\n"
        "3,3\n"
        "2,6\n"
        "5,1\n"
        "1,2\n"
        "5,5\n"
        "2,5\n"
        "6,5\n"
        "1,4\n"
        "0,4\n"
        "6,4\n"
        "1,1\n"
        "6,1\n"
        "1,0\n"
        "0,5\n"
        "1,6\n"
        "2,0\n"
    );
}

int main()
{
    runTests();

    const auto byte = runSim(71, getInput());
    std::cout << byte[0] << ',' << byte[1] << std::endl;
    return 0;
}
