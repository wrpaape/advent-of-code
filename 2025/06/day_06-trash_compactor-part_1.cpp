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

auto getInput(std::istream& stream)
{
    std::vector<Expression> input;
    std::string line;
    std::getline(stream, line);
    assert(!line.empty());
    for (std::istringstream cols(line); 
        cols >> input.emplace_back().d_operands.emplace_back(); )
        continue;
    input.pop_back();
    const auto n = input.size();
    while (true) {
        std::getline(stream, line);
        assert(!line.empty());
        if (line.find_first_of("+*") != std::string::npos)
            break;
        std::istringstream cols(line);
        for (std::size_t i = 0; i < n; ++i)
            cols >> input[i].d_operands.emplace_back();
    }
    for (std::size_t i = 0, j = 0; i < n; ++i, ++j) {
        j = line.find_first_of("+*", j);
        assert(j != std::string::npos);
        input[i].d_operator = line[j];
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

std::int64_t eval(const Expression& expression)
{
    switch (const auto& [ops, op] = expression; op) {
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

void check(int               lineNumber,
           std::int64_t      expectedValue,
           const Expression& expression)
{
    if (const auto value = eval(expression); value != expectedValue) {
        std::cerr
            << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                           lineNumber, expectedValue, value)
            << std::endl;
    }
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
    check(__LINE__, 33210,   {{123,  45,   6}, '*'});
    check(__LINE__, 490,     {{328,  64,  98}, '+'});
    check(__LINE__, 4243455, {{ 51, 387, 215}, '*'});
    check(__LINE__, 401,     {{ 64,  23, 314}, '+'});
    check(__LINE__, 4277556, example);
}

int main()
{
    runTests();

    const auto total = solve(getInput());
    std::cout << total << std::endl;
    return 0;
}
