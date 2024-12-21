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
#include <unordered_set>
#include <utility>
#include <vector>

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

struct State {
    static constexpr std::size_t k_NumRobots = 2;

    std::array<char, k_NumRobots+2> d_keys;
    std::string                     d_remCode;

    static bool isValidDirKey(char dirKey)
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

    static std::optional<std::array<int, 2>>
    dirCoordsForKey(char dirKey)
    {
        assert(isValidDirKey(dirKey));

        switch (dirKey) {
            case 'A': return std::nullopt;
            case '<': return std::array<int, 2>{  0, -1 };
            case '>': return std::array<int, 2>{  0, +1 };
            case '^': return std::array<int, 2>{ -1,  0 };
            case 'v': return std::array<int, 2>{ +1,  0 };
        }
        assert(!"invalid dir key");
        std::unreachable();
    }

    static std::optional<char>
    dirKeyForCoords(const std::array<int, 2>& c)
    {
        switch (c[0]) {
        case 0:
            switch (c[1]) {
            case 1:  return '^';
            case 2:  return 'A';
            default: return std::nullopt;
            }
        case 1:
            switch (c[1]) {
            case 0:  return '<';
            case 1:  return 'v';
            case 2:  return '>';
            default: return std::nullopt;
            }
        default:
            return std::nullopt;
        }
    }

    static bool isValidNumKey(char numKey)
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

    static std::optional<char>
    numKeyForCoords(const std::array<int, 2>& c)
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

    State(const std::string_view& code)
    : d_remCode(code.rbegin(), code.rend())
    {
        d_keys.fill('A');
    }

    bool tryMoveRoboDir(std::size_t r, const std::array<int, 2>& d)
    {
        assert(r < k_NumRobots);

        const auto [ i,  j] = roboDirCoords(r);
        const auto [di, dj] = d;
        if (const auto dirKey = dirKeyForCoords({ i+di, j+dj }); dirKey) {
            roboDirKey(r) = *dirKey;
            return true;
        }
        return false;
    }

    bool tryMoveRoboNum(const std::array<int, 2>& d)
    {
        assert(!isDone());

        const auto [ i,  j] = roboNumCoords();
        const auto [di, dj] = d;
        if (const auto numKey = numKeyForCoords({ i+di, j+dj }); numKey) {
            d_keys.back() = *numKey;
            return true;
        }
        return false;
    }

    char& roboDirKey(std::size_t r)
    {
        assert(r < k_NumRobots);

        return d_keys[r+1];
    }

    char roboDirKey(std::size_t r) const
    {
        assert(r < k_NumRobots);

        return d_keys[r+1];
    }

    bool isDone() const
    {
        return d_remCode.empty();
    }

    std::array<int, 2> roboDirCoords(std::size_t r) const
    {
        assert(r < k_NumRobots);

        switch (roboDirKey(r)) {
            case '^': return { 0, 1 };
            case 'A': return { 0, 2 };
            case '<': return { 1, 0 };
            case 'v': return { 1, 1 };
            case '>': return { 1, 2 };
        }
        assert(!"invalid robot dir key state");
        std::unreachable();
    }

    std::array<int, 2> roboNumCoords() const
    {
        switch (d_keys.back()) {
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
        assert(!"invalid robot num key state");
        std::unreachable();
    }

    std::optional<State> tryPress(char dirKey) const
    {
        assert(!isDone());
        assert(isValidDirKey(dirKey));

        std::optional<State> nS(*this);
        nS->d_keys.front() = dirKey;
        for (std::size_t r = 0; r < k_NumRobots; ++r)
            if (const auto d = dirCoordsForKey(nS->d_keys[r]); d)
                return nS->tryMoveRoboDir(r, *d) ? nS : std::nullopt;

        if (const auto d = dirCoordsForKey(nS->d_keys[k_NumRobots]); d)
            return nS->tryMoveRoboNum(*d) ? nS : std::nullopt;

        if (nS->d_keys.back() != nS->d_remCode.back())
            return std::nullopt;

        nS->d_remCode.pop_back();
        return nS;
    }

    auto operator<=>(const State&) const = default;
};

template<>
struct std::hash<State>
{
    std::size_t operator()(const State& s) const noexcept
    {
        const std::string_view keys(s.d_keys.begin(), s.d_keys.end()),
                               remCode(s.d_remCode);
        const std::size_t keysHash    = std::hash<std::string_view>{}(keys),
                          remCodeHash = std::hash<std::string_view>{}(remCode);
        return keysHash ^ (remCodeHash << 1);
    }
};

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

std::int64_t minNumPresses(const std::string_view& code)
{
    std::queue<State>         frontier;
    std::unordered_set<State> visited;
    frontier.emplace(code);
    visited.emplace(code);
    for (std::size_t numPresses = 0; ; ++numPresses) {
        assert(!frontier.empty());
        for (std::size_t rem = frontier.size(); rem > 0; --rem) {
            const State s = frontier.front();
            frontier.pop();
            if (s.isDone())
                return numPresses;

            for (const char dirKey : std::string_view("A^v<>"))
                if (const auto nS = s.tryPress(dirKey);
                    nS && visited.emplace(*nS).second)
                    frontier.emplace(*nS);
        }
    }
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

std::int64_t solve(const std::vector<std::string>& codes)
{
    return std::transform_reduce(codes.begin(), codes.end(), 0LL, std::plus{},
                                 [](const std::string& code) {
                                     return minNumPresses(code) *
                                            getNumericPart(code);
                                 });
}

void checkMinNumPresses(int                     lineNumber,
                        std::int64_t            expectedNumPresses,
                        const std::string_view& code)
{
    const std::int64_t numPresses = minNumPresses(code);
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
    const std::int64_t complexity = solve(getInput(input));
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
}

int main()
{
    runTests();

    const auto complexity = solve(getInput());
    std::cout << complexity << std::endl;
    return 0;
}
