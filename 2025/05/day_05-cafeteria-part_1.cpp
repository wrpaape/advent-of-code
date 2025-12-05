#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <ranges>
#include <span>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

struct Input {
    std::vector<std::array<std::int64_t, 2>> d_ranges;
    std::vector<std::int64_t>                d_ids;
};

auto getInput(std::istream& stream)
{
    Input input;
    for (std::string line; std::getline(stream, line) && !line.empty(); ) {
        auto& [first, last] = input.d_ranges.emplace_back();
        const auto dash = line.find('-');
        assert(dash != std::string::npos);
        const auto [_ptr1, err1] = std::from_chars(line.data(),
                                                   line.data()+dash,
                                                   first);
        assert(err1 == std::errc());
        const auto [_ptr2, err2] = std::from_chars(line.data()+dash+1,
                                                   line.data()+line.size(),
                                                   last);
        assert(err2 == std::errc());
    }
    for (std::int64_t id; stream >> id; )
        input.d_ids.push_back(id);
    return input;
}

auto getInput()
{
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        if (std::filesystem::is_regular_file(entry.path()) &&
            entry.path().string().ends_with("-input.txt")) {
            std::ifstream stream(entry.path().string());
            return getInput(stream);
        }
    }
    std::cerr << "Failed to find input file in the current directory"
              << std::endl;
    std::abort();
}

auto getInput(std::string_view input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::vector<std::array<std::int64_t, 2>> mergeRanges(
                     const std::span<const std::array<std::int64_t, 2>> ranges)
{
    std::vector<std::array<std::int64_t, 2>> merged(ranges.begin(),
                                                    ranges.end());
    if (!merged.empty()) {
        std::ranges::sort(merged);
        auto o = merged.begin();
        for (const auto r : merged | std::views::drop(1))
            if (((*o)[1]+1) >= r[0])
                (*o)[1] = std::max((*o)[1], r[1]);
            else
                *++o = r;
        merged.erase(o+1, merged.end());
    }
    return merged;
}

std::map<std::int64_t, std::int64_t> makeRanges(
                      const std::span<const std::array<std::int64_t, 2>> input)
{
    std::map<std::int64_t, std::int64_t> ranges;
    for (const auto [first, last] : mergeRanges(input))
        ranges[first] = last;
    return ranges;
}

bool isFresh(const std::map<std::int64_t, std::int64_t>& ranges,
             std::int64_t                                id)
{
    const auto it = ranges.upper_bound(id);
    return (it != ranges.begin()) && (std::prev(it)->second >= id);
}

std::int64_t solve(const Input& input)
{
    std::int64_t numFresh = 0;
    for (const auto ranges = makeRanges(input.d_ranges);
         const std::int64_t id : input.d_ids)
        numFresh += isFresh(ranges, id);
    return numFresh;
}

void check(int                                         lineNumber,
           bool                                        expectedIsFresh,
           const std::map<std::int64_t, std::int64_t>& ranges,
           std::int64_t                                id)
{
    if (const auto f = isFresh(ranges, id); f != expectedIsFresh) {
        std::cerr
            << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                           lineNumber, expectedIsFresh, f)
            << std::endl;
    }
}

void check(int              lineNumber,
           std::int64_t     expectedNumFresh,
           std::string_view source)

{
    if (const auto numFresh = solve(getInput(source));
        numFresh != expectedNumFresh) {
        std::cerr
            << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                           lineNumber, expectedNumFresh, numFresh)
            << std::endl;
    }
}

void runTests()
{
    const std::string_view example =
R"(3-5
10-14
16-20
12-18

1
5
8
11
17
32
)";
    const auto ranges = makeRanges(getInput(example).d_ranges);
    check(__LINE__, false, ranges,  1);
    check(__LINE__, true,  ranges,  5);
    check(__LINE__, false, ranges,  8);
    check(__LINE__, true,  ranges, 11);
    check(__LINE__, true,  ranges, 17);
    check(__LINE__, false, ranges, 32);
    check(__LINE__, 3, example);
}

int main()
{
    runTests();

    const auto numFresh = solve(getInput());
    std::cout << numFresh << std::endl;
    return 0;
}
