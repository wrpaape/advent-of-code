#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct TrieNode {
    bool                         d_isEnd;
    std::array<std::int16_t, 26> d_next;
    TrieNode()
    : d_isEnd(false)
    {
        d_next.fill(-1);
    }
};

auto getLines(std::istream& stream)
{
    std::vector<std::string> lines;
    for (std::string line; std::getline(stream, line); )
        lines.push_back(std::move(line));

    return lines;
}

auto getInput(std::istream& stream)
{
    std::pair<std::vector<std::string>, std::vector<std::string>> input;
    auto& [patterns, designs] = input;
    const std::vector<std::string> lines(getLines(stream));
    assert(lines.size() >= 2);
    assert(lines[1].empty());
    for (std::string::size_type pos = 0; pos < lines[0].size(); ) {
        const auto delim = std::min(lines[0].size(),
                                    lines[0].find(", ", pos + 1));
        patterns.push_back(lines[0].substr(pos, delim - pos));
        pos = delim + 2;
    }
    designs.assign(std::make_move_iterator(lines.begin()+2),
                   std::make_move_iterator(lines.end()));
    return input;
}

auto getInput()
{
    std::ifstream stream("day_19-linen_layout-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::vector<TrieNode> makeTrie(const std::vector<std::string>& patterns)
{
    std::vector<TrieNode> trie(1);
    for (const std::string& pattern : patterns) {
        std::int16_t n = 0;
        for (const char token : pattern) {
            std::int16_t nextN = trie.at(n).d_next[token-'a'];
            if (nextN < 0) {
                nextN = trie.size();
                trie.emplace_back();
                trie[n].d_next[token-'a'] = nextN;
            }
            n = nextN;
        }
        trie[n].d_isEnd = true;
    }
    return trie;
}

std::int64_t numWaysToMake(const std::vector<TrieNode>& trie,
                           const std::string&           design)
{
    std::vector<std::int64_t> numWays(design.size()+1);
    numWays[0] = 1;
    for (std::string::size_type pSize = 0; pSize < design.size(); ++pSize) {
        if (!numWays[pSize])
            continue;

        std::int16_t n = 0;
        for (std::string::size_type size = pSize + 1; size <= design.size();
             ++size) {
            n = trie[n].d_next[design[size-1]-'a'];
            if (n < 0)
                break;

            if (trie[n].d_isEnd)
                numWays[size] += numWays[pSize];
        }
    }
    return numWays[design.size()];
}

std::int64_t solve(const std::vector<std::string>& patterns,
                   const std::vector<std::string>& designs)
{

    return std::transform_reduce(
        designs.begin(), designs.end(), 0LL, std::plus{},
        [trie=makeTrie(patterns)](const std::string& design) {
            return numWaysToMake(trie, design);
        });
}

std::int64_t solve(
     const std::pair<std::vector<std::string>, std::vector<std::string>>& input)
{
    return solve(input.first, input.second);
}

void check(int                     lineNumber,
           std::int64_t            expectedNumDesigns,
           const std::string_view& input)
{
    const std::int64_t numDesigns = solve(getInput(input));
    if (numDesigns != expectedNumDesigns) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected: " << expectedNumDesigns
                  << "\n> actual:   " << numDesigns
                  << std::endl;
    }
}

void runTests()
{
    check(
        __LINE__,
        16,
        "r, wr, b, g, bwu, rb, gb, br\n"
        "\n"
        "brwrr\n"
        "bggr\n"
        "gbbr\n"
        "rrbgbr\n"
        "ubwu\n"
        "bwurrg\n"
        "brgr\n"
        "bbrgwb\n"
    );
}

int main()
{
    runTests();

    const auto numDesigns = solve(getInput());
    std::cout << numDesigns << std::endl;
    return 0;
}
