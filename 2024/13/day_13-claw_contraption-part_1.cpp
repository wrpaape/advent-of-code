#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

struct Vec2 {
    std::int64_t d_x, d_y;
};

struct Machine {
    Vec2 d_a, d_b, d_prize;
};

auto getInput(std::istream& stream)
{
    std::vector<Machine> machines;
    const std::regex buttonMatcher("^Button ([AB]): X\\+(\\d+), Y\\+(\\d+)$"),
                     prizeMatcher( "^Prize: X=(\\d+), Y=(\\d+)$");
    while (stream) {
        machines.emplace_back();
        std::string line;
        bool        haveMatch;
        std::smatch match;

        std::getline(stream, line);
        assert(stream);
        haveMatch = std::regex_search(line, match, buttonMatcher);
        assert(haveMatch);
        assert(match.size() == 4);
        assert(match[1] == 'A');
        machines.back().d_a.d_x = std::stoll(match[2]);
        machines.back().d_a.d_y = std::stoll(match[3]);

        std::getline(stream, line);
        assert(stream);
        haveMatch = std::regex_search(line, match, buttonMatcher);
        assert(haveMatch);
        assert(match.size() == 4);
        assert(match[1] == 'B');
        machines.back().d_b.d_x = std::stoll(match[2]);
        machines.back().d_b.d_y = std::stoll(match[3]);

        std::getline(stream, line);
        assert(stream);
        haveMatch = std::regex_search(line, match, prizeMatcher);
        assert(haveMatch);
        assert(match.size() == 3);
        machines.back().d_prize.d_x = std::stoll(match[1]);
        machines.back().d_prize.d_y = std::stoll(match[2]);

        std::getline(stream, line);
        assert(line.empty());
    }
    assert(!machines.empty());
    return machines;
}

auto getInput()
{
    std::ifstream stream("day_13-claw_contraption-input.txt");
    return getInput(stream);
}

void minCoins(std::uint64_t& numA, std::uint64_t& numB, const Machine& machine)
{
    const std::int64_t C = machine.d_a.d_x,
                       D = machine.d_b.d_x,
                       E = machine.d_a.d_y,
                       F = machine.d_b.d_y,
                       X = machine.d_prize.d_x,
                       Y = machine.d_prize.d_y;
    assert(C > 0);
    assert(D > 0);
    assert(E > 0);
    assert(F > 0);
    assert(X > 0);
    assert(Y > 0);

    numA = 0, numB = 0;

    // Check if solution to equations
    //..
    //  C*a + D*b = X
    //  E*a + F*b = Y
    //..
    // are coincident ('a'- and 'b'-intercepts are equal):
    //..
    // (X/D == Y/F) && (X/C == Y/E)
    //..

    if (((X*F) == (Y*D)) && ((X*E) == (Y*C))) {
        // minimize
        //..
        //  3*a + b
        //..

        if (C >= 3*D) {
            // more cost-effective to maximize 'a'
            const std::int64_t a = X/C, b = (Y - E*a)/F;
            numA = a, numB = b;
        }
        else {
            // more cost-effective to maximize 'b'
            const std::int64_t b = X/D, a = (Y - F*b)/E;
            numA = a, numB = b;
        }
    }
    else {
        // Check if single solution exists (intersection):
        //
        //..
        //  b = (Y - E*a)/F
        //  C*a + D*(Y - E*a)/F = X
        //  C*F*a + D*Y - D*E*a = F*X
        //  (C*F - D*E)*a = F*X - D*Y
        //  a = (F*X - D*Y)/(C*F - D*E)
        //..

        const std::int64_t aNumerator   = F*X - D*Y,
                           aDenominator = C*F - D*E;
        if (aDenominator == 0)
            return;

        const std::int64_t a          = aNumerator / aDenominator,
                           aRemainder = aNumerator % aDenominator;
        if ((a < 0) || (aRemainder != 0))
            return;

        const std::int64_t bNumerator   = Y - E*a,
                           bDenominator = F;
        if (bDenominator == 0)
            return;

        const std::int64_t b          = bNumerator / bDenominator,
                           bRemainder = bNumerator % bDenominator;
        if ((b < 0) || (bRemainder != 0))
            return;

        numA = a, numB = b;
    }
}

bool check(std::uint64_t      expectedNumA,
           std::uint64_t      expectedNumB,
           const std::string& input)
{
    std::istringstream stream(input);
    const auto machines = getInput(stream);
    if (machines.size() != 1) {
        std::cerr << "bad input:\n" << input << std::endl;
        return false;
    }
    std::uint64_t numA = 0, numB = 0;
    minCoins(numA, numB, machines[0]);
    if ((numA != expectedNumA) || (numB != expectedNumB)) {
        std::cerr << "failure:\n" << input << "\n"
                     "expected: (" << expectedNumA << ", " << expectedNumB << ")\n"
                     "actual: (" << numA << ", " << numB << ")\n"
                  << std::endl;
        return false;
    }
    return true;
}

void runTests()
{
    assert(check(0,1, "Button A: X+1, Y+1\n"
                      "Button B: X+9, Y+9\n"
                      "Prize: X=9, Y=9\n"));

    assert(check(1,2, "Button A: X+1, Y+1\n"
                      "Button B: X+4, Y+4\n"
                      "Prize: X=9, Y=9\n"));

    assert(check(1,4, "Button A: X+1, Y+1\n"
                      "Button B: X+2, Y+2\n"
                      "Prize: X=9, Y=9\n"));

    assert(check(0,9, "Button A: X+1, Y+1\n"
                      "Button B: X+1, Y+1\n"
                      "Prize: X=9, Y=9\n"));

    assert(check(1,0, "Button A: X+9, Y+9\n"
                      "Button B: X+1, Y+1\n"
                      "Prize: X=9, Y=9\n"));

    assert(check(2,1, "Button A: X+4, Y+4\n"
                      "Button B: X+1, Y+1\n"
                      "Prize: X=9, Y=9\n"));

    assert(check(0,9, "Button A: X+2, Y+2\n"
                      "Button B: X+1, Y+1\n"
                      "Prize: X=9, Y=9\n"));

    assert(check(0,9, "Button A: X+1, Y+1\n"
                      "Button B: X+1, Y+1\n"
                      "Prize: X=9, Y=9\n"));
}

int main()
{
    runTests();
    const std::vector<Machine> machines = getInput();
    std::uint64_t total = 0;
    for (const Machine& machine : machines) {
        std::uint64_t numA = 0, numB = 0;
        minCoins(numA, numB, machine);
        total += (3*numA + numB);
    }
    std::cout << total << std::endl;
    return 0;
}
