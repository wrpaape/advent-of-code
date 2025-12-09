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
#include <span>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

struct Point {
    std::int64_t d_x, d_y;
    auto operator<=>(const Point&) const = default;
};

auto getInput(std::istream& stream)
{
    std::vector<Point> input;
    for (std::string line; std::getline(stream, line); ) {
        auto& p = input.emplace_back();
        const auto lineEnd = line.data() + line.size();
        const auto [ptr1, err1] = std::from_chars(line.data(), lineEnd, p.d_x);
        assert(ptr1 != lineEnd);
        assert(*ptr1 == ',');
        assert(err1 == std::errc());
        const auto [ptr2, err2] = std::from_chars(ptr1+1, lineEnd, p.d_y);
        assert(ptr2 == lineEnd);
        assert(err2 == std::errc());
    }
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

std::int64_t area(const Point& lhs, const Point& rhs)
{
    const auto dx = lhs.d_x - rhs.d_x + 1;
    const auto dy = lhs.d_y - rhs.d_y + 1;
    return std::abs(dx * dy);
}

std::int64_t solve(std::span<const Point> input)
{
    assert(!input.empty());

    std::int64_t maxArea = 0;
    const std::int64_t n = input.size();
    for (std::int64_t i = 0; i < n; ++i)
        for (std::int64_t j = i+1; j < n; ++j)
            maxArea = std::max(maxArea, area(input[i], input[j]));
    return maxArea;
}

void check(int lineNumber, std::int64_t expectMaxArea, std::string_view source)
{
    if (const auto maxArea = solve(getInput(source));
        maxArea != expectMaxArea) {
        std::cerr
            << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                           lineNumber, expectMaxArea, maxArea)
            << std::endl;
    }
}

void runTests()
{
    const std::string_view example =
R"(7,1
11,1
11,7
9,7
9,5
2,5
2,3
7,3
)";
    check(__LINE__, 50, example);
}

int main()
{
    runTests();

    const auto maxArea = solve(getInput());
    std::cout << maxArea << std::endl;
    return 0;
}
