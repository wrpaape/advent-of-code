#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

auto getInput(std::istream& stream)
{
    std::vector<std::int64_t> secretNumbers;
    std::copy(std::istream_iterator<std::int64_t>(stream),
              std::istream_iterator<std::int64_t>{},
              std::back_inserter(secretNumbers));
    return secretNumbers;
}

auto getInput()
{
    std::ifstream stream("day_22-monkey_market-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::int64_t mix(std::int64_t lhs, std::int64_t rhs)
{
    return lhs ^ rhs;
}

std::int64_t prune(std::int64_t num)
{
    return num % 16777216;
}

std::int64_t generateSecretNumber(std::int64_t secretNumber,
                                  std::size_t  numIterations)
{
    for (auto remIters = numIterations; remIters > 0; --remIters) {
        secretNumber = mix(secretNumber, secretNumber * 64);
        secretNumber = prune(secretNumber);
        secretNumber = mix(secretNumber, secretNumber / 32);
        secretNumber = prune(secretNumber);
        secretNumber = mix(secretNumber, secretNumber * 2048);
        secretNumber = prune(secretNumber);
    }
    return secretNumber;
}

std::int64_t solve(const std::vector<std::int64_t>& secretNumbers,
                   std::size_t                      numIterations)
{
    return std::transform_reduce(
        secretNumbers.begin(), secretNumbers.end(), 0LL, std::plus{},
        [=](std::int64_t secretNumber) {
            return generateSecretNumber(secretNumber, numIterations);
        });
}

void check(int          lineNumber,
           std::int64_t expectedNewSecretNumber,
           std::size_t  numIterations,
           std::int64_t initialSecretNumber)
{
    const auto newSecretNumber = generateSecretNumber(initialSecretNumber,
                                                      numIterations);
    if (newSecretNumber != expectedNewSecretNumber) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedNewSecretNumber
                  << "\n> actual:   " << newSecretNumber
                  << std::endl;
    }
}

void check(int                     lineNumber,
           std::int64_t            expectedSum,
           std::size_t             numIterations,
           const std::string_view& input)
{
    const auto sum = solve(getInput(input), numIterations);
    if (sum != expectedSum) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedSum
                  << "\n> actual:   " << sum
                  << std::endl;
    }
}

void runTests()
{
    constexpr std::int64_t secretNumSeq[] = {
        123, 15887950, 16495136, 527345, 704524, 1553684, 12683156, 11100544,
        12249484, 7753432, 5908254
    };
    for (std::size_t i = 0; i < (std::size(secretNumSeq)-1); ++i)
        check(__LINE__, secretNumSeq[i+1], 1, secretNumSeq[i]);

    check(__LINE__,  8685429, 2000,    1);
    check(__LINE__,  4700978, 2000,   10);
    check(__LINE__, 15273692, 2000,  100);
    check(__LINE__,  8667524, 2000, 2024);
    check(__LINE__, 37327623, 2000,
          "1\n"
          "10\n"
          "100\n"
          "2024\n");
}

int main()
{
    runTests();

    const auto sum = solve(getInput(), 2000);
    std::cout << sum << std::endl;
    return 0;
}
