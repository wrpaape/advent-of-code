#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

enum State : std::uint8_t {
    e_Unset = 255,
    e_Off   = 0,
    e_On    = 1
};

enum Op : std::uint8_t {
    e_And,
    e_Or,
    e_Xor
};

struct Gate {
    std::uint16_t d_in1;
    std::uint16_t d_in2;
    Op            d_op;
    std::uint16_t d_out;
};

struct Input {
    std::vector<std::string> d_wires;
    std::vector<State>       d_initStates;
    std::vector<Gate>        d_gates;
};

std::uint16_t getW(const std::vector<std::string>& wires,
                   const std::string&              wire)
{
    assert(std::binary_search(wires.begin(), wires.end(), wire));

    return std::distance(wires.begin(),
                         std::lower_bound(wires.begin(), wires.end(), wire));
}

auto getInput(std::istream& stream)
{
    std::unordered_map<std::string, State> initStates;
    Input input;
    for (std::string line; std::getline(stream, line) && !line.empty(); ) {
        const auto d = line.find(':');
        assert(d != std::string::npos);
        const std::string wire      = line.substr(0, d),
                          initState = line.substr(d+2);
        assert(!wire.empty());
        assert((initState == "0") || (initState == "1"));
        assert(!initStates.contains(wire));
        initStates[wire] = (initState == "0") ? e_Off : e_On;
    }
    assert(stream);
    std::vector<std::array<std::string, 4>> inputGates;
    std::unordered_set<std::string>         seenOuts;
    for (std::string line; std::getline(stream, line); ) {
        const auto d1 = line.find(' ', 0),
                   d2 = line.find(' ', d1+1),
                   d3 = line.find(' ', d2+1),
                   d4 = line.find(' ', d3+1);
        assert(d1 != std::string::npos);
        assert(d2 != std::string::npos);
        assert(d3 != std::string::npos);
        assert(d4 != std::string::npos);
        const std::string in1 = line.substr( 0,   d1),
                          op  = line.substr(d1+1, d2-d1-1),
                          in2 = line.substr(d2+1, d3-d2-1),
                          out = line.substr(d4+1);
        assert(!in1.empty());
        assert(!in2.empty());
        assert((op == "AND") || (op == "OR") || (op == "XOR"));
        assert(!out.empty());
        assert(!seenOuts.contains(out));
        seenOuts.insert(out);
        initStates.emplace(in1, e_Unset);
        initStates.emplace(in2, e_Unset);
        initStates.emplace(out, e_Unset);
        inputGates.push_back({ in1, in2, op, out });
    }
    assert(initStates.size() <= std::numeric_limits<std::uint16_t>::max());
    for (const auto& [wire, _] : initStates)
        input.d_wires.push_back(wire);

    std::sort(input.d_wires.begin(), input.d_wires.end());
    for (const std::string& wire : input.d_wires)
        input.d_initStates.push_back(initStates.find(wire)->second);

    for (const auto& [in1, in2, op, out] : inputGates)
        input.d_gates.push_back({
            .d_in1 = getW(input.d_wires, in1),
            .d_in2 = getW(input.d_wires, in2),
            .d_op  = (op == "AND") ? e_And : (op == "OR") ? e_Or : e_Xor,
            .d_out = getW(input.d_wires, out)
        });
    return input;
}

