#include <cassert>
#include <numeric>
#include <functional>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

std::string getInput()
{
    std::ifstream      input("day_03-mull_it_over-input.txt");
    std::ostringstream str;
    str << input.rdbuf();
    return str.str();
}

int main()
{
    const std::string input = getInput();
    const std::regex  matcher("mul\\(([0-9]{1,3}),([0-9]{1,3})\\)");
    const std::sregex_iterator mBegin(input.begin(), input.end(), matcher), mEnd;
    const auto total = std::transform_reduce(mBegin, mEnd, 0ULL, std::plus{},
                                             [](const auto& match) {
        assert(match.size() == 3);
        return std::stoull(match[1]) * std::stoull(match[2]);
    });
    std::cout << total << std::endl;
    return 0;
}
