#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
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
    for (const auto [line, bytePos] : std::views::zip(lines, bytePositions)) {
        const char *c = line.c_str(), *const cEnd = c + line.size();
        for (auto& coord : bytePos) {
            const auto [ptr, ec] = std::from_chars(c, cEnd, coord);
            c = ptr + (ptr != cEnd);
            assert(ec == std::errc());
        }
    }
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

std::int64_t getComp(std::vector<std::int64_t>& comps, std::int64_t n)
{
    assert(n >= 0);
    assert(n < std::int64_t(comps.size()));
    assert(comps[n] < std::int64_t(comps.size()));

    if (comps[n] < 0)
        return n;

    comps[n] = getComp(comps, comps[n]);
    return comps[n];
}

bool tryMerge(std::vector<std::int64_t>& comps, std::int64_t n1, std::int64_t n2) {
    auto c1 = getComp(comps, n1), c2 = getComp(comps, n2);
    if (c1 == c2)
        return false;

    if (comps[c2] < comps[c1])
        std::swap(c1, c2);

    comps[c1] -= (comps[c1] == comps[c2]);
    comps[c2] = c1;
    return true;
}

std::array<std::int64_t, 2>
runSim(std::int64_t                                    gridSize,
       const std::vector<std::array<std::int64_t, 2>>& bytes)
{
    assert(gridSize >= 0);

    const auto numComps = gridSize*gridSize;
    std::vector<std::int64_t> comps(numComps, -1);
    for (const auto [x, y] : bytes)
        comps[y*gridSize+x] = std::numeric_limits<std::int64_t>::max();

    constexpr std::int64_t k_Dir[]  = { 0, +1, 0, -1, 0 };
    std::deque<std::array<std::int64_t, 2>> frontier;
    for (std::int64_t pI = 0, pN = 0; pI < gridSize; ++pI) {
        for (std::int64_t pJ = 0; pJ < gridSize; ++pJ, ++pN) if (comps[pN] < numComps) {
            frontier.clear();
            frontier.push_back({ pI, pJ });
            do {
                const auto [i, j] = frontier.front();
                frontier.pop_front();
                const auto n = i*gridSize+j;
                for (int d = 0; d < 4; ++d)
                    if (const auto nI = i+k_Dir[d], nJ = j+k_Dir[d+1], nN = nI*gridSize+nJ;
                        (nI >= 0) && (nI < gridSize) &&
                        (nJ >= 0) && (nJ < gridSize) &&
                        (comps[nN] < numComps) &&
                        tryMerge(comps, n, nN))
                        frontier.push_back({ nI, nJ });
            } while (!frontier.empty());
        }
    }
    assert(getComp(comps, 0) != getComp(comps, numComps-1));
    for (const auto [x, y] : bytes | std::views::reverse) {
        const auto i = y, j = x, n = i*gridSize+j;
        assert(comps[n] == std::numeric_limits<std::int64_t>::max());
        comps[n] = -1;
        for (int d = 0; d < 4; ++d)
            if (const auto nI = i+k_Dir[d], nJ = j+k_Dir[d+1], nN = nI*gridSize+nJ;
                (nI >= 0) && (nI < gridSize) &&
                (nJ >= 0) && (nJ < gridSize) &&
                (comps[nN] < numComps))
                (void) tryMerge(comps, n, nN);

        if (getComp(comps, 0) == getComp(comps, numComps-1))
            return { x, y };
    }
    assert(!"bad input");
    std::unreachable();
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

    const std::array<std::int64_t, 2> byte = runSim(71, getInput()), expected = { 22, 33 };
    std::cout << byte[0] << ',' << byte[1] << std::endl;
    assert(byte == expected);
    return 0;
}
