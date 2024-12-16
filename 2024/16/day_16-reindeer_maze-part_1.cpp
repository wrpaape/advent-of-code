#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

enum Dir { e_West, e_North, e_East, e_South };
constexpr int k_Dir[] = { 0, -1, 0, +1, 0 };

struct Cursor {
    std::int32_t d_i;
    std::int32_t d_j;
    int          d_d;
    auto operator<=>(const Cursor&) const = default;
};

struct State {
    std::int64_t d_score;
    Cursor       d_cursor;
    auto operator<=>(const State&) const = default;
};

auto getInput(std::istream& stream)
{
    std::vector<std::string> maze;
    while (std::getline(stream, maze.emplace_back()) && !maze.back().empty())
        continue;

    assert(!maze.empty());
    maze.pop_back();
    assert(!maze.empty());
    return maze;
}

auto getInput()
{
    std::ifstream stream("day_16-reindeer_maze-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

Cursor findStart(const std::vector<std::string>& maze)
{
	for (std::int32_t i = 0; i < maze.size(); ++i)
		for (std::int32_t j = 0; j < maze[i].size(); ++j)
			if (maze[i][j] == 'S')
                return { .d_i = i, .d_j = j, .d_d = e_East };

    assert(!"invalid maze");
    std::unreachable();
}

std::int64_t solve(const std::vector<std::string>& maze)
{
	const std::int32_t m = maze.size(), n = maze[0].size();
    std::array<std::int64_t, 4> initScores;
    initScores.fill(std::numeric_limits<std::int64_t>::max());
	std::vector<std::vector<std::array<std::int64_t, 4>>> minScores(
                       m,
                       std::vector<std::array<std::int64_t, 4>>(n, initScores));
    const auto cell = [&](const Cursor& c) { return maze[c.d_i][c.d_j]; };
    const auto minScore = [&](const Cursor& c) -> std::int64_t& {
        return minScores[c.d_i][c.d_j][c.d_d];
    };
    State state = { .d_score = 0, .d_cursor = findStart(maze) };
    std::priority_queue<State, std::vector<State>, std::greater<State>> nextState;
    nextState.push(state);
    while (true) {
        assert(!nextState.empty());
        const auto [score, c] = nextState.top();
        nextState.pop();
        const auto [i, j, d] = c;

        if (cell(c) == 'E')
            return score;

        for (int dd = 1; dd <= 3; ++dd) {
            const int   numTurns = 1 + ((dd & 1) == 0);
            const State nS = { .d_score  = score + (1000*numTurns),
                               .d_cursor = { .d_i = i,
                                             .d_j = j,
                                             .d_d = (d + dd) % 4 } };
            if (nS.d_score < minScore(nS.d_cursor)) {
                minScore(nS.d_cursor) = nS.d_score;
                nextState.push(nS);
            }
        }
        const State nS = { .d_score  = score + 1,
                           .d_cursor = { .d_i = i + k_Dir[d],
                                         .d_j = j + k_Dir[d+1],
                                         .d_d = d } };
        if ((cell(nS.d_cursor) != '#') && (nS.d_score < minScore(nS.d_cursor))) {
            minScore(nS.d_cursor) = nS.d_score;
            nextState.push(nS);
        }
    }
}

void check(std::int64_t expectedScore, const std::string_view& input)
{
    const auto grid = getInput(input);
    const std::int64_t score = solve(grid);
    if (score != expectedScore) {
        std::cerr << "failure:"
                  << "\n> expected:\n" << expectedScore
                  << "\n> actual:\n" << score
                  << std::endl;
    }
}

void runTests()
{
	check(
		7036,
		"###############\n"
		"#.......#....E#\n"
		"#.#.###.#.###.#\n"
		"#.....#.#...#.#\n"
		"#.###.#####.#.#\n"
		"#.#.#.......#.#\n"
		"#.#.#####.###.#\n"
		"#...........#.#\n"
		"###.#.#####.#.#\n"
		"#...#.....#.#.#\n"
		"#.#.#.###.#.#.#\n"
		"#.....#...#.#.#\n"
		"#.###.#.#.#.#.#\n"
		"#S..#.....#...#\n"
		"###############\n"
	);
	check(
		11048,
		"#################\n"
		"#...#...#...#..E#\n"
		"#.#.#.#.#.#.#.#.#\n"
		"#.#.#.#...#...#.#\n"
		"#.#.#.#.###.#.#.#\n"
		"#...#.#.#.....#.#\n"
		"#.#.#.#.#.#####.#\n"
		"#.#...#.#.#.....#\n"
		"#.#.#####.#.###.#\n"
		"#.#.#.......#...#\n"
		"#.#.###.#####.###\n"
		"#.#.#...#.....#.#\n"
		"#.#.#.#####.###.#\n"
		"#.#.#.........#.#\n"
		"#.#.#.#########.#\n"
		"#S#.............#\n"
		"#################\n"
	);
}

int main()
{
    runTests();

    const auto maze = getInput();
    const auto score = solve(maze);
    std::cout << score << std::endl;
    return 0;
}
