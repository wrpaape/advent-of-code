#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

struct Vec2 {
    std::int64_t d_x, d_y;
    auto operator<=>(const Vec2&) const = default;
};

struct Robot {
    Vec2 d_pos, d_vel;
    auto operator<=>(const Robot&) const = default;
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

void updateRobots(std::vector<Robot>& robots,
                  std::int64_t        numRows,
                  std::int64_t        numCols)
{
    for (Robot& robot : robots)
        updateRobot(robot, numRows, numCols);
}

void runSim(std::vector<Robot>& robots,
            std::int64_t       numRows,
            std::int64_t       numCols,
            std::int64_t       numSeconds)
{
    for ( ; numSeconds > 0; --numSeconds)
        updateRobots(robots, numRows, numCols);
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
    std::string output(numRows*lineSize, ' ');
    for (auto i = numCols; i < output.size(); i += lineSize)
        output[i] = '\n';

    for (const Robot& robot : robots) {
        const auto i = robot.d_pos.d_y, j = robot.d_pos.d_x;
        auto& cell = output[i*lineSize+j];
        cell = '#';
    }
    return output;
}

bool haveOverlap(const std::vector<Robot>& robots)
{
    std::set<Vec2> pos;
    std::transform(robots.begin(), robots.end(), std::inserter(pos, pos.end()),
                   [](const Robot& robot) { return robot.d_pos; });
    return pos.size() != robots.size();
}

int main()
{
    // runTests();
    std::vector<Robot> robots = getInput();
    std::int64_t numRows = 103, numCols = 101;
    const auto disp   = [&]() { return display(robots, numRows, numCols); };
    const auto update = [&]() { updateRobots(robots, numRows, numCols); };
    std::map<std::vector<Robot>, std::int64_t> state;
    state[robots] = 0;
    std::int64_t collisionTime = 0;
    do {
        update();
    } while (state.emplace(robots, ++collisionTime).second);
    std::cout << "collision: " << state[robots] << "=>" << collisionTime << std::endl;

    const std::int64_t onset = 84, stride = 101;
    std::int64_t t;
    for (t = state[robots]; t < onset; ++t)
        update();
    do {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
        std::cout << "\x1B[2J\x1B[H"
                  << "t=" << t << std::string(numCols, '=') << '\n'
                  << disp() << std::endl;
       for (auto nextT = t + stride; t < nextT; ++t)
           update();
    } while (t <= collisionTime);
    return 0;
}
