#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

auto getInput()
{
    std::vector<std::uint64_t> stones;
    std::ifstream stream("day_11-plutonian_pebbles-input.txt");
    const std::istream_iterator<std::uint64_t> sBegin(stream), sEnd;
    std::copy(sBegin, sEnd, std::back_inserter(stones));
    return stones;
}

std::vector<std::uint64_t> makeMags()
{
    std::vector<std::uint64_t> mags;
    for (std::uint64_t pMag = 0, mag = 1; mag > pMag; pMag = mag, mag *= 10)
        mags.push_back(mag);

    return mags;
}

std::uint64_t numDigits(const std::vector<std::uint64_t>& mags,
                        std::uint64_t                     stone)
{
    return std::distance(mags.begin(),
                         std::upper_bound(mags.begin(), mags.end(),
                                          stone + (stone == 0)));
}

std::uint64_t magForDigits(const std::vector<std::uint64_t>& mags,
                           std::uint64_t                     numDigits)
{
    assert(numDigits > 0);

    return mags[numDigits-1];
}

int main()
{
    std::vector<std::uint64_t> stones = getInput(), pStones, mags = makeMags();
    for (int remIterations = 25; remIterations > 0; --remIterations) {
        std::swap(stones, pStones);
        stones.clear();
        for (const std::uint64_t pStone : pStones) {
            if (pStone == 0) {
                stones.push_back(1);
            }
            else if (const std::uint64_t numDigs = numDigits(mags, pStone);
                     (numDigs & 1) == 0) {
                const std::uint64_t mag = magForDigits(mags, (numDigs / 2) + 1),
                                    s1  = pStone / mag,
                                    s2  = pStone % mag;
                stones.push_back(s1);
                stones.push_back(s2);
            }
            else {
                stones.push_back(pStone * 2024);
            }
        }
    }
    std::cout << stones.size() << std::endl;
    return 0;
}
