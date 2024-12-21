#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <queue>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
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
    std::ifstream stream("day_21-keypad_conundrum-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

// Numeric Keypad
//..
//  +---+---+---+
//  | 7 | 8 | 9 |
//  +---+---+---+
//  | 4 | 5 | 6 |
//  +---+---+---+
//  | 1 | 2 | 3 |
//  +---+---+---+
//      | 0 | A |
//      +---+---+
//..
// Directional Keypad
//..
//      +---+---+
//      | ^ | A |
//  +---+---+---+
//  | < | v | > |
//  +---+---+---+
//..
//
// 780A
//
// NEXT: A->7,                        ... P1, 7->8, P1, 8->0, P1, 0->A, P1
// PREV: A->U+2, U->L+1, L->A ...
//
// NEXT: A->U,  
// PREV: A->L, P1, L->A, P1

// NEXT: A 
// CURR: A ... ->
// PREV: A ... 
//
constexpr std::string_view k_DirKeys("A<>^v");
constexpr std::string_view k_NumKeys("A0123456789");

bool isValidDirKey(char dirKey)
{
    switch (dirKey) {
        case 'A':
        case '<':
        case '>':
        case '^':
        case 'v': return true;
        default:  return false;
    }
}

std::size_t indexForDirKey(char dirKey)
{
    assert(isValidDirKey(dirKey));

    const std::string_view::size_type index = k_DirKeys.find(dirKey);
    assert(index != std::string_view::npos);
    return index;
}

std::array<int, 2> dirForDirKey(char dirKey)
{
    assert(isValidDirKey(dirKey));

    switch (dirKey) {
        case '<': return {  0, -1 };
        case '>': return {  0, +1 };
        case '^': return { -1,  0 };
        case 'v': return { +1,  0 };
    }
    assert(!"invalid dir");
    std::unreachable();
}

bool isValidNumKey(char numKey)
{
    switch (numKey) {
        case 'A':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': return true;
        default:  return false;
    }
}

std::size_t indexForNumKey(char numKey)
{
    assert(isValidNumKey(numKey));

    const std::string_view::size_type index = k_NumKeys.find(numKey);
    assert(index != std::string_view::npos);
    return index;
}

std::array<int, 2> coordsForNumKey(char numKey)
{
    assert(isValidNumKey(numKey));

    switch (numKey) {
        case 'A': return { 3, 2 };
        case '0': return { 3, 1 };
        case '1': return { 2, 0 };
        case '2': return { 2, 1 };
        case '3': return { 2, 2 };
        case '4': return { 1, 0 };
        case '5': return { 1, 1 };
        case '6': return { 1, 2 };
        case '7': return { 0, 0 };
        case '8': return { 0, 1 };
        case '9': return { 0, 2 };
    }
    assert(!"invalid num key");
    std::unreachable();
}

std::optional<char> numKeyForCoords(const std::array<int, 2>& c)
{
    switch (c[0]) {
    case 0:
        switch (c[1]) {
        case 0:  return '7';
        case 1:  return '8';
        case 2:  return '9';
        default: return std::nullopt;
        }
    case 1:
        switch (c[1]) {
        case 0:  return '4';
        case 1:  return '5';
        case 2:  return '6';
        default: return std::nullopt;
        }
    case 2:
        switch (c[1]) {
        case 0:  return '1';
        case 1:  return '2';
        case 2:  return '3';
        default: return std::nullopt;
        }
    case 3:
        switch (c[1]) {
        case 1:  return '0';
        case 2:  return 'A';
        default: return std::nullopt;
        }
    default:
        return std::nullopt;
    }
}

typedef std::array<std::array<std::int64_t, 5>, 5> DirCostTable;

DirCostTable makeDirCostTable(std::size_t numRobots)
{
    DirCostTable pDC, nDC;
    for (std::size_t i = 0; i < nDC.size(); ++i)
        for (std::size_t j = 0; j < nDC[i].size(); ++j)
            nDC[i][j] = 1; // move i->j + press

    const std::size_t iA = indexForDirKey('A'),
                      iL = indexForDirKey('<'),
                      iR = indexForDirKey('>'),
                      iU = indexForDirKey('^'),
                      iD = indexForDirKey('v');
    for ( ; numRobots > 0; --numRobots) {
        pDC = nDC;
        nDC[iA][iA] = pDC[iA][iA];
        nDC[iA][iL] = std::min(pDC[iA][iD] + pDC[iD][iL] + 1 + pDC[iL][iA],
                               pDC[iA][iL] + pDC[iL][iD] + pDC[iD][iL] + pDC[iL][iA]);
        nDC[iA][iR] = pDC[iA][iD] + pDC[iD][iA];
        nDC[iA][iU] = pDC[iA][iL] + pDC[iL][iA];
        nDC[iA][iD] = std::min(pDC[iA][iD] + pDC[iD][iL] + pDC[iL][iA],
                               pDC[iA][iL] + pDC[iL][iD] + pDC[iD][iA]);

        nDC[iL][iA] = std::min(pDC[iA][iR] + 1 + pDC[iR][iU] + pDC[iU][iA],
                               pDC[iA][iR] + pDC[iR][iU] + pDC[iU][iR] + pDC[iR][iA]);
        nDC[iL][iL] = pDC[iA][iA];
        nDC[iL][iR] = pDC[iA][iR] + 1 + pDC[iR][iA];
        nDC[iL][iU] = pDC[iA][iR] + pDC[iR][iU] + pDC[iU][iA];
        nDC[iL][iD] = pDC[iA][iR] + pDC[iR][iA];

        nDC[iR][iA] = pDC[iA][iU] + pDC[iU][iA];
        nDC[iR][iL] = pDC[iA][iL] + 1 + pDC[iL][iA];
        nDC[iR][iR] = pDC[iA][iA];
        nDC[iR][iU] = std::min(pDC[iA][iL] + pDC[iL][iU] + pDC[iU][iA],
                               pDC[iA][iU] + pDC[iU][iL] + pDC[iL][iA]);
        nDC[iR][iD] = pDC[iA][iL] + pDC[iL][iA];

        nDC[iU][iA] = pDC[iA][iR] + pDC[iR][iA];
        nDC[iU][iL] = pDC[iA][iD] + pDC[iD][iL] + pDC[iL][iA];
        nDC[iU][iR] = std::min(pDC[iA][iD] + pDC[iD][iR] + pDC[iR][iA],
                               pDC[iA][iR] + pDC[iR][iD] + pDC[iD][iA]);
        nDC[iU][iU] = pDC[iA][iA];
        nDC[iU][iD] = pDC[iA][iD] + pDC[iD][iA];

        nDC[iD][iA] = std::min(pDC[iA][iR] + pDC[iR][iU] + pDC[iU][iA],
                               pDC[iA][iU] + pDC[iU][iR] + pDC[iR][iA]);
        nDC[iD][iL] = pDC[iA][iL] + pDC[iL][iA];
        nDC[iD][iR] = pDC[iA][iR] + pDC[iR][iA];
        nDC[iD][iU] = pDC[iA][iU] + pDC[iU][iA];
        nDC[iD][iD] = pDC[iA][iA];
    }
    return nDC;
}

std::int64_t minNumPresses(const DirCostTable& dirCost,
                           char                start,
                           char                finish)
{
    typedef std::tuple<std::int64_t, char, char> State;
    std::priority_queue<State, std::vector<State>, std::greater<State>> nState;
    std::array<std::array<std::int64_t, 5>, 11> minCosts;
    for (auto& row : minCosts)
        row.fill(std::numeric_limits<std::int64_t>::max());

    const auto movePressCost = [&](char dirFrom, char dirTo) {
        return dirCost[indexForDirKey(dirFrom)][indexForDirKey(dirTo)];
    };
    const auto minCostFor = [&](char numKey, char dirKey) -> std::int64_t& {
        return minCosts[indexForNumKey(numKey)][indexForDirKey(dirKey)];
    };
    minCostFor(start, 'A') = 0;
    nState.emplace(0, start, 'A');
    std::int64_t minCost = std::numeric_limits<std::int64_t>::max();
    while (!nState.empty()) {
        const auto [pCost, pNum, pDir] = nState.top();
        nState.pop();
        if (pNum == finish) {
            minCost = std::min(minCost, pCost + movePressCost(pDir, 'A'));
            continue;
        }
        const auto [i, j] = coordsForNumKey(pNum);
        for (const char nDir : k_DirKeys.substr(1)) {
            const auto [di, dj] = dirForDirKey(nDir);
            if (const auto nNum = numKeyForCoords({ i+di, j+dj }); nNum) {
                const auto nCost = pCost + movePressCost(pDir, nDir);
                auto& nMinCost = minCostFor(*nNum, nDir);
                if (nCost < nMinCost) {
                    nMinCost = nCost;
                    nState.emplace(nCost, *nNum, nDir);
                }
            }
        }
    }
    return minCost;
}

std::int64_t minNumPresses(const std::string_view& code,
                           std::size_t             numRobots)
{
    std::int64_t numPresses = 0;
    const auto dirCost = makeDirCostTable(numRobots);
    char pNum = 'A';
    for (const char nNum : code) {
        numPresses += minNumPresses(dirCost, pNum, nNum);
        pNum = nNum;
    }
    return numPresses;
}

std::int64_t getNumericPart(const std::string_view& code)
{
    assert(code.size() > 1);
    assert(code.back() == 'A');

    std::int64_t numericPart;
    const auto [_, ec] = std::from_chars(code.data(), code.data()+code.size(),
                                         numericPart);
    assert(ec == std::errc());
    return numericPart;
}

std::int64_t solve(const std::vector<std::string>& codes,
                   std::size_t                     numRobots)
{

    return std::transform_reduce(
        codes.begin(), codes.end(), 0LL, std::plus{},
        [=](const std::string& code) {
            return minNumPresses(code, numRobots) * getNumericPart(code);
        });
}

void checkMinNumPresses(int                     lineNumber,
                        std::int64_t            expectedNumPresses,
                        const std::string_view& code)
{
    const std::int64_t numPresses = minNumPresses(code, 2);
    if (numPresses != expectedNumPresses) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedNumPresses
                  << "\n> actual:   " << numPresses
                  << std::endl;
    }
}

