#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

struct Cursor {
    std::int64_t d_i, d_j;
    auto operator<=>(const Cursor&) const = default;
};

void transformCell(char& out1, char& out2, char cell)
{
    switch (cell) {
    case '#':
        out1 = '#', out2 = '#';
        return;
    case 'O':
        out1 = '[', out2 = ']';
        return;
    case '.':
        out1 = '.', out2 = '.';
        return;
    case '@':
        out1 = '@', out2 = '.';
        return;
    }
    assert(!"invalid row");
    std::unreachable();
}

void transformRow(std::string& row)
{
    std::string output(row.size() * 2, 'X');
    for (std::int64_t i = 0; i < row.size(); ++i)
        transformCell(output[2*i], output[2*i+1], row[i]);

    row = std::move(output);
}

void transformGrid(std::vector<std::string>& grid)
{
    for (std::string& row : grid)
        transformRow(row);
}

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
    transformGrid(grid);

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

bool isVertical(std::int64_t direction)
{
    return direction & 1;
}

bool isBox(char cell)
{
    return (cell == '[') || (cell == ']');
}

char& cell(std::vector<std::string>& grid, const Cursor& cursor)
{
    return grid[cursor.d_i][cursor.d_j];
}

bool update(std::vector<std::string>& grid, Cursor& robot, char command)
{
    assert(cell(grid, robot) == '@');

    const std::int64_t d = dirFromCommand(command), di = Dir[d], dj = Dir[d+1];
    Cursor c = { .d_i = robot.d_i+di, .d_j = robot.d_j+dj };
    if (!isBox(cell(grid, c))) {
        if (cell(grid, c) == '.') {
            cell(grid, c) = '@';
            cell(grid, robot) = '.';
            robot = c;
            return true;
        }
        return false;
    }
    const auto correctBoxCursor = [&](Cursor& box) {
        assert(isBox(cell(grid, box)));
        if (cell(grid, box) == ']')
            --box.d_j;
    };
    correctBoxCursor(c);
    const bool isVert = isVertical(d);
    const std::int64_t dib = di, djb = (command == '>') ? 2 : dj;
    std::vector<std::vector<Cursor>> boxes;
    boxes.emplace_back().emplace_back(c);
    while (!boxes.back().empty()) {
        boxes.emplace_back();
        auto& nBoxes = *boxes.rbegin();
        for (const Cursor& pB : boxes.rbegin()[1]) {
            assert(cell(grid, pB) == '[');
            Cursor nB = { .d_i = pB.d_i+dib, .d_j = pB.d_j+djb };
            if (isBox(cell(grid, nB))) {
                correctBoxCursor(nB);
                nBoxes.emplace_back(nB);
            }
            else if (cell(grid, nB) != '.') {
                return false;
            }
            if (!isVert)
                continue;

            nB = { .d_i = pB.d_i+dib, .d_j = pB.d_j+djb+1 };
            if (isBox(cell(grid, nB))) {
                correctBoxCursor(nB);
                if (nBoxes.empty() || (nBoxes.back() != nB))
                    nBoxes.emplace_back(nB);
            }
            else if (cell(grid, nB) != '.') {
                return false;
            }
        }
    }
    for (const std::vector<Cursor>& row : boxes | std::views::reverse) {
        for (const Cursor& pB1 : row) {
            const Cursor pB2 = { .d_i = pB1.d_i,    .d_j = pB1.d_j+1    },
                         nB1 = { .d_i = pB1.d_i+di, .d_j = pB1.d_j+dj   },
                         nB2 = { .d_i = pB1.d_i+di, .d_j = pB1.d_j+dj+1 };
            cell(grid, pB1) = '.', cell(grid, pB2) = '.';
            cell(grid, nB1) = '[', cell(grid, nB2) = ']';
        }
    }
    cell(grid, robot) = '.';
    robot.d_i += di;
    robot.d_j += dj;
    cell(grid, robot) = '@';
    return true;
}

