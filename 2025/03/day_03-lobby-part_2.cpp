#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <ranges>
#include <span>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

auto getInput(std::istream& stream)
{
    std::vector<std::string> input;
    for (std::string line; std::getline(stream, line); )
        input.push_back(std::move(line));
    return input;
}

auto getInput()
{
    std::ifstream stream("day_03-lobby-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::int64_t joltageForBank(std::string_view bank)
{
    assert(bank.size() >= 12);

    const std::int64_t n = bank.size();
    std::vector<std::int64_t> pMaxJ(n+1), nMaxJ(n+1);
    for (std::int64_t size = 1, mag = 1; size <= 12; ++size, mag *= 10) {
        std::swap(pMaxJ, nMaxJ);
        for (std::int64_t i = n-size; i >= 0; --i) {
            nMaxJ[i] = std::max(nMaxJ[i+1], mag*(bank[i]-'0') + pMaxJ[i+1]);
        }
    }
    return nMaxJ[0];
}

std::int64_t solve(std::span<const std::string> input)
{
    return std::ranges::fold_left(input |
                                  std::views::transform(joltageForBank),
                                  0LL,
                                  std::plus{});
}

void checkBank(int              lineNumber,
               std::int64_t     expectedJoltage,
               std::string_view bank)
{
    if (const auto joltage = joltageForBank(bank);
        joltage != expectedJoltage) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedJoltage
                  << "\n> actual:   " << joltage
                  << std::endl;
    }
}

void check(int                     lineNumber,
           std::int64_t            expectedTotalJoltage,
           const std::string_view& source)
{
    if (const auto totalJoltage = solve(getInput(source));
        totalJoltage != expectedTotalJoltage) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedTotalJoltage
                  << "\n> actual:   " << totalJoltage
                  << std::endl;
    }
}

void runTests()
{
    checkBank(__LINE__, 987654321111, "987654321111111");
    checkBank(__LINE__, 811111111119, "811111111111119");
    checkBank(__LINE__, 434234234278, "234234234234278");
    checkBank(__LINE__, 888911112111, "818181911112111");
    check(
        __LINE__,
        3121910778619,
        "987654321111111\n"
        "811111111111119\n"
        "234234234234278\n"
        "818181911112111\n"
    );
}

int main()
{
    runTests();

    const auto sumInvalids = solve(getInput());
    std::cout << sumInvalids << std::endl;
    return 0;
}
