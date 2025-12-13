#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// 0+0
// AAA.
// ABAB
// ABAB
// .BBB
//
// 2+5
// .AAB
// AABB
// ABBB

struct Requirement {
  std::int64_t d_m;
  std::int64_t d_n;
  std::vector<std::int64_t> d_numPresents;
};

using Orientation = std::array<std::array<char, 3>, 3>;

namespace std {
  template<>
  struct hash<Orientation> {
    std::size_t operator()(const Orientation& o) const {
      const char buffer[] = { o[0][0], o[0][1], o[0][2],
                              o[1][0], o[1][1], o[1][2],
                              o[2][0], o[2][1], o[2][2] };
      return hash<string_view>{}(
          std::string_view(buffer, buffer + sizeof(buffer)));
    }
  };
}

struct Present {
  std::vector<Orientation> d_orientations;
};

struct Input {
  std::vector<Present>      d_presents;
  std::vector<Requirement> d_requirements;
};

void rotate(Orientation& o)
{
  Orientation nO;
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      nO[j][2-i] = o[i][j];
  o = nO;
}

void flip(Orientation& o)
{
  std::swap(o[0][0], o[0][2]);
  std::swap(o[1][0], o[1][2]);
  std::swap(o[2][0], o[2][2]);
}

Present makePresent(std::string_view r1,
                    std::string_view r2,
                    std::string_view r3)
{
    assert(r1.size() == 3);
    assert(r2.size() == 3);
    assert(r3.size() == 3);

    Present p;
    Orientation o;
    r1.copy(o[0].data(), 3);
    r2.copy(o[1].data(), 3);
    r3.copy(o[2].data(), 3);
    std::unordered_set<Orientation> uniques;
    uniques.insert(o); rotate(o);
    uniques.insert(o); rotate(o);
    uniques.insert(o); rotate(o);
    uniques.insert(o); rotate(o);
    flip(o);
    uniques.insert(o); rotate(o);
    uniques.insert(o); rotate(o);
    uniques.insert(o); rotate(o);
    uniques.insert(o);
    p.d_orientations.assign(uniques.begin(), uniques.end());
    return p;
}

Requirement makeRequirement(std::string_view l)
{
  Requirement r;
  const auto lEnd = l.data() + l.size();
  const auto [ptr1, err1] =
      std::from_chars(l.data(), lEnd, r.d_m);
  assert(err1 == std::errc());
  assert(ptr1 != lEnd);
  assert(*ptr1 == 'x');
  const auto [ptr2, err2] =
      std::from_chars(ptr1+1, lEnd, r.d_n);
  assert(err2 == std::errc());
  assert(ptr2 != lEnd);
  assert(*ptr2 == ':');
  for (auto p = ptr2+1; p != lEnd;) {
    assert(*p == ' ');
    const auto [ptr, err] =
        std::from_chars(p + 1, lEnd, r.d_numPresents.emplace_back());
    assert(err == std::errc());
    p = ptr;
  }
  return r;
}

auto getInput(std::istream &stream) {
  Input input;
  std::string line;
  while (true) {
    std::getline(stream, line);
    assert(stream);
    if (line.find('x') != std::string::npos)
      break;

    std::string r1, r2, r3;
    std::getline(stream, r1);
    assert(r1.size() == 3);
    std::getline(stream, r2);
    assert(r2.size() == 3);
    std::getline(stream, r3);
    assert(r3.size() == 3);
    std::getline(stream, line);
    assert(line.empty());
    input.d_presents.push_back(makePresent(r1, r2, r3));
  }
  do {
    input.d_requirements.push_back(makeRequirement(line));
  } while (std::getline(stream, line));
  return input;
}

auto getInput() {
  for (const auto &entry : std::filesystem::directory_iterator(".")) {
    if (std::filesystem::is_regular_file(entry.path()) &&
        entry.path().string().ends_with("-input.txt")) {
      std::ifstream stream(entry.path().string());
      return getInput(stream);
    }
  }
  std::cerr << "Failed to find input file in the current directory"
            << std::endl;
  std::abort();
}

auto getInput(std::string_view input) {
  std::istringstream stream{std::string(input)};
  return getInput(stream);
}

std::int64_t countPaths(
    const std::unordered_map<std::string, std::vector<std::string>> &graph,
    const std::string &src, const std::string &dst) {
  std::unordered_map<std::string, std::int64_t> numPaths;
  const auto dfs = [&](this auto&& dfs, const std::string& u) -> std::int64_t {
    if (u == dst)
      return 1;
    const auto [it, newEntry] = numPaths.emplace(u, -1);
    if (!newEntry) {
      assert(it->second >= 0);
      return it->second;
    }
    std::int64_t numP = 0;
    if (const auto edges = graph.find(u); edges != graph.end())
      for (const std::string& v : edges->second)
        numP += dfs(v);
    return numPaths[u] = numP;
  };
  return dfs(src);
}

#include <vector>
#include <algorithm>