std::int64_t sumBoxes(const std::vector<std::string>& grid)
{
    std::int64_t total = 0;
    for (std::int64_t i = 0; i < grid.size(); ++i)
        for (std::int64_t j = 0; j < grid[i].size(); ++j)
            if (grid[i][j] == '[')
                total += (100 * i + j);

    return total;
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

std::int64_t runSim(std::vector<std::string>& grid,
                    const std::string_view&   commands,
                    bool                      debug = false)
{
    auto robot = findRobot(grid);
    std::int64_t i = 0;
    for (const char command : commands) {
        std::string output;
        if (debug) {
            output += "\x1B[2J\x1B[H"; 
            output += "\n\ncommand(" + std::to_string(++i) + "/" + std::to_string(commands.size()) + "): ";
            output += command;
        }
        const auto prevRobot = robot;
        const auto prevGrid  = grid;
        const bool success   = update(grid, robot, command);
        if (success) {
            assert(robot != prevRobot);
            assert(grid != prevGrid);
        }
        else {
            assert(robot == prevRobot);
            assert(grid == prevGrid);
        }
        if (debug) {
            if (!success)
                output += " (STUCK)";

            output += "\n\n";
            output += display(grid);
            std::cout << output << std::endl;
            using namespace std::chrono_literals;
            // std::this_thread::sleep_for(success ? 100ms : 1000ms);
            std::this_thread::sleep_for(10ms);
        }
    }

    return sumBoxes(grid);
}

std::vector<std::string> gridFromString(const std::string_view& str)
{
    std::vector<std::string> grid;
    for (std::istringstream stream{std::string(str)};
         std::getline(stream, grid.emplace_back()); )
        continue;

    assert(!grid.empty());
    grid.pop_back();
    assert(!grid.empty());
    return grid;
}

void checkTransform(const std::string_view& output, const std::string_view& input)
{
    std::vector<std::string> oGrid(gridFromString(output)), iGrid(gridFromString(input));
    transformGrid(iGrid);
    if (oGrid != iGrid) {
        std::cerr << "failure:"
                  << "\n> expected:\n" << output
                  << "\n> actual:\n" << display(iGrid)
                  << std::endl;
    }
}

void checkGridTransition(const std::vector<std::string>& pGrid,
                         const std::vector<std::string>& nGrid,
                         char                            command)
{
    auto grid  = pGrid;
    auto robot = findRobot(pGrid);
    update(grid, robot, command);
    if (grid != nGrid) {
        std::cerr << "\n> initial:\n" << display(pGrid)
                  << "\n> command: " << command << "\n"
                  << "\n> expected:\n" << display(nGrid)
                  << "\n> actual:\n" << display(grid)
                  << std::endl;
    }
}

void checkTransition(const std::string_view& pGrid,
                     const std::string_view& nGrid,
                     char                    command)
{
    return checkGridTransition(gridFromString(pGrid), gridFromString(nGrid), command);
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
    checkTransform(
        "##############\n"
        "##......##..##\n"
        "##..........##\n"
        "##....[][]@.##\n"
        "##....[]....##\n"
        "##..........##\n"
        "##############\n",
        "#######\n"
        "#...#.#\n"
        "#.....#\n"
        "#..OO@#\n"
        "#..O..#\n"
        "#.....#\n"
        "#######\n"
    );
	checkTransform(
        "####################\n"
        "##....[]....[]..[]##\n"
        "##............[]..##\n"
        "##..[][]....[]..[]##\n"
        "##....[]@.....[]..##\n"
        "##[]##....[]......##\n"
        "##[]....[]....[]..##\n"
        "##..[][]..[]..[][]##\n"
        "##........[]......##\n"
        "####################\n",
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
    );
    const std::vector<std::string> grids[] = {
		 { "##############", "##......##..##", "##..........##", "##....[][]@.##", "##....[]....##", "##..........##", "##############" },
		 { "##############", "##......##..##", "##..........##", "##...[][]@..##", "##....[]....##", "##..........##", "##############" },
		 { "##############", "##......##..##", "##..........##", "##...[][]...##", "##....[].@..##", "##..........##", "##############" },
		 { "##############", "##......##..##", "##..........##", "##...[][]...##", "##....[]....##", "##.......@..##", "##############" },
		 { "##############", "##......##..##", "##..........##", "##...[][]...##", "##....[]....##", "##......@...##", "##############" },
		 { "##############", "##......##..##", "##..........##", "##...[][]...##", "##....[]....##", "##.....@....##", "##############" },
		 { "##############", "##......##..##", "##...[][]...##", "##....[]....##", "##.....@....##", "##..........##", "##############" },
		 { "##############", "##......##..##", "##...[][]...##", "##....[]....##", "##.....@....##", "##..........##", "##############" },
		 { "##############", "##......##..##", "##...[][]...##", "##....[]....##", "##....@.....##", "##..........##", "##############" },
		 { "##############", "##......##..##", "##...[][]...##", "##....[]....##", "##...@......##", "##..........##", "##############" },
		 { "##############", "##......##..##", "##...[][]...##", "##...@[]....##", "##..........##", "##..........##", "##############" },
		 { "##############", "##...[].##..##", "##...@.[]...##", "##....[]....##", "##..........##", "##..........##", "##############" },
    };
    const std::string_view commands = "<vv<<^^<<^^";
    for (std::int64_t i = 0; i < commands.size(); ++i)
        checkGridTransition(grids[i], grids[i+1], commands[i]);

    checkTransition(
    "#######\n"
    "#.....#\n"
    "#.[]..#\n"
    "#[]...#\n"
    "#.[]..#\n"
    "#..@..#\n"
    "#######\n",
    "#######\n"
    "#.[]..#\n"
    "#[]...#\n"
    "#.[]..#\n"
    "#..@..#\n"
    "#.....#\n"
    "#######\n",
    '^'
    );

    checkTransition(
    "#######\n"
    "#.....#\n"
    "#.[]..#\n"
    "#[]...#\n"
    "#.[]..#\n"
    "#.@...#\n"
    "#######\n",
    "#######\n"
    "#.[]..#\n"
    "#[]...#\n"
    "#.[]..#\n"
    "#.@...#\n"
    "#.....#\n"
    "#######\n",
    '^'
    );

    checkTransition(
    "#######\n"
    "#.....#\n"
    "#..[].#\n"
    "#@[]..#\n"
    "#..[].#\n"
    "#.....#\n"
    "#######\n",
    "#######\n"
    "#.....#\n"
    "#..[].#\n"
    "#.@[].#\n"
    "#..[].#\n"
    "#.....#\n"
    "#######\n",
    '>'
    );

    checkTransition(
    "#######\n"
    "#.....#\n"
    "#.[]..#\n"
    "#..[]@#\n"
    "#.[]..#\n"
    "#.....#\n"
    "#######\n",
    "#######\n"
    "#.....#\n"
    "#.[]..#\n"
    "#.[]@.#\n"
    "#.[]..#\n"
    "#.....#\n"
    "#######\n",
    '<'
    );

    checkTransition(
   "##########\n"
   "#...@....#\n"
   "#...[]...#\n"
   "#..[][]..#\n"
   "#.[].[]..#\n"
   "#.[]..[].#\n"
   "#[][].[].#\n"
   "#[][][][]#\n"
   "#......[]#\n"
   "######[].#\n"
   "#........#\n"
   "##########\n",
   "##########\n"
   "#........#\n"
   "#...@....#\n"
   "#...[]...#\n"
   "#..[][]..#\n"
   "#.[].[]..#\n"
   "#.[]..[].#\n"
   "#[][].[].#\n"
   "#[][][][]#\n"
   "######.[]#\n"
   "#.....[].#\n"
   "##########\n",
    'v'
    );

    checkTransition(
   "##########\n"
   "#...@....#\n"
   "#...[]...#\n"
   "#..[][]..#\n"
   "#.[].[]..#\n"
   "#.[]..[].#\n"
   "#[][].[].#\n"
   "#[][][][]#\n"
   "##.....[]#\n"
   "######[].#\n"
   "#........#\n"
   "##########\n",
   "##########\n"
   "#...@....#\n"
   "#...[]...#\n"
   "#..[][]..#\n"
   "#.[].[]..#\n"
   "#.[]..[].#\n"
   "#[][].[].#\n"
   "#[][][][]#\n"
   "##.....[]#\n"
   "######[].#\n"
   "#........#\n"
   "##########\n",
    'v'
    );


    assert(9021 == sumBoxes(
        { { "####################" },
          { "##[].......[].[][]##" },
          { "##[]...........[].##" },
          { "##[]........[][][]##" },
          { "##[]......[]....[]##" },
          { "##..##......[]....##" },
          { "##..[]............##" },
          { "##..@......[].[][]##" },
          { "##......[][]..[]..##" },
          { "####################" } }
    ));
    check(
        9021,
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
