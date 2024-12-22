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
#include <unordered_map>
#include <map>
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

std::int64_t nextSecretNumber(std::int64_t secretNumber)
{
    secretNumber = mix(secretNumber, secretNumber * 64);
    secretNumber = prune(secretNumber);
    secretNumber = mix(secretNumber, secretNumber / 32);
    secretNumber = prune(secretNumber);
    secretNumber = mix(secretNumber, secretNumber * 2048);
    secretNumber = prune(secretNumber);
    return secretNumber;
}

typedef std::array<std::int8_t, 4> Seq;
// struct HashSeq {
//     std::size_t operator()(const Seq& seq) const
//     {
//         std::uint32_t value = (std::uint32_t(seq[0]) <<  0) |
//                               (std::uint32_t(seq[1]) <<  8) |
//                               (std::uint32_t(seq[2]) << 16) |
//                               (std::uint32_t(seq[3]) << 24);
//         return std::hash<std::uint32_t>{}(value);
//     }
// };
// typedef std::unordered_map<Seq, std::int64_t, HashSeq> SeqCounter;
typedef std::map<Seq, std::int64_t> SeqCounter;

auto addBananas(std::int64_t secretNumber, std::size_t numSecretNumbers)
{
    SeqCounter seqCounts;
    std::array<std::int8_t, 4> seq;
    assert(numSecretNumbers > seq.size());
    std::int8_t nextPrice = secretNumber % 10, prevPrice;
    const auto updatePrices = [&]() {
        secretNumber = nextSecretNumber(secretNumber);
        prevPrice    = nextPrice;
        nextPrice    = secretNumber % 10;
    };
    for (std::size_t i = 0; i < seq.size(); ++i) {
        updatePrices();
        seq[i] = nextPrice - prevPrice;
    }
    for (auto remIters = numSecretNumbers - seq.size() - 1; ; --remIters) {
        seqCounts.emplace(seq, nextPrice);
        if (remIters == 0)
            return seqCounts;

        std::copy(std::next(seq.begin()), seq.end(), seq.begin());
        updatePrices();
        seq.back() = nextPrice - prevPrice;
    }
}

std::int64_t solve(const std::vector<std::int64_t>& secretNumbers,
                   std::size_t                      numSecretNumbers)
{
    SeqCounter seqCountSums;
    for (const std::int64_t secretNumber : secretNumbers) {
        const auto seqCounts = addBananas(secretNumber, numSecretNumbers);
        for (const auto [seq, count] : seqCounts)
            seqCountSums[seq] += count;
    }
    return std::max_element(seqCountSums.begin(), seqCountSums.end(),
                            [](const auto& lhs, const auto& rhs) {
                                return lhs.second < rhs.second;
                            })->second;
}

void check(int                     lineNumber,
           std::int64_t            expectedMaxBananas,
           std::size_t             numSecretNumbers,
           const std::string_view& input)
{
    const auto maxBananas = solve(getInput(input), numSecretNumbers);
    if (maxBananas != expectedMaxBananas) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedMaxBananas
                  << "\n> actual:   " << maxBananas
                  << std::endl;
    }
}

void runTests()
{
    check(__LINE__,  6, 10,
          "123\n");
    check(__LINE__, 23, 2000,
          "1\n"
          "2\n"
          "3\n"
          "2024\n");
}

int main()
{
    runTests();

    const auto sum = solve(getInput(), 2000);
    std::cout << sum << std::endl;
    return 0;
}
