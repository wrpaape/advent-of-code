#include <cassert>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

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
    std::uint64_t checkSum = 0, pos = 0;
    std::string line = getInput();
    for (std::uint64_t l = 0, r = line.size()-1; ; l += 2) {
        for (std::uint64_t lRem = line[l]-'0', lId = l / 2; lRem > 0; --lRem)
            checkSum += lId * pos++;

        if (l >= r)
            break;

        for (std::uint64_t sRem = line[l+1]-'0'; sRem > 0; ) {
            while ((l < r) && (line[r] == '0'))
                r -= 2;

            if (l >= r)
                break;

            auto rRem = std::min<std::uint64_t>(sRem, line[r]-'0');
            sRem    -= rRem;
            line[r] -= rRem;
            for (const std::uint64_t rId = r / 2; rRem > 0; --rRem)
                checkSum += rId * pos++;
        }
    }
    std::cout << checkSum << std::endl;
    return 0;
}
