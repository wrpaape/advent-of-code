#include <array>
#include <cassert>
#include <charconv>
#include <cmath>
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

std::array<std::int64_t, 2> rotate(std::int64_t pos, std::int64_t rot)
{
    std::int64_t numClicks = 0;
    if (pos == 0) {
        numClicks = (rot == 0) ? 1 :  (abs(rot) / 100);
    }
    else if (rot < 0) {
        if (-rot >= pos) {
            const auto dist = -rot - pos, div = dist / 100;
            numClicks = 1 + div;
        }
    }
    else if (rot > 0) {
        if (const auto off = 100 - pos; rot >= off) {
            const auto dist = rot - off, div = dist / 100;
            numClicks = 1 + div;
        }
    }
    else {
        numClicks = (pos == 0);
    }
    pos += rot, pos %= 100, pos += 100, pos %= 100;
    return { pos, numClicks };
}

std::int64_t solve(std::span<const std::int64_t> input)
{
    std::int64_t pos = 50, pass = 0;
    for (const std::int64_t rot : input) {
        const auto [nPos, numClicks] = rotate(pos, rot);
        pos = nPos;
        pass += numClicks;
    }
    return pass;
}

void check(int          lineNumber,
           std::int64_t expectedNPos,
           std::int64_t expectedClicks,
           std::int64_t pos,
           std::int64_t rot)
{
    const auto [nPos, numClicks] = rotate(pos, rot);
    if (nPos != expectedNPos) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected nPos: " << expectedNPos
                  << "\n> actual nPos:   " << nPos
                  << std::endl;
    }
    if (expectedClicks != numClicks) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected clicks: " << expectedClicks
                  << "\n> actual clicks:   " << numClicks
                  << std::endl;
    }
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
    check(__LINE__, 82, 1, 50,  -68);
    check(__LINE__, 52, 0, 82,  -30);
    check(__LINE__, 0,  1, 52,  +48);
    check(__LINE__, 95, 0,  0,   -5);
    check(__LINE__, 0,  1,  0,    0);
    check(__LINE__, 0,  1,  0, +100);
    check(
        __LINE__,
        6,
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

    const auto numPairs = solve(getInput());
    std::cout << numPairs << std::endl;
    return 0;
}
