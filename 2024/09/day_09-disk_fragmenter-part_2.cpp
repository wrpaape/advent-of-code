#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <tuple>
#include <vector>

auto getInput()
{
    std::string   line;
    std::ifstream stream("day_09-disk_fragmenter-input.txt");
    std::getline(stream, line);
    assert(stream);
    assert(!line.empty());
    return line;
}

int main()
{
    // [size] = { p1, p2, ... }
    std::string line = getInput();
    std::array<std::set<std::int64_t>, 10> holes;
    for (std::int64_t h = 1; h < line.size(); h += 2)
        if (const std::int64_t hSize = line[h]-'0'; hSize > 0)
            holes[hSize].emplace(h);

    std::vector<std::tuple<std::int64_t, std::int64_t, std::int64_t>> files;
    for (std::int64_t f = line.size() - 1; f >= 0; f -= 2) {
        const std::int64_t fSize = line[f]-'0';
        if (fSize == 0)
            continue;

        std::int64_t h = f, hSize = -1;
        for (std::int64_t size = fSize; size < holes.size(); ++size) {
            if (!holes[size].empty()) {
                if (*holes[size].begin() < h) {
                    h     = *holes[size].begin();
                    hSize = size;
                }
            }
        }
        if (hSize < 0) {
            files.emplace_back(f, f/2, fSize);
            continue;
        }
        files.emplace_back(h, f/2, fSize);
        holes[hSize].erase(holes[hSize].begin());
        hSize -= fSize;
        if (hSize > 0) {
            holes[hSize].emplace(h);
        }
        holes[fSize].emplace(f);
    }
    for (std::int64_t hSize = 1; hSize < holes.size(); ++hSize)
        for (const std::int64_t h : holes[hSize])
            files.emplace_back(h, -1, hSize);

    std::stable_sort(files.begin(), files.end(), [](const auto& lhs, const auto& rhs) {
                         return std::get<0>(lhs) < std::get<0>(rhs);
                     });
    std::uint64_t checkSum = 0, pos = 0;
    for (const auto [_, id, size] : files) {
        if (id >= 0)
            for (std::uint64_t i = 0; i < size; ++i)
                checkSum += (pos + i) * id;

        pos += size;
    }
    std::cout << checkSum << std::endl;
    return 0;
}
