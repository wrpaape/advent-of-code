#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <span>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

auto getInput(std::istream& stream)
{
    std::vector<std::array<std::int64_t, 2>> input;
    for (std::string line; std::getline(stream, line); ) {
        for (std::size_t pos = 0; pos < line.size(); ) {
            const auto dash  = line.find('-', pos+1),
                       comma = std::min(line.size(), line.find(',', dash+1));
            auto& [first, last] = input.emplace_back();
            const auto [_ptr1, err1] = std::from_chars(line.data()+pos,
                                                       line.data()+dash,
                                                       first);
            assert(err1 == std::errc());
            const auto [_ptr2, err2] = std::from_chars(line.data()+dash+1,
                                                       line.data()+comma,
                                                       last);
            assert(err2 == std::errc());
            pos = comma + 1;
        }
    }
    return input;
}

auto getInput()
{
    std::ifstream stream("day_02-gift_shop-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::span<char> makeRepeat(std::span<char> buffer, std::int64_t x)
{
    const auto [ptr, err] = std::to_chars(buffer.data(),
                                          buffer.data()+buffer.size(), x);
    assert(err == std::errc());
    const auto dEnd = std::copy(buffer.data(), ptr, ptr);
    return std::span(buffer.data(), dEnd);
}

std::int64_t toInt(std::span<const char> digits)
{
    std::int64_t x = -1;
    const auto [ptr_, err] = std::from_chars(digits.data(),
                                             digits.data()+digits.size(), x);
    assert(err == std::errc());
    return x;
}

std::int64_t makeRepeat(std::int64_t x)
{
    char buffer[20];
    const auto digits = makeRepeat(buffer, x);
    return toInt(digits);
}

std::vector<std::int64_t> invalidsInRange(std::int64_t first,
                                          std::int64_t last)
{
    assert(first >= 0);
    assert(last >= 0);
    assert(first <= last);

    std::vector<std::int64_t> invalids;
    for (std::int64_t x = 1, inv; inv = makeRepeat(x), inv <= last; ++x)
        if (inv >= first)
            invalids.push_back(inv);
    return invalids;
}

std::int64_t solve(std::span<const std::array<std::int64_t, 2>> input)
{
    std::int64_t sumInvalids = 0;
    for (const auto [first, last] : input) {
        const auto invalids = invalidsInRange(first, last);
        sumInvalids += std::reduce(invalids.begin(), invalids.end());
    }
    return sumInvalids;
}

std::string join(std::span<const std::int64_t> nums)
{
    std::string result;
    const char *delim = "";
    for (const auto num : nums)
        result += delim, result += std::to_string(num), delim = ",";
    return result;
}

void check(int                           lineNumber,
           std::span<const std::int64_t> expectedInvalids,
           std::int64_t                  first,
           std::int64_t                  last)
{
    if (const auto invalids = invalidsInRange(first, last);
        !std::ranges::equal(expectedInvalids, invalids)) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << join(expectedInvalids)
                  << "\n> actual:   " << join(invalids)
                  << std::endl;
    }
}

void check(int                     lineNumber,
           std::int64_t            expectedSum,
           const std::string_view& source)
{
    if (const auto sumInvalids = solve(getInput(source));
        sumInvalids != expectedSum) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedSum
                  << "\n> actual:   " << sumInvalids
                  << std::endl;
    }
}

void runTests()
{
    check(__LINE__, std::array{11L, 22L},    11,         22);
    check(__LINE__, std::array{99L},         95,         115);
    check(__LINE__, std::array{1010L},       998,        1012);
    check(__LINE__, std::array{1188511885L}, 1188511880, 1188511890);
    check(__LINE__, std::array{222222L},     222220,     222224);
    check(__LINE__, {},                      1698522,    1698528);
    check(__LINE__, std::array{446446L},     446443,     446449);
    check(__LINE__, std::array{38593859L},   38593856,   38593862);
    check(
        __LINE__,
        1227775554,
        "11-22,95-115,998-1012,1188511880-1188511890,222220-222224,"
        "1698522-1698528,446443-446449,38593856-38593862,565653-565659,"
        "824824821-824824827,2121212118-2121212124\n"
    );
}

int main()
{
    runTests();

    const auto sumInvalids = solve(getInput());
    std::cout << sumInvalids << std::endl;
    return 0;
}