// Returns CNF clauses enforcing: at least k of vars[] are true.
// vars: list of variable indices (positive integers, DIMACS-style)
// k <= vars.size()
void atLeastK(std::vector<std::vector<std::int64_t>> *clauses,
              std::int64_t *nextVar, std::span<const std::int64_t> vars,
              std::int64_t k) {
  assert(clauses);
  assert(nextVar);
  assert(k > 0);
  assert(vars.size() >= k);
  assert(std::ranges::is_sorted(vars));
  assert(!vars.empty());
  assert(*nextVar >= vars.back());

  std::int64_t n = vars.size();

  // s[i][j] = variable index for “among first i vars, at least j are true”
  // We use 1-based indexing for clarity
  std::vector<std::vector<std::int64_t>> s(n + 1, std::vector<std::int64_t>(k + 1, 0));

  for (std::int64_t i = 1; i <= n; ++i) {
    for (std::int64_t j = 1; j <= k && j <= i; ++j) {
      s[i][j] = ++*nextVar;
      assert(s[i][j] != 0);
    }
  }

  // Sequential counter clauses
  for (std::int64_t i = 1; i <= n; ++i) {
    std::int64_t xi = vars[i - 1];

    // j = 1 case:
    // (¬xi ∨ s[i][1])
    clauses->push_back({-xi, s[i][1]});

    if (i > 1) {
      // (¬s[i-1][1] ∨ s[i][1])
      assert(-s[i-1][1] != 0);
      clauses->push_back({-s[i - 1][1], s[i][1]});
    }

    // j >= 2 cases
    for (std::int64_t j = 2; j <= k && j <= i; ++j) {
      // (¬xi ∨ ¬s[i-1][j-1] ∨ s[i][j])
      clauses->push_back({-xi, -s[i - 1][j - 1], s[i][j]});

      if (i > j) {
        // (¬s[i-1][j] ∨ s[i][j])
        clauses->push_back({-s[i - 1][j], s[i][j]});
      }
    }
  }

  // Enforce at least k: s[n][k] must be true
  clauses->push_back({s[n][k]});
}

void buildClauses(std::vector<std::vector<std::int64_t>> *clauses, std::int64_t *numVars,
                  std::span<const Present> presents, const Requirement &r) {
  assert(clauses);
  assert(numVars);
  const auto& [m, n, numPresents] = r;
  assert(m >= 3);
  assert(n >= 3);
  assert(presents.size() == numPresents.size());

  std::vector<std::vector<std::vector<std::int64_t>>> grid(
      m, std::vector<std::vector<std::int64_t>>(n));
  const std::int64_t G = numPresents.size();
  std::vector<std::vector<std::int64_t>> groups(G);
  std::int64_t var = 0;
  for (std::int64_t g = 0; g < G; ++g) if (numPresents[g] > 0) {
    for (const auto& o : presents[g].d_orientations) {
      for (std::int64_t i = 0; (i+2) < m; ++i) {
        for (std::int64_t j = 0; (j+2) < n; ++j) {
          groups[g].push_back(++var);
          for (std::int64_t ii = 0; ii < 3; ++ii) {
            for (std::int64_t jj = 0; jj < 3; ++jj) {
              if (o[ii][jj] == '#') {
                grid[i+ii][j+jj].push_back(var);
              }
            }
          }
        }
      }
    }
  }
  clauses->clear();
  for (const auto& r : grid) {
    for (const auto& c : r) if (!c.empty()) {
      auto& out = clauses->emplace_back();
      for (std::int64_t i = 0; i < c.size(); ++i) {
        for (std::int64_t j = i+1; j < c.size(); ++j) {
          out.push_back(-c[i]);
          out.push_back(-c[j]);
        }
      }
    }
    for (std::int64_t g = 0; g < G; ++g) if (numPresents[g] > 0) {
      const std::int64_t N = groups[g].size();
      const std::int64_t K = numPresents[g];
      assert(N >= K);
      atLeastK(clauses, &var, groups[g], K);
    }
  }
  *numVars = var;
}

void writeCnf(std::string_view cnfFile, std::span<const Present> presents,
              const Requirement &r) {
  const auto& [m, n, numPresents] = r;
  assert(m >= 3);
  assert(n >= 3);
  assert(presents.size() == numPresents.size());

  std::ofstream stream(std::string(cnfFile), std::ios::trunc);
  std::vector<std::vector<std::int64_t>> clauses;
  std::int64_t numVars = 0;
  buildClauses(&clauses, &numVars, presents, r);
  stream << "p cnf " << numVars << ' ' << clauses.size() << '\n';
  for (const auto& clause : clauses) {
    assert(!clause.empty());
    for (const std::int64_t var : clause) {
      // assert(var != 0);
      stream << var << ' ';
    }
    stream << "0\n";
  }
  stream << std::flush;
}

bool canFit(std::span<const Present> presents, const Requirement &r) {
  const auto& [m, n, numPresents] = r;
  assert(m >= 3);
  assert(n >= 3);
  assert(presents.size() == numPresents.size());
  const std::string cnfFile("day_12.cnf");
  writeCnf(cnfFile, presents, r);
  const int status = std::system(("cryptominisat " + cnfFile).c_str());
  assert(WIFEXITED(status));
  const int exitCode = WEXITSTATUS(status);
  assert((exitCode == 10) || (exitCode == 20));
  return (exitCode == 10);
}

std::int64_t solve(const Input& input) {
  std::int64_t total = 0, i = 0;
  for (const auto& r : input.d_requirements) {
    std::cout << ++i << std::endl;
    total += canFit(input.d_presents, r);
  }
  return total;
}

void check(int lineNumber, std::int64_t expectMaxArea,
           std::string_view source) {
  if (const auto maxArea = solve(getInput(source)); maxArea != expectMaxArea) {
    std::cerr << std::format("failure({}):\n> expected: {}\n> actual:   {}\n",
                             lineNumber, expectMaxArea, maxArea)
              << std::endl;
  }
}

void runTests() {
  const std::string_view example =
      R"(0:
###
##.
##.

1:
###
##.
.##

2:
.##
###
##.

3:
##.
###
##.

4:
###
#..
###

5:
###
.#.
###

4x4: 0 0 0 0 2 0
12x5: 1 0 1 0 2 2
12x5: 1 0 1 0 3 2
)";
  check(__LINE__, 2, example);
}

int main() {
  // runTests();

  const auto result = solve(getInput());
  std::cout << result << std::endl;
  return 0;
}
