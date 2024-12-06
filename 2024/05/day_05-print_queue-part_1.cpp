#include <algorithm>
#include <cassert>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

std::pair<std::vector<std::pair<int, int>>,
          std::vector<std::vector<int>>> getInput()
{
    std::pair<std::vector<std::pair<int, int>>,
              std::vector<std::vector<int>>> result;
    auto& [edges, orderings] = result;
    std::ifstream input("day_05-print_queue-input.txt");
    std::string   line;
    while (std::getline(input, line) && !line.empty()) {
        const char *c = line.c_str(), *const cEnd = c + line.size();
        auto& [prev, next] = edges.emplace_back();
        const auto [ptr1, ec1] = std::from_chars(c, cEnd, prev);
        assert(ec1 == std::errc());
        const auto [ptr2, ec2] = std::from_chars(ptr1+1, cEnd, next);
        assert(ec2 == std::errc());
    }
    while (std::getline(input, line)) {
        const char *c = line.c_str(), *const cEnd = c + line.size();
        auto& ordering = orderings.emplace_back();
        while (true) {
            int num;
            const auto [ptr, ec] = std::from_chars(c, cEnd, num);
            assert(ec == std::errc());
            ordering.emplace_back(num);
            if (ptr == cEnd)
                break;

            c = ptr+1;
        }
    }
    return result;
}

bool isOrdered(const std::unordered_map<int, std::vector<int>>& graph,
               const std::vector<int>&                          ordering)
{
    std::unordered_map<int, int> ingress;
    for (const int num : ordering)
        ingress[num] = 0;

    for (const int num : ordering)
        if (auto foundNext = graph.find(num); foundNext != graph.end())
            for (const int next : foundNext->second)
                if (auto it = ingress.find(next); it != ingress.end())
                    ++it->second;

    for (const int num : ordering) {
        if (ingress[num] != 0)
            return false;

        const auto foundNext = graph.find(num);
        if (foundNext == graph.end())
            continue;

        for (const int next : foundNext->second)
            if (auto it = ingress.find(next); it != ingress.end())
                --it->second;
    }
    return true;
}

int main()
{
    const auto [edges, orderings] = getInput();
    std::unordered_map<int, std::vector<int>> graph;
    for (const auto [prev, next] : edges)
        graph[prev].emplace_back(next);

    int total = 0;
    for (const auto& ordering : orderings)
        if (isOrdered(graph, ordering))
            total += *std::midpoint(ordering.data(),
                                    ordering.data() + ordering.size());

    std::cout << total << std::endl;
    return 0;
}
