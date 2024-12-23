#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

auto getInput(std::istream& stream)
{
    std::vector<std::pair<std::string, std::string>> edges;
    for (std::string line; std::getline(stream, line); ) {
        const auto delim = line.find('-');
        assert(delim != std::string::npos);
        edges.emplace_back(line.substr(0, delim), line.substr(delim+1));
    }
    return edges;
}

auto getInput()
{
    std::ifstream stream("day_23-lan_party-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::vector<std::string> extractNodes(
                 const std::vector<std::pair<std::string, std::string>>& edges)
{
    std::vector<std::string> nodes(2*edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i) {
        nodes[2*i]   = edges[i].first;
        nodes[2*i+1] = edges[i].second;
    }
    std::sort(nodes.begin(), nodes.end());
    nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());
    return nodes;
}

std::int64_t getN(const std::vector<std::string>& nodes,
                  const std::string&              node)
{
    assert(std::binary_search(nodes.begin(), nodes.end(), node));

    return std::distance(nodes.begin(),
                         std::lower_bound(nodes.begin(), nodes.end(), node));
}

std::vector<std::vector<bool>> buildGraph(
                 const std::vector<std::string>&                         nodes,
                 const std::vector<std::pair<std::string, std::string>>& edges)
{
    std::vector<std::vector<bool>> graph(nodes.size(),
                                         std::vector<bool>(nodes.size()));
    for (const auto& [node1, node2] : edges) {
        const auto n1 = getN(nodes, node1), n2 = getN(nodes, node2);
        graph[n1][n2] = true;
        graph[n2][n1] = true;
    }
    return graph;
}

//   A B C D E
// A 0 1 0 0 0
// B 1 1 1 0 1
// C 0 1 1 0 1
// D 0 0 0 0 0
// E 0 1 1 0 1
//
// A: B
// B: A,B,C,E
// C: B,C,E
// D:
// E: B,C,E
//
// ka-co
//
// [0][1]
//
// ta-co
// de-co
// ta-ka
// de-ta
// ka-de

std::string solve(
                 const std::vector<std::pair<std::string, std::string>>& edges)
{
    const auto nodes = extractNodes(edges);
    const auto graph = buildGraph(nodes, edges);
    const auto n     = nodes.size();
    std::vector<bool> mSet(n), iSet(n), oSet(n);
    for (std::size_t i = 0; i < n; ++i) {
        std::fill(iSet.begin(), iSet.end(), false);
        iSet[i] = true;
        for (std::size_t j = 0; j < n; ++j) if (graph[i][j]) {
            std::transform(iSet.begin(), iSet.end(),
                           graph[j].begin(),
                           oSet.begin(),
                           std::logical_and{});
            if (oSet == iSet)
                iSet[j] = true;
        }
        if (std::count(iSet.begin(), iSet.end(), true) >
            std::count(mSet.begin(), mSet.end(), true))
            std::swap(mSet, iSet);
    }
    std::string password;
    const char *sep = "";
    for (std::size_t i = 0; i < n; ++i) if (mSet[i]) {
        password += sep;
        password += nodes[i];
        sep = ",";
    }
    return password;
}

void check(int                     lineNumber,
           const std::string_view& expectedPassword,
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
    check(__LINE__,
          "co,de,ka,ta",
          "kh-tc\n"
          "qp-kh\n"
          "de-cg\n"
          "ka-co\n"
          "yn-aq\n"
          "qp-ub\n"
          "cg-tb\n"
          "vc-aq\n"
          "tb-ka\n"
          "wh-tc\n"
          "yn-cg\n"
          "kh-ub\n"
          "ta-co\n"
          "de-co\n"
          "tc-td\n"
          "tb-wq\n"
          "wh-td\n"
          "ta-ka\n"
          "td-qp\n"
          "aq-cg\n"
          "wq-ub\n"
          "ub-vc\n"
          "de-ta\n"
          "wq-aq\n"
          "wq-vc\n"
          "wh-yn\n"
          "ka-de\n"
          "kh-ta\n"
          "co-tc\n"
          "wh-qp\n"
          "tb-vc\n"
          "td-yn\n");
}

int main()
{
    runTests();

    const auto password = solve(getInput());
    std::cout << password << std::endl;
    return 0;
}
