#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

enum Dir { e_West, e_North, e_East, e_South };
constexpr int k_Dir[] = { 0, -1, 0, +1, 0 };

struct Point {
    std::int32_t d_i;
    std::int32_t d_j;
    auto operator<=>(const Point&) const = default;
};
struct Cursor {
    Point d_pos;
    int   d_d;
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

Point findCell(const std::vector<std::string>& maze, char cell)
{
	for (std::int32_t i = 0; i < maze.size(); ++i)
		for (std::int32_t j = 0; j < maze[i].size(); ++j)
			if (maze[i][j] == cell)
                return { .d_i = i, .d_j = j };

    assert(!"invalid maze");
    std::unreachable();
}

std::int64_t solve(const std::vector<std::string>& maze, bool debug = false)
{
	const std::int32_t m = maze.size(), n = maze[0].size();
    constexpr std::int64_t k_MaxScore     = std::numeric_limits<std::int64_t>::max();
    constexpr Point        k_InitPosition = { .d_i = -1, .d_j = -1 };
    constexpr Cursor       k_InitCursor   = { .d_pos = k_InitPosition, .d_d = -1 };
    std::array<std::int64_t, 4> initScores;
    initScores.fill(k_MaxScore);
	std::vector<std::vector<std::array<std::int64_t, 4>>> minScores(
                       m,
                       std::vector<std::array<std::int64_t, 4>>(n, initScores));
    std::array<Cursor, 4> initCursors;
    initCursors.fill(k_InitCursor);
	std::vector<std::vector<std::array<std::vector<Cursor>, 4>>> prevCursorSets(
                            m,
                            std::vector<std::array<std::vector<Cursor>, 4>>(n));
    const auto cell = [&](const Point& p) { return maze[p.d_i][p.d_j]; };
    const auto minScore = [&](const Cursor& c) -> std::int64_t& {
        return minScores[c.d_pos.d_i][c.d_pos.d_j][c.d_d];
    };
    const auto prevCursors = [&](const Cursor& c) -> std::vector<Cursor>& {
        return prevCursorSets[c.d_pos.d_i][c.d_pos.d_j][c.d_d];
    };
    const Point k_Start = findCell(maze, 'S'), k_Exit = findCell(maze, 'E');
    const auto& minExitScores = minScores[k_Exit.d_i][k_Exit.d_j];
    const auto isDone = [&]() {
        return std::all_of(minExitScores.begin(), minExitScores.end(),
                           [](std::int64_t score) { return score != k_MaxScore; });
    };
    std::priority_queue<State, std::vector<State>, std::greater<State>> nextState;
    nextState.push({ .d_score  = 0,
                     .d_cursor = { .d_pos = k_Start, .d_d = e_East } });
    while (!nextState.empty()) {
        const auto [score, cursor] = nextState.top();
        nextState.pop();
        const auto [pos, d] = cursor;
        if (pos == k_Exit) {
            if (isDone())
                break;
            else
                continue;
        }
        const auto [i, j] = pos;
        for (int dd = 1; dd <= 3; ++dd) {
            const int   numTurns = 1 + ((dd & 1) == 0);
            const State nS = { .d_score  = score + (1000*numTurns),
                               .d_cursor = { .d_pos = { .d_i = i, .d_j = j },
                                             .d_d   = (d + dd) % 4 } };
            assert(nS.d_cursor != cursor);
            if (nS.d_score <= minScore(nS.d_cursor)) {
                if (nS.d_score < minScore(nS.d_cursor)) {
                    minScore(nS.d_cursor) = nS.d_score;
                    nextState.push(nS);
                    prevCursors(nS.d_cursor).clear();
                }
                prevCursors(nS.d_cursor).push_back(cursor);
            }
        }
        const State nS = { .d_score  = score + 1,
                           .d_cursor = { .d_pos = { .d_i = i + k_Dir[d],
                                                    .d_j = j + k_Dir[d+1] },
                                         .d_d   = d } };
        assert(nS.d_cursor != cursor);
        if ((cell(nS.d_cursor.d_pos) != '#') &&
            (nS.d_score <= minScore(nS.d_cursor))) {
            if (nS.d_score < minScore(nS.d_cursor)) {
                minScore(nS.d_cursor) = nS.d_score;
                nextState.push(nS);
                prevCursors(nS.d_cursor).clear();
            }
            prevCursors(nS.d_cursor).push_back(cursor);
        }
    }
    std::set<Cursor> visited;
    std::set<Point>  seats;
    const auto bestScore = *std::min_element(minExitScores.begin(),
                                             minExitScores.end());
    for (int d = 0; d < 4; ++d) if (minExitScores[d] == bestScore) {
        std::queue<Cursor> frontier;
        frontier.push({ .d_pos = k_Exit, .d_d = d });
        while (!frontier.empty()) {
            const auto c = frontier.front();
            frontier.pop();
            seats.insert(c.d_pos);
            for (const auto& pC : prevCursors(c)) if (visited.insert(pC).second) {
                frontier.push(pC);
            }
        }
    }
    if (debug) {
        auto mazeWithSeats = maze;
        for (const auto& p : seats) mazeWithSeats[p.d_i][p.d_j] = 'O';
        for (const auto& row : mazeWithSeats) std::cout << row << std::endl;
    }
    return seats.size();
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
		45,
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
		64,
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
