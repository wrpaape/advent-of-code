#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stack>
#include <string>
#include <utility>
#include <vector>

auto getInput()
{
    std::vector<std::pair<std::uint64_t, std::vector<std::uint64_t>>> equations;
    std::ifstream stream("day_07-bridge_repair-input.txt");
    for (std::string line; std::getline(stream, line); ) {
        const char *c = line.c_str(), *const cEnd = c + line.size();
        auto& [result, operands] = equations.emplace_back();
        auto res = std::from_chars(c, cEnd, result);
        assert(res.ec == std::errc());
        assert(c < cEnd);
        for (c = res.ptr + 2;
             res = std::from_chars(c, cEnd, operands.emplace_back()),
             res.ptr != cEnd; c = res.ptr + 1) {
            assert(res.ec == std::errc());
            continue;
        }
        assert(res.ec == std::errc());
    }
    assert(!equations.empty());
    return equations;
}

std::uint64_t concatenate(std::uint64_t lhs, std::uint64_t rhs)
{
    std::uint64_t total = 0;
    do {
        lhs   *= 10;
        total *= 10;
        total += 9;
    } while (total < rhs);
    return lhs + rhs;
}

bool canSatisfy(std::uint64_t result, const std::vector<std::uint64_t>& operands)
{
    assert(!operands.empty());

    std::vector<std::uint64_t> prev, next;
    next.push_back(operands[0]);
    for (int i = 1; i < operands.size(); ++i) {
        std::swap(prev, next);
        next.clear();
        for (const auto prevSum : prev) {
            next.push_back(prevSum + operands[i]);
            next.push_back(prevSum * operands[i]);
            next.push_back(concatenate(prevSum, operands[i]));
        }
    }
    return std::find(next.begin(), next.end(), result) != next.end();
}

int main()
{
    std::uint64_t total = 0;
    for (const auto& [result, operations] : getInput())
        if (canSatisfy(result, operations))
            total += result;

    std::cout << total << std::endl;
    return 0;
}
