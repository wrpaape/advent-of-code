#include <cassert>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

std::pair<std::vector<int>, std::vector<int>> getInput()
{
    std::vector<std::string> lines;
    for (std::ifstream input("day_01-historian_hysteria-input.txt");
         std::getline(input, lines.emplace_back()); ) {
    }
    lines.pop_back();
    std::vector<int> lhs(lines.size()), rhs(lines.size());
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const char       *cursor  = lines[i].c_str(),
                   *const lineEnd = cursor + lines[i].size();
        const auto [ptr1, errc1] = std::from_chars(cursor, lineEnd, lhs[i]);
        assert(errc1 == std::errc());
        for (cursor = ptr1; isspace(*cursor); ++cursor)
            continue;
        
        const auto [_, errc2] = std::from_chars(cursor, lineEnd, rhs[i]);
        assert(errc2 == std::errc());
    }
    return { std::move(lhs), std::move(rhs) };
}

int main()
{
    auto [lhs, rhs] = getInput();
    unsigned long long totalSimilarity = 0; 
    std::unordered_map<int, int> rhsCounts;
    for (const auto num : rhs)
        ++rhsCounts[num];

    for (std::size_t i = 0; i < lhs.size(); ++i)
        totalSimilarity +=
                   static_cast<unsigned long long>(lhs[i]) * rhsCounts[lhs[i]];

    std::cout << totalSimilarity << std::endl;
    return 0;
}
