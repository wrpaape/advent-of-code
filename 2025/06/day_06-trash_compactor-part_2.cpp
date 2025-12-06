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

struct Expression {
    std::vector<std::int64_t> d_operands;
    char                      d_operator = 'X';
};

std::vector<std::string> rotate(std::span<const std::string> grid)
{
    const std::size_t m = grid.size(), n = grid.empty() ? 0 : grid[0].size();
    std::vector<std::string> rotated(n, std::string(m, 'X'));
    for (std::size_t i = 0; i < m; ++i)
        for (std::size_t j = 0; j < n; ++j)
            rotated[n-j-1][i] = grid[i][j];
    return rotated;
}


auto getInput(std::istream& stream)
{
    std::vector<Expression> input;
    std::vector<std::string> grid;
    for (std::string line; std::getline(stream, line); )
        grid.push_back(std::move(line));
    std::string last(std::move(grid.back()));
    grid.pop_back();
    for (std::size_t i = std::string::npos; ; --i) {
        i = last.find_last_of("+*", i);
        if (i == std::string::npos)
            break;
        input.emplace_back().d_operator = last[i];
        if (i == 0)
            break;
    }
    auto rows = rotate(grid);
    for (auto& row : rows)
        std::erase(row, ' ');
    rows.emplace_back();
    for (std::size_t i = 0, row = 0; i < input.size(); ++i, ++row) {
        assert(row < rows.size());
        for ( ; !rows[row].empty(); ++row) {
            const auto& num = rows[row];
            const auto begin = num.data(), end = begin + num.size();
            const auto [ptr, err] =
               std::from_chars(begin, end, input[i].d_operands.emplace_back());
            assert(ptr == end);
            assert(err == std::errc());
        }
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

std::vector<std::int64_t> convert(std::span<const std::string> operands)
{
    std::size_t maxMag = 0;
    for (const auto& num : operands)
        maxMag = std::max(maxMag, num.size());
    std::vector<std::int64_t> nums(maxMag);
    for (std::size_t mag = 0; mag < maxMag; ++mag) {
        for (const auto& num : operands) if (mag < num.size()) {
            nums[mag] *= 10, nums[mag] += (num.rbegin()[mag]-'0');
        }
    }
    return nums;
}

std::int64_t eval(const Expression& expression)
{
    switch(const auto& [ops, op] = expression; op) {
        case '+':
            return std::reduce(ops.begin(), ops.end(), 0L, std::plus{});
        case '*':
            return std::reduce(ops.begin(), ops.end(), 1L, std::multiplies{});
        default:
            break;
    }
    assert(!"invalid operator");
    std::unreachable();
}

std::int64_t solve(std::span<const Expression> input)
{
    return std::transform_reduce(input.begin(), input.end(), 0L,
                                 std::plus{}, &eval);
}

void check(int lineNumber, std::int64_t expectedTotal, std::string_view source)

{
    if (const auto total = solve(getInput(source)); total != expectedTotal) {
        std::cerr
            << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                           lineNumber, expectedTotal, total)
            << std::endl;
    }
}

void runTests()
{
    const std::string_view example =
R"(123 328  51 64 
 45 64  387 23 
  6 98  215 314
*   +   *   +  
)";
    check(__LINE__, 3263827, example);
}

int main()
{
    runTests();

    const auto total = solve(getInput());
    std::cout << total << std::endl;
    return 0;
}