auto getInput()
{
    std::ifstream stream("day_24-crossed_wires-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::uint64_t makeNumber(const std::vector<std::string>& wires,
                         const std::vector<State>&       states)
{
    std::uint64_t num = 0, numOutputs = 0;
    for (const auto& [wire, state] : std::views::zip(wires, states)
                                   | std::views::reverse) {
        assert(state != e_Unset);
        if (wire.starts_with('z')) {
            assert(++numOutputs <= 64);
            num <<= 1;
            num |= (state == e_On);
        }
    }
    return num;
}

std::vector<Gate> topologicalSort(const std::vector<Gate>& gates,
                                  std::uint16_t            numWires)
{
    assert(gates.size() < std::numeric_limits<std::uint16_t>::max());

    std::vector<std::vector<std::uint16_t>> edges(numWires);
    std::vector<bool> haveGate(numWires);
    std::vector<std::uint16_t> ingress(numWires), gateForW(numWires);
    for (std::uint16_t g = 0; g < gates.size(); ++g) {
        const auto [in1, in2, _, out] = gates[g];
        edges[in1].push_back(out);
        edges[in2].push_back(out);
        ingress[out] += 2;
        haveGate[out] = true;
        gateForW[out] = g;
    }
    std::vector<std::uint16_t> pRoots, nRoots;
    for (std::uint16_t w = 0; w < numWires; ++w) if (ingress[w] == 0) {
        nRoots.push_back(w);
    }
    std::vector<Gate> sorted;
    while (!nRoots.empty()) {
        nRoots.swap(pRoots);
        nRoots.clear();
        for (const std::uint16_t root : pRoots) {
            if (haveGate[root])
                sorted.push_back(gates[gateForW[root]]);

            for (const std::uint16_t e : edges[root]) if (--ingress[e] == 0) {
                nRoots.push_back(e);
            }
        }
    }
    assert(sorted.size() == gates.size());
    return sorted;
}

std::uint64_t solve(const Input& input, std::string *finalStates = nullptr)
{
    auto states = input.d_initStates;
    for (const auto [in1, in2, op, out] :
                        topologicalSort(input.d_gates, input.d_wires.size())) {
        assert(states[in1] != e_Unset);
        assert(states[in2] != e_Unset);
        assert(states[out] == e_Unset);
        const std::int8_t bit1 = (states[in1] == e_On),
                          bit2 = (states[in2] == e_On);
        switch (op) {
        case e_And: states[out] = State(bit1 & bit2); break;
        case e_Or:  states[out] = State(bit1 | bit2); break;
        case e_Xor: states[out] = State(bit1 ^ bit2); break;
        default:    assert(!"invalid op"); std::unreachable();
        }
    }
    if (finalStates) {
        finalStates->clear();
        for (const auto [wire, state] :
                                      std::views::zip(input.d_wires, states)) {
            assert(state != e_Unset);
            *finalStates += wire;
            *finalStates += ": ";
            *finalStates += char('0' + (state == e_On));
            *finalStates += '\n';
        }
    }
    return makeNumber(input.d_wires, states);
}

void check(int                     lineNumber,
           std::uint64_t           expectedNum,
           const std::string_view& expectedStates,
           const std::string_view& source)
{
    std::string states;
    if (const auto num = solve(getInput(source), &states);
        num != expectedNum) {
        std::cerr << "failure(" << lineNumber << ") - num:"
                  << "\n> expected: " << expectedNum
                  << "\n> actual:   " << num
                  << std::endl;
    }
    if (states != expectedStates) {
        std::cerr << "failure(" << lineNumber << ") - states:"
                  << "\n> expected:\n" << expectedStates
                  << "\n> actual:\n" << states
                  << std::endl;
    }
}

void runTests()
{
    check(
        __LINE__,
        4,
        "x00: 1\n"
        "x01: 1\n"
        "x02: 1\n"
        "y00: 0\n"
        "y01: 1\n"
        "y02: 0\n"
        "z00: 0\n"
        "z01: 0\n" 
        "z02: 1\n",
        "x00: 1\n"
        "x01: 1\n"
        "x02: 1\n"
        "y00: 0\n"
        "y01: 1\n"
        "y02: 0\n"
        "\n"
        "x00 AND y00 -> z00\n"
        "x01 XOR y01 -> z01\n"
        "x02 OR y02 -> z02\n"
    );
    check(
        __LINE__,
        2024,
        "bfw: 1\n"
        "bqk: 1\n"
        "djm: 1\n"
        "ffh: 0\n"
        "fgs: 1\n"
        "frj: 1\n"
        "fst: 1\n"
        "gnj: 1\n"
        "hwm: 1\n"
        "kjc: 0\n"
        "kpj: 1\n"
        "kwq: 0\n"
        "mjb: 1\n"
        "nrd: 1\n"
        "ntg: 0\n"
        "pbm: 1\n"
        "psh: 1\n"
        "qhw: 1\n"
        "rvg: 0\n"
        "tgd: 0\n"
        "tnw: 1\n"
        "vdt: 1\n"
        "wpb: 0\n"
        "x00: 1\n"
        "x01: 0\n"
        "x02: 1\n"
        "x03: 1\n"
        "x04: 0\n"
        "y00: 1\n"
        "y01: 1\n"
        "y02: 1\n"
        "y03: 1\n"
        "y04: 1\n"
        "z00: 0\n"
        "z01: 0\n"
        "z02: 0\n"
        "z03: 1\n"
        "z04: 0\n"
        "z05: 1\n"
        "z06: 1\n"
        "z07: 1\n"
        "z08: 1\n"
        "z09: 1\n"
        "z10: 1\n"
        "z11: 0\n"
        "z12: 0\n",
        "x00: 1\n"
        "x01: 0\n"
        "x02: 1\n"
        "x03: 1\n"
        "x04: 0\n"
        "y00: 1\n"
        "y01: 1\n"
        "y02: 1\n"
        "y03: 1\n"
        "y04: 1\n"
        "\n"
        "ntg XOR fgs -> mjb\n"
        "y02 OR x01 -> tnw\n"
        "kwq OR kpj -> z05\n"
        "x00 OR x03 -> fst\n"
        "tgd XOR rvg -> z01\n"
        "vdt OR tnw -> bfw\n"
        "bfw AND frj -> z10\n"
        "ffh OR nrd -> bqk\n"
        "y00 AND y03 -> djm\n"
        "y03 OR y00 -> psh\n"
        "bqk OR frj -> z08\n"
        "tnw OR fst -> frj\n"
        "gnj AND tgd -> z11\n"
        "bfw XOR mjb -> z00\n"
        "x03 OR x00 -> vdt\n"
        "gnj AND wpb -> z02\n"
        "x04 AND y00 -> kjc\n"
        "djm OR pbm -> qhw\n"
        "nrd AND vdt -> hwm\n"
        "kjc AND fst -> rvg\n"
        "y04 OR y02 -> fgs\n"
        "y01 AND x02 -> pbm\n"
        "ntg OR kjc -> kwq\n"
        "psh XOR fgs -> tgd\n"
        "qhw XOR tgd -> z09\n"
        "pbm OR djm -> kpj\n"
        "x03 XOR y03 -> ffh\n"
        "x00 XOR y04 -> ntg\n"
        "bfw OR bqk -> z06\n"
        "nrd XOR fgs -> wpb\n"
        "frj XOR qhw -> z04\n"
        "bqk OR frj -> z07\n"
        "y03 OR x01 -> nrd\n"
        "hwm AND bqk -> z03\n"
        "tgd XOR rvg -> z12\n"
        "tnw OR pbm -> gnj\n"
    );
}

int main()
{
    runTests();

    std::string states;
    const auto num = solve(getInput(), &states);
    std::cout << num << std::endl;
    return 0;
}