void checkNumericPart(int                     lineNumber,
                      std::int64_t            expectedNumericPart,
                      const std::string_view& code)
{
    const std::int64_t numericPart = getNumericPart(code);
    if (numericPart != expectedNumericPart) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedNumericPart
                  << "\n> actual:   " << numericPart
                  << std::endl;
    }
}

void check(int                     lineNumber,
           std::int64_t            expectedComplexity,
           const std::string_view& input)
{
    const std::int64_t complexity = solve(getInput(input), 2);
    if (complexity != expectedComplexity) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedComplexity
                  << "\n> actual:   " << complexity
                  << std::endl;
    }
}

void runTests()
{
    constexpr std::string_view codes[] = {
        "029A", "980A", "179A", "456A", "379A",
    };
    constexpr std::int64_t numericParts[] = {
          29,    980,    179,    456,    379,
    };
    constexpr std::string_view sequences[] = {
        "<vA<AA>>^AvAA<^A>A<v<A>>^AvA^A<vA>^A<v<A>^A>AAvA^A<v<A>A>^AAAvA<^A>A",
        "<v<A>>^AAAvA^A<vA<AA>>^AvAA<^A>A<v<A>A>^AAAvA<^A>A<vA>^A<A>A",
        "<v<A>>^A<vA<A>>^AAvAA<^A>A<v<A>>^AAvA^A<vA>^AA<A>A<v<A>A>^AAAvA<^A>A",
        "<v<A>>^AA<vA<A>>^AAvAA<^A>A<vA>^A<A>A<vA>^A<A>A<v<A>A>^AAvA<^A>A",
        "<v<A>>^AvA^A<vA<AA>>^AAvA<^A>AAvA^A<vA>^AA<A>A<v<A>A>^AAAvA<^A>A",
    };
    for (const auto [code, sequence] : std::views::zip(codes, sequences))
        checkMinNumPresses(__LINE__, sequence.size(), code);

    for (const auto [code, numericPart] : std::views::zip(codes, numericParts))
        checkNumericPart(__LINE__, numericPart, code);

    check(__LINE__, 126384,
          "029A\n"
          "980A\n"
          "179A\n"
          "456A\n"
          "379A\n");
    check(__LINE__, 174124,
          "780A\n"
          "539A\n"
          "341A\n"
          "189A\n"
          "682A\n");
}

int main()
{
    runTests();

    // 347775748043170: TOO HIGH
    const auto complexity = solve(getInput(), 25);
    std::cout << complexity << std::endl;
    return 0;
}
