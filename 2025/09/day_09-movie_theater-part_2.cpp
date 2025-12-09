#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <span>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

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
    const auto dx = std::abs(lhs.d_x - rhs.d_x) + 1;
    const auto dy = std::abs(lhs.d_y - rhs.d_y) + 1;
    return dx * dy;
}

std::int64_t solve(std::span<const Point> input)
{
    assert(input.size() >= 4);

    std::unordered_set<int> xs, ys;
    for (const auto [x, y] : input)
        xs.insert(x), ys.insert(y);

    std::vector<std::int64_t> xSorted(xs.begin(), xs.end()),
                              ySorted(ys.begin(), ys.end());
    std::ranges::sort(xSorted);
    std::ranges::sort(ySorted);
    const auto indexFor = [](std::span<const std::int64_t> sorted, std::int64_t coord) {
        const auto it = std::ranges::lower_bound(sorted, coord);
        assert(*it == coord);
        return std::distance(sorted.begin(), it);
    };
    const auto iForX = [&](std::int64_t x) { return indexFor(xSorted, x); };
    const auto jForY = [&](std::int64_t y) { return indexFor(ySorted, y); };
    const std::int64_t N = input.size(), m = xSorted.size(), n = ySorted.size();
    std::vector<std::vector<bool>> grid(m, std::vector<bool>(n));
    for (std::int64_t i = 0; i < N; ++i) {
        const auto p1 = input[i], p2 = input[(i+1)%N];
        if (p1.d_x == p2.d_x) {
            const auto [y1, y2] = std::minmax(p1.d_y, p2.d_y);
            const auto i = iForX(p1.d_x), j1 = jForY(y1), j2 = jForY(y2);
            for (auto j = j1; j <= j2; ++j)
                grid[i][j] = true;
        }
        else if (p1.d_y == p2.d_y) {
            const auto [x1, x2] = std::minmax(p1.d_x, p2.d_x);
            const auto i1 = iForX(x1), i2 = iForX(x2), j = jForY(p1.d_y);
            for (auto i = i1; i <= i2; ++i)
                grid[i][j] = true;
        }
        else {
            assert(!"bad input");
            std::abort();
        }
    }
    const auto floodFill = [&](this auto&& floodFill, std::int64_t i, std::int64_t j) -> void {
        grid[i][j] = true;
        constexpr std::int64_t k_Dir[] = { 1, 0, -1, 0, 1 };
        for (std::int64_t d = 0; d < 4; ++d)
            if (const auto nI = i + k_Dir[d], nJ = j + k_Dir[d+1];
                (nI >= 0) && (nI < m) &&
                (nJ >= 0) && (nJ < n) &&
                !grid[nI][nJ])
                floodFill(nI, nJ);
    };
    floodFill(m/2, n/2);
    std::int64_t maxArea = 0;
    for (std::int64_t I = 0; I < N; ++I) {
        for (std::int64_t J = I+1; J < N; ++J) {
            const auto [x1, y1] = input[I];
            const auto [x2, y2] = input[J];
            const auto [xMin, xMax] = std::minmax({x1, x2});
            const auto [yMin, yMax] = std::minmax({y1, y2});
            const std::int64_t i1 = iForX(xMin), i2 = iForX(xMax);
            const std::int64_t j1 = jForY(yMin), j2 = jForY(yMax);
            for (std::int64_t i = i1; i <= i2; ++i) {
                for (std::int64_t j = j1; j <= j2; ++j) {
                    if (!grid[i][j]) {
                        goto nextPair;
                    }
                }
            }
            maxArea = std::max(maxArea, area(input[I], input[J]));
            nextPair: ;
        }
    }
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
// ..............
// .......#XXX#..
// .......XXXXX..
// ..#XXXX#XXXX..
// ..XXXXXXXXXX..
// ..#XXXXXX#XX..
// .........XXX..
// .........#X#..
// ..............
    check(__LINE__, 24, example);
}

int main()
{
    runTests();

    const auto maxArea = solve(getInput());
    std::cout << maxArea << std::endl;
    return 0;
}
