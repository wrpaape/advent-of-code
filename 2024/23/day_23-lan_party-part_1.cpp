#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
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

std::int64_t solve(
   const std::vector<std::pair<std::string, std::string>>&  edges,
   std::int64_t                                             setSize,
   char                                                     key,
   std::string                                             *sets      = nullptr,
   std::string                                             *matchSets = nullptr)
{
    const auto nodes = extractNodes(edges);
    const auto graph = buildGraph(nodes, edges);
    std::size_t numMatchSets = 0;
    const auto appendSet = [&](std::string *sets,
                               std::size_t i, std::size_t j, std::size_t k) {
        if (sets) {
            *sets += nodes[i];
            *sets += ',';
            *sets += nodes[j];
            *sets += ',';
            *sets += nodes[k];
            *sets += '\n';
        }
    };
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        for (std::size_t j = i+1; j < nodes.size(); ++j) if (graph[i][j]) {
            for (std::size_t k = j+1; k < nodes.size(); ++k) {
                if (graph[k][i] && graph[k][j]) {
                    appendSet(sets, i, j, k);
                    if (nodes[i].starts_with(key) ||
                        nodes[j].starts_with(key) ||
                        nodes[k].starts_with(key)) {
                        ++numMatchSets;
                        appendSet(matchSets, i, j, k);
                    }
                }
            }
        }
    }
    return numMatchSets;
}

void check(int                     lineNumber,
           std::int64_t            expectedNumMatchSets,
           const std::string_view& expectedMatchSets,
           const std::string_view& expectedSets,
           const std::string_view& source)
{
    std::string sets, matchSets;
    if (const auto numMatchSets = solve(getInput(source), 3, 't',
                                        &sets, &matchSets);
        numMatchSets != expectedNumMatchSets) {
        std::cerr << "failure(" << lineNumber << ") - num match sets:"
                  << "\n> expected: " << expectedNumMatchSets
                  << "\n> actual:   " << numMatchSets
                  << std::endl;
    }
    if (matchSets != expectedMatchSets) {
        std::cerr << "failure(" << lineNumber << ") - match sets:"
                  << "\n> expected: " << expectedMatchSets
                  << "\n> actual:   " << matchSets
                  << std::endl;
    }
    if (sets != expectedSets) {
        std::cerr << "failure(" << lineNumber << ") - sets:"
                  << "\n> expected: " << expectedSets
                  << "\n> actual:   " << sets
                  << std::endl;
    }
}

void runTests()
{
    check(__LINE__,
          7,
          "co,de,ta\n"
          "co,ka,ta\n"
          "de,ka,ta\n"
          "qp,td,wh\n"
          "tb,vc,wq\n"
          "tc,td,wh\n"
          "td,wh,yn\n",
          "aq,cg,yn\n"
          "aq,vc,wq\n"
          "co,de,ka\n"
          "co,de,ta\n"
          "co,ka,ta\n"
          "de,ka,ta\n"
          "kh,qp,ub\n"
          "qp,td,wh\n"
          "tb,vc,wq\n"
          "tc,td,wh\n"
          "td,wh,yn\n"
          "ub,vc,wq\n",
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

    const auto numSets = solve(getInput(), 3, 't');
    std::cout << numSets << std::endl;
    return 0;
}
