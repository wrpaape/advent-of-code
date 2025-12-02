#include <cassert>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <span>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

auto getInput(std::istream& stream)
{
    std::vector<std::int64_t> input;
    for (std::string line; std::getline(stream, line); ) {
        assert(line.size() >= 2);
        const auto [_ptr, err] = std::from_chars(line.data()+1,
                                                 line.data()+line.size(),
                                                 input.emplace_back());
        assert(err == std::errc());
        switch (line[0]) {
            case 'L':
                input.back() = -input.back();
                break;
            case 'R':
                break;
            default:
                std::unreachable();
        }
    }
    return input;
}

auto getInput()
{
    std::ifstream stream("day_01-secret_entrance-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::int64_t solve(std::span<const std::int64_t> input)
{
    std::int64_t pos = 50, pass = 0;
    for (std::int64_t rot : input)
        pos += rot, pos %= 100, pass += (pos == 0);
    return pass;
}

void check(int                     lineNumber,
           std::int64_t            expectedPassword,
           const std::string_view& source)
{
    if (const auto password = solve(getInput(source));
        password != expectedPassword) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedPassword
                  << "\n> actual:   " << password
                  << std::endl;
    }
}

void runTests()
{
    check(
        __LINE__,
        3,
        "L68\n"
        "L30\n"
        "R48\n"
        "L5\n"
        "R60\n"
        "L55\n"
        "L1\n"
        "L99\n"
        "R14\n"
        "L82\n"
    );
}

int main()
{
    runTests();

    const auto password = solve(getInput());
    std::cout << password << std::endl;
    return 0;
}
