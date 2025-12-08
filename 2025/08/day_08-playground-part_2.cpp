#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <span>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

struct Point {
    std::int64_t d_x, d_y, d_z;
};

std::int64_t dSq(const Point& lhs, const Point& rhs)
{
    const auto dx = lhs.d_x - rhs.d_x;
    const auto dy = lhs.d_y - rhs.d_y;
    const auto dz = lhs.d_z - rhs.d_z;
    return dx*dx + dy*dy + dz*dz;
}

auto getInput(std::istream& stream)
{
    std::vector<Point> input;
    for (std::string line; std::getline(stream, line); ) {
        auto& p = input.emplace_back();
        const auto lineEnd = line.data() + line.size();
        const auto [ptr1, err1] = std::from_chars(line.data(), lineEnd, p.d_x);
        assert(ptr1 != lineEnd);
        assert(*ptr1 == ',');
        const auto [ptr2, err2] = std::from_chars(ptr1+1, lineEnd, p.d_y);
        assert(ptr2 != lineEnd);
        assert(*ptr2 == ',');
        const auto [ptr3, err3] = std::from_chars(ptr2+1, lineEnd, p.d_z);
        assert(ptr3 == lineEnd);
        assert(err1 == std::errc());
        assert(err2 == std::errc());
        assert(err3 == std::errc());
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

std::int64_t solve(std::span<const Point> input)
{
    assert(!input.empty());

    const std::int64_t n = input.size();
    std::vector<std::array<std::int64_t, 3>> edges;
    for (std::int64_t i = 0; i < n; ++i)
        for (std::int64_t j = i+1; j < n; ++j)
            edges.push_back({dSq(input[i], input[j]), i, j});
    std::vector<std::int64_t> comps(n, -1);
    const auto getC = [&](this auto&& getC, std::int64_t i) -> std::int64_t {
        return (comps[i] >= 0) ? comps[i] = getC(comps[i]) : i;
    };
    const auto join = [&](std::int64_t i1, std::int64_t i2) {
        std::int64_t c1 = getC(i1), c2 = getC(i2);
        if (c1 == c2)
            return false;
        if (comps[c2] < comps[c1])
            std::swap(c1, c2);
        comps[c1] += comps[c2];
        comps[c2] = c1;
        return true;
    };
    std::ranges::make_heap(edges, std::greater{});
    for (std::int64_t numC = n; ; ) {
        std::ranges::pop_heap(edges, std::greater{});
        const auto [_, i, j] = edges.back();
        edges.pop_back();
        if (join(i, j) && (--numC == 1))
            return input[i].d_x * input[j].d_x;
    }
}

void check(int lineNumber, std::int64_t expectProduct, std::string_view source)
{
    if (const auto product = solve(getInput(source));
        product != expectProduct) {
        std::cerr
            << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                           lineNumber, expectProduct, product)
            << std::endl;
    }
}

void runTests()
{
    const std::string_view example =
R"(162,817,812
57,618,57
906,360,560
592,479,940
352,342,300
466,668,158
542,29,236
431,825,988
739,650,466
52,470,668
216,146,977
819,987,18
117,168,530
805,96,715
346,949,466
970,615,88
941,993,340
862,61,35
984,92,344
425,690,689
)";
    check(__LINE__, 25272, example);
}

int main()
{
    runTests();

    const auto product = solve(getInput());
    std::cout << product << std::endl;
    return 0;
}
