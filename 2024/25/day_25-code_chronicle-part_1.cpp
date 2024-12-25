#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

struct Input {
    std::vector<std::array<std::int8_t, 5>> d_keys;
    std::vector<std::array<std::int8_t, 5>> d_locks;
};

auto getInput(std::istream& stream)
{
    Input input;
    const auto isMatch = [](const std::string& line, char token) { 
        return std::all_of(line.begin(), line.end(),
                           [=](char cell) { return cell == token; });
    };
    for (std::string line; std::getline(stream, line); ) {
        const auto [heightToken, obj] =
              isMatch(line, '.')
            ? std::make_pair('.', &input.d_keys.emplace_back())
            : isMatch(line, '#')
            ? std::make_pair('#', &input.d_locks.emplace_back())
            : std::make_pair('X', nullptr);
        assert(obj);
        while (std::getline(stream, line) && !line.empty())
            for (const auto& [token, counter] : std::views::zip(line, *obj))
                counter += (token == heightToken);
    }
    return input;
}

auto getInput()
{
    std::ifstream stream("day_25-code_chronicle-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

bool haveFit(const std::array<std::int8_t, 5>& key,
             const std::array<std::int8_t, 5>& lock)
{
    return std::ranges::all_of(std::views::zip(key, lock), [](const auto& p) {
        const auto [k, l] = p;
        return k >= l;
    });
}

std::uint64_t solve(const Input& input)
{
    std::uint64_t numPairs = 0;
    for (const auto& key : input.d_keys)
        for (const auto& lock : input.d_locks)
            numPairs += haveFit(key, lock);

    return numPairs;
}

void check(int                     lineNumber,
           std::uint64_t           expectedNumPairs,
           const std::string_view& source)
{
    if (const auto numPairs = solve(getInput(source));
        numPairs != expectedNumPairs) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedNumPairs
                  << "\n> actual:   " << numPairs
                  << std::endl;
    }
}

void runTests()
{
    check(
        __LINE__,
        3,
        "#####\n"
        ".####\n"
        ".####\n"
        ".####\n"
        ".#.#.\n"
        ".#...\n"
        ".....\n"
        "\n"
        "#####\n"
        "##.##\n"
        ".#.##\n"
        "...##\n"
        "...#.\n"
        "...#.\n"
        ".....\n"
        "\n"
        ".....\n"
        "#....\n"
        "#....\n"
        "#...#\n"
        "#.#.#\n"
        "#.###\n"
        "#####\n"
        "\n"
        ".....\n"
        ".....\n"
        "#.#..\n"
        "###..\n"
        "###.#\n"
        "###.#\n"
        "#####\n"
        "\n"
        ".....\n"
        ".....\n"
        ".....\n"
        "#....\n"
        "#.#..\n"
        "#.#.#\n"
        "#####\n"
    );
}

int main()
{
    runTests();

    const auto numPairs = solve(getInput());
    std::cout << numPairs << std::endl;
    return 0;
}
