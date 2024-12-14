#include <algorithm>
#include <array>
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
#include <string_view>
#include <vector>

struct Vec2 {
    std::int64_t d_x, d_y;
};

struct Robot {
    Vec2 d_pos, d_vel;
};

typedef std::array<std::array<std::int64_t, 2>, 2> QuadrantCounter;

auto getInput(std::istream& stream)
{
    std::vector<Robot> robots;
    const std::regex robotMatcher("^p=([^,]+),([^ ]+) v=([^,]+),(.+)$");
    for (std::string line; std::getline(stream, line); ) {
        robots.emplace_back();
        std::smatch match;
        const bool haveMatch = std::regex_search(line, match, robotMatcher);
        assert(haveMatch);
        assert(match.size() == 5);
        robots.back().d_pos.d_x = std::stoll(match[1]);
        robots.back().d_pos.d_y = std::stoll(match[2]);
        robots.back().d_vel.d_x = std::stoll(match[3]);
        robots.back().d_vel.d_y = std::stoll(match[4]);
    }
    assert(!robots.empty());
    return robots;
}

auto getInput()
{
    // std::ifstream stream("day_14-restroom_redoubt-input-example.txt");
    std::ifstream stream("day_14-restroom_redoubt-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

void updateRobot(Robot&       robot,
                 std::int64_t numRows,
                 std::int64_t numCols)
{
    const auto  i = robot.d_pos.d_y,  j = robot.d_pos.d_x,
               vI = robot.d_vel.d_y, vJ = robot.d_vel.d_x;
    auto nI = (i + vI) % numRows,
         nJ = (j + vJ) % numCols;
    if (nI < 0) nI += numRows;
    if (nJ < 0) nJ += numCols;
    robot.d_pos.d_y = nI, robot.d_pos.d_x = nJ;
}

void runSim(std::vector<Robot>& robots,
            std::int64_t       numRows,
            std::int64_t       numCols,
            std::int64_t       numSeconds)
{
    for ( ; numSeconds > 0; --numSeconds)
        for (Robot& robot : robots)
            updateRobot(robot, numRows, numCols);
}

void countRobots(QuadrantCounter&          numRobots,
                 const std::vector<Robot>& robots,
                 std::int64_t              numRows,
                 std::int64_t              numCols)
{
    assert(numRows & 1);
    assert(numCols & 1);

    for (auto& row : numRobots)
        row.fill(0);

    const std::int64_t midRow = numRows/2, midCol = numCols/2;
    for (const Robot& robot : robots) {
        const auto i = robot.d_pos.d_y, j = robot.d_pos.d_x;
        numRobots[i > midRow][j > midCol] += ((i != midRow) & (j != midCol));
    }
}

std::string display(const std::vector<Robot>& robots,
                    std::int64_t              numRows,
                    std::int64_t              numCols)
{
    const auto lineSize = numCols+1;
    std::string output(numRows*lineSize, '.');
    for (auto i = numCols; i < output.size(); i += lineSize)
        output[i] = '\n';

    for (const Robot& robot : robots) {
        const auto i = robot.d_pos.d_y, j = robot.d_pos.d_x;
        if (auto& cell = output[i*lineSize+j]; cell == '.')
            cell = '1';
        else
            ++cell;
    }
    return output;
}

bool check(const std::string_view& expectedOutput,
           const std::string_view& input,
           std::int64_t            numRows,
           std::int64_t            numCols,
           std::int64_t            numSeconds)
{
    auto robots = getInput(input);
    runSim(robots, numRows, numCols, numSeconds);
    const auto output = display(robots, numRows, numCols);
    if (output != expectedOutput) {
        std::cerr << "failure:"
                  << "\n> expected:\n" << expectedOutput
                  << "\n> actual:\n" << output
                  << std::endl;
        return false;
    }
    return true;
}

void runTests()
{
    assert(check(
        "1.12.......\n"
        "...........\n"
        "...........\n"
        "......11.11\n"
        "1.1........\n"
        ".........1.\n"
        ".......1...\n",
        "p=0,4 v=3,-3\n"
        "p=6,3 v=-1,-3\n"
        "p=10,3 v=-1,2\n"
        "p=2,0 v=2,-1\n"
        "p=0,0 v=1,3\n"
        "p=3,0 v=-2,-2\n"
        "p=7,6 v=-1,-3\n"
        "p=3,0 v=-1,-2\n"
        "p=9,3 v=2,3\n"
        "p=7,3 v=-1,2\n"
        "p=2,4 v=2,-3\n"
        "p=9,5 v=-3,-3\n",
        7, 11,
        0
    ));
    assert(check(
        "...........\n"
        "...........\n"
        "...........\n"
        "...........\n"
        "..1........\n"
        "...........\n"
        "...........\n",
        "p=2,4 v=2,-3\n",
        7, 11,
        0
    ));
    assert(check(
        "...........\n"
        "....1......\n"
        "...........\n"
        "...........\n"
        "...........\n"
        "...........\n"
        "...........\n",
        "p=2,4 v=2,-3\n",
        7, 11,
        1
    ));
    assert(check(
        "...........\n"
        "...........\n"
        "...........\n"
        "...........\n"
        "...........\n"
        "......1....\n"
        "...........\n",
        "p=2,4 v=2,-3\n",
        7, 11,
        2
    ));
    assert(check(
        "...........\n"
        "...........\n"
        "........1..\n"
        "...........\n"
        "...........\n"
        "...........\n"
        "...........\n",
        "p=2,4 v=2,-3\n",
        7, 11,
        3
    ));
    assert(check(
        "...........\n"
        "...........\n"
        "...........\n"
        "...........\n"
        "...........\n"
        "...........\n"
        "..........1\n",
        "p=2,4 v=2,-3\n",
        7, 11,
        4
    ));
    assert(check(
        "...........\n"
        "...........\n"
        "...........\n"
        ".1.........\n"
        "...........\n"
        "...........\n"
        "...........\n",
        "p=2,4 v=2,-3\n",
        7, 11,
        5
    ));
    assert(check(
        "......2..1.\n"
        "...........\n"
        "1..........\n"
        ".11........\n"
        ".....1.....\n"
        "...12......\n"
        ".1....1....\n",
        "p=0,4 v=3,-3\n"
        "p=6,3 v=-1,-3\n"
        "p=10,3 v=-1,2\n"
        "p=2,0 v=2,-1\n"
        "p=0,0 v=1,3\n"
        "p=3,0 v=-2,-2\n"
        "p=7,6 v=-1,-3\n"
        "p=3,0 v=-1,-2\n"
        "p=9,3 v=2,3\n"
        "p=7,3 v=-1,2\n"
        "p=2,4 v=2,-3\n"
        "p=9,5 v=-3,-3\n",
        7, 11,
        100
    ));
}

int main()
{
    // runTests();
    std::vector<Robot> robots = getInput();
    // std::int64_t numRows = 7, numCols = 11, numSeconds = 100;
    std::int64_t numRows = 103, numCols = 101, numSeconds = 100;
    runSim(robots, numRows, numCols, numSeconds);
    QuadrantCounter q;
    countRobots(q, robots, numRows, numCols);
    // std::cout <<   "Q1=" << q[0][0]
    //           << ", Q2=" << q[0][1]
    //           << ", Q3=" << q[1][0]
    //           << ", Q4=" << q[1][1]
    //           << std::endl;
    const auto total = q[0][0] * q[0][1] * q[1][0] * q[1][1];
    std::cout << total << std::endl;
    return 0;
}
