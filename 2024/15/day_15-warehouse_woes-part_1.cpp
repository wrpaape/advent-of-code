#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct Cursor {
    std::int64_t d_i, d_j;
};

auto getInput(std::istream& stream)
{
    std::pair<std::vector<std::string>, std::string> input;
    auto& [grid, commands] = input;
    while (std::getline(stream, grid.emplace_back()) && !grid.back().empty())
        continue;

    assert(stream);
    assert(!grid.empty());
    grid.pop_back();
    assert(!grid.empty());

    for (std::string line; std::getline(stream, line); )
        commands += std::move(line);

    assert(!commands.empty());
    return input;
}

auto getInput()
{
    std::ifstream stream("day_15-warehouse_woes-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

Cursor findRobot(const std::vector<std::string>& grid)
{
    for (std::int64_t i = 0; i < grid.size(); ++i)
        for (std::int64_t j = 0; j < grid[i].size(); ++j)
            if (grid[i][j] == '@')
                return { .d_i = i, .d_j = j };

    assert(!"missing robot");
    std::unreachable();
}

constexpr std::int64_t Dir[] = { 0, -1, 0, +1, 0 };

std::int64_t dirFromCommand(char command)
{
    switch (command) {
        case '<': return 0;
        case '^': return 1;
        case '>': return 2;
        case 'v': return 3;
    }
    assert(!"invalid command");
    std::unreachable();
}

void update(std::vector<std::string>& grid, Cursor& robot, char command)
{
    assert(grid[robot.d_i][robot.d_j] == '@');

    const std::int64_t d = dirFromCommand(command), di = Dir[d], dj = Dir[d+1];
    std::int64_t i = robot.d_i, j = robot.d_j, numSteps = 0;
    do {
        i += di, j += dj, ++numSteps;
    } while (grid[i][j] == 'O');
    if (grid[i][j] == '#')
        return;

    assert(grid[i][j] == '.');
    if (numSteps > 1)
        grid[i][j] = 'O';

    grid[robot.d_i][robot.d_j] = '.';
    robot.d_i += di, robot.d_j += dj;
    grid[robot.d_i][robot.d_j] = '@';
}

std::int64_t sumBoxes(const std::vector<std::string>& grid)
{
    std::int64_t total = 0;
    for (std::int64_t i = 0; i < grid.size(); ++i)
        for (std::int64_t j = 0; j < grid[i].size(); ++j)
            if (grid[i][j] == 'O')
                total += (100 * i + j);

    return total;
}

std::int64_t runSim(std::vector<std::string>& grid,
                    const std::string_view&   commands)
{
    auto robot = findRobot(grid);
    for (const char command : commands)
        update(grid, robot, command);

    return sumBoxes(grid);
}

std::string display(const std::vector<std::string>& grid)
{
    std::string str;
    for (const std::string& row : grid) {
        str += row;
        str += '\n';
    }
    return str;
}

void check(std::int64_t expectedSum, const std::string_view& input)
{
    auto [grid, commands] = getInput(input);
    const std::int64_t sum = runSim(grid, commands);
    if (sum != expectedSum) {
        std::cerr << "failure:"
                  << "\n> expected:\n" << expectedSum
                  << "\n> actual:\n" << sum
                  << "\n" << display(grid)
                  << std::endl;
    }
}

void runTests()
{
    const std::vector<std::string> grids[] = {
        { "########", "#..O.O.#", "##@.O..#", "#...O..#", "#.#.O..#", "#...O..#", "#......#", "########" },
        { "########", "#..O.O.#", "##@.O..#", "#...O..#", "#.#.O..#", "#...O..#", "#......#", "########" },
        { "########", "#.@O.O.#", "##..O..#", "#...O..#", "#.#.O..#", "#...O..#", "#......#", "########" },
        { "########", "#.@O.O.#", "##..O..#", "#...O..#", "#.#.O..#", "#...O..#", "#......#", "########" },
        { "########", "#..@OO.#", "##..O..#", "#...O..#", "#.#.O..#", "#...O..#", "#......#", "########" },
        { "########", "#...@OO#", "##..O..#", "#...O..#", "#.#.O..#", "#...O..#", "#......#", "########" },
        { "########", "#...@OO#", "##..O..#", "#...O..#", "#.#.O..#", "#...O..#", "#......#", "########" },
        { "########", "#....OO#", "##..@..#", "#...O..#", "#.#.O..#", "#...O..#", "#...O..#", "########" },
        { "########", "#....OO#", "##..@..#", "#...O..#", "#.#.O..#", "#...O..#", "#...O..#", "########" },
        { "########", "#....OO#", "##.@...#", "#...O..#", "#.#.O..#", "#...O..#", "#...O..#", "########" },
        { "########", "#....OO#", "##.....#", "#..@O..#", "#.#.O..#", "#...O..#", "#...O..#", "########" },
        { "########", "#....OO#", "##.....#", "#...@O.#", "#.#.O..#", "#...O..#", "#...O..#", "########" },
        { "########", "#....OO#", "##.....#", "#....@O#", "#.#.O..#", "#...O..#", "#...O..#", "########" },
        { "########", "#....OO#", "##.....#", "#.....O#", "#.#.O@.#", "#...O..#", "#...O..#", "########" },
        { "########", "#....OO#", "##.....#", "#.....O#", "#.#O@..#", "#...O..#", "#...O..#", "########" },
        { "########", "#....OO#", "##.....#", "#.....O#", "#.#O@..#", "#...O..#", "#...O..#", "########" },
    };
    const std::string_view commands = "<^^>>>vv<v>>v<<";
    for (std::int64_t i = 0; i < commands.size(); ++i) {
        auto grid  = grids[i];
        auto robot = findRobot(grid);
        update(grid, robot, commands[i]);
        if (grid != grids[i+1]) {
            std::cerr << "failure(" << i << "):"
                      << "\n> initial:\n" << display(grids[i])
                      << "\n> command: " << commands[i] << "\n"
                      << "\n> expected:\n" << display(grids[i+1])
                      << "\n> actual:\n" << display(grid)
                      << std::endl;
        }
    }
    check(
        2028,
        "########\n"
        "#..O.O.#\n"
        "##@.O..#\n"
        "#...O..#\n"
        "#.#.O..#\n"
        "#...O..#\n"
        "#......#\n"
        "########\n"
        "\n"
        "<^^>>>vv<v>>v<<\n"
    );
    assert(10092 == sumBoxes(
        { { "##########" },
          { "#.O.O.OOO#" },
          { "#........#" },
          { "#OO......#" },
          { "#OO@.....#" },
          { "#O#.....O#" },
          { "#O.....OO#" },
          { "#O.....OO#" },
          { "#OO....OO#" },
          { "##########" } }
    ));
    check(
        10092,
        "##########\n"
        "#..O..O.O#\n"
        "#......O.#\n"
        "#.OO..O.O#\n"
        "#..O@..O.#\n"
        "#O#..O...#\n"
        "#O..O..O.#\n"
        "#.OO.O.OO#\n"
        "#....O...#\n"
        "##########\n"
        "\n"
        "<vv>^<v^>v>^vv^v>v<>v^v<v<^vv<<<^><<><>>v<vvv<>^v^>^<<<><<v<<<v^vv^v>^\n"
        "vvv<<^>^v^^><<>>><>^<<><^vv^^<>vvv<>><^^v>^>vv<>v<<<<v<^v>^<^^>>>^<v<v\n"
        "><>vv>v^v^<>><>>>><^^>vv>v<^^^>>v^v^<^^>v^^>v^<^v>v<>>v^v^<v>v^^<^^vv<\n"
        "<<v<^>>^^^^>>>v^<>vvv^><v<<<>^^^vv^<vvv>^>v<^^^^v<>^>vvvv><>>v^<<^^^^^\n"
        "^><^><>>><>^^<<^^v>>><^<v>^<vv>>v>>>^v><>^v><<<<v>>v<v<v>vvv>^<><<>^><\n"
        "^>><>^v<><^vvv<^^<><v<<<<<><^v<<<><<<^^<v<^^^><^>>^<v^><<<^>>^v<v^v<v^\n"
        ">^>>^v>vv>^<<^v<>><<><<v<<v><>v<^vv<<<>^^v^>^^>>><<^v>>v^v><^^>>^<>vv^\n"
        "<><^^>^^^<><vvvvv^v<v<<>^v<v>v<<^><<><<><<<^^<<<^<<>><<><^^^>^^<>^>v<>\n"
        "^^>vv<^v^v<vv>^<><v<^v>^^^>>>^^vvv^>vvv<>>>^<^>>>>>^<<^v>^vvv<>^<><<v>\n"
        "v^^>>><<^^<>>^v^<v^vv<>v^<<>^<^v^v><^<<<><<^<v><v<>vv>>v><v^<vv<>v^<<^\n"
    );
}

int main()
{
    runTests();

    auto [grid, commands] = getInput();
    const auto total = runSim(grid, commands);
    std::cout << total << std::endl;
    return 0;
}
