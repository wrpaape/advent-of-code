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
    const std::regex  matcher("mul\\(([0-9]{1,3}),([0-9]{1,3})\\)|do\\(\\)|don't\\(\\)");
    unsigned long long total = 0;
    bool enabled = true;
    for (std::sregex_iterator m(input.begin(), input.end(), matcher), mEnd; m != mEnd; ++m) {
        assert(m->size() == 3);

        const auto matchStr = m->str();
        if (matchStr == "do()") {
            enabled = true;
        }
        else if (matchStr == "don't()") {
            enabled = false;
        }
        else if (enabled) {
            total += (std::stoull((*m)[1]) * std::stoull((*m)[2]));
        }
    }
    std::cout << total << std::endl;
    return 0;
}
