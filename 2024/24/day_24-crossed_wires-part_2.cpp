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

constexpr std::uint16_t k_NoGate(-1);

struct GateInfo {
    std::array<std::uint16_t, 3> d_inGates;
    std::uint16_t                d_outGate;
    GateInfo()
    : d_outGate(k_NoGate)
    {
        d_inGates.fill(k_NoGate);
    }
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

std::uint16_t minToken(const std::vector<std::string>& wires,
                       char                            token)
{
    const auto it = std::find_if(
             wires.begin(), wires.end(),
             [=](const std::string& wire) { return wire.starts_with(token); });
    return std::distance(wires.begin(), it);
}

std::uint16_t maxToken(const std::vector<std::string>& wires,
                       char                            token)
{
    const auto it = std::find_if(
             wires.rbegin(), wires.rend(),
             [=](const std::string& wire) { return wire.starts_with(token); });
    assert(it != wires.rend());
    return wires.size() - 1 - std::distance(wires.rbegin(), it);
}

std::uint64_t getNumber(const std::vector<std::string>& wires,
                        const std::vector<State>&       states,
                        char                            wireKey)
{
    std::uint64_t num = 0, numOutputs = 0;
    for (const auto& [wire, state] : std::views::zip(wires, states)
                                   | std::views::reverse) {
        if (wire.starts_with(wireKey)) {
            assert(state != e_Unset);
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

//..
//  Adder
//
//  x0--.
//       XOR->z0
//  y0--'
//
//  x0--.
//       AND->c1
//  y0--'
//
//  ...
//
//                  ci--.
//                       XOR->zi
//  xi--.            .--'
//       XOR->xyXi--'
//  yi--'           '---.
//                       AND->xyci--.
//                  ci--'            '.
//                                     OR->ci+1
//                  xi--.            .'
//                       AND->xyAi--'
//                  yi--'
//..
//
// o '?': intermediate output wire
//
// o 'e_And':
//     o output of '(xNN & yNN)' should be '?' (input to 1x OR gate)
//     o output of '(xi & yi)' should be '?' (input to 1x OR gate)
//     o output of '(? & ?)' should be '?' (input to 1x OR gate)
//
// o 'e_Or':
//     o output of '(? | ?)' should be:
//         o 'ci+1': input to 2x XOR gates
//         o 'zN': max z and input to no other gate
//
// o 'e_Xor':
//     o output of '(x0 ^ y0)' should be 'z0' (input to no other gate)
//     o output of '(xi ^ yi)' should be '?' (input to 1x AND gate and 1x XOR gate)
//     o output of '(? ^ ?)' should be zi (input to no other gate)
//
//
//   dwp: need XOR or OR, have AND
//
// ffj: need AND,       have XOR
// gjh: need AND,       have XOR
// kfm: need AND,       have XOR
//
// z08: need XOR,       have AND
// z22: need XOR,       have AND
// z31: need XOR,       have OR

std::string solve(const Input& input, std::uint16_t numSwaps, bool debug=false)
{
    const std::uint16_t x00 = minToken(input.d_wires, 'x'),
                        y00 = minToken(input.d_wires, 'y'),
                        zNN = maxToken(input.d_wires, 'z');
    std::vector<GateInfo> gateInfo(input.d_wires.size());
    for (std::uint16_t g = 0; g < input.d_gates.size(); ++g) {
        const auto [in1, in2, op, out] = input.d_gates[g];
        assert(gateInfo[in1].d_inGates[op] == k_NoGate);
        gateInfo[in1].d_inGates[op] = g;
        assert(gateInfo[in2].d_inGates[op] == k_NoGate);
        gateInfo[in2].d_inGates[op] = g;
        gateInfo[out].d_outGate = g;
    }
    const auto isZOutput = [&](std::uint16_t w) {
        const auto [a, o, x] = gateInfo[w].d_inGates;
        return (a == k_NoGate) && (o == k_NoGate) && (x == k_NoGate);
    };
    const auto isCarryInput = [&](std::uint16_t w) {
        const auto [a, o, x] = gateInfo[w].d_inGates;
        return (a == k_NoGate) && (o != k_NoGate) && (x == k_NoGate);
    };
    const auto isInnerInput = [&](std::uint16_t w) {
        const auto [a, o, x] = gateInfo[w].d_inGates;
        return (a != k_NoGate) && (o == k_NoGate) && (x != k_NoGate);
    };
    const auto isX00Y00 = [&](std::uint16_t in1, std::uint16_t in2) {
        const auto [i1, i2] = std::minmax(in1, in2);
        return (i1 == x00) && (i2 == y00);
    };
    std::vector<std::uint16_t> outOfPlace;
    const auto printGate = [&](std::ostream& stream, std::uint16_t g) {
        const auto [in1, in2, op, out] = input.d_gates[g];
        const auto& wires = input.d_wires;
        stream << '[' << wires[in1]
               << ((op == e_And) ? " AND "
                                 : (op == e_Or) ? " OR  " : " XOR ")
               << wires[in2] << " -> " << wires[out] << ']';
    };
    const auto markOutOfPlace = [&](int                     line,
                                    const std::string_view& reason,
                                    std::uint16_t           w) {
        if (debug) {
            std::cerr << line << ": output " << input.d_wires[w]
                      << " out of place ";
            printGate(std::cerr, gateInfo[w].d_outGate);
            std::cerr << " - " << reason << std::endl;
        }

        outOfPlace.push_back(w);
    };
    const auto isXY = [&](std::uint16_t w) {
        return input.d_wires[w].starts_with('x') ||
               input.d_wires[w].starts_with('y');
    };
    const auto isZ = [&](std::uint16_t w) {
        return input.d_wires[w].starts_with('z');
    };
    for (std::uint16_t w = 0; w < input.d_wires.size(); ++w) {
        const auto [counts, g] = gateInfo[w];
        if (g >= input.d_gates.size()) {
            assert(isXY(w));
            continue;
        }
        const auto [in1, in2, op, out] = input.d_gates[g];
        assert(out == w);
        if (isZOutput(w)) {
            if (!isZ(w)) {
                markOutOfPlace(__LINE__, "shoulde be Z", w);
            }
            else if (w == zNN) {
                if (op != e_Or) {
                    markOutOfPlace(__LINE__, "should be OR", w);
                }
            }
            else {
                if (op != e_Xor) {
                    markOutOfPlace(__LINE__, "should be XOR", w);
                }
            }
        }
        else if (isCarryInput(w)) {
            if (isZ(w)) {
                markOutOfPlace(__LINE__, "should be leaf", w);
            }
            else if (op != e_And) {
                markOutOfPlace(__LINE__, "should be AND", w);
            }
        }
        else if (isInnerInput(w)) {
            if (isZ(w)) {
                markOutOfPlace(__LINE__, "should be leaf", w);
            }
            else if (op == e_And) {
                if (!isX00Y00(in1, in2)) {
                    markOutOfPlace(__LINE__, "should be XOR or OR", w);
                }
            }
        }
        else {
            assert(!"invalid counts");
            std::unreachable();
        }
        if (op == e_Or) {
            for (const std::uint16_t in : { in1, in2 }) {
                const std::uint16_t gIn = gateInfo[in].d_outGate;
                if (gIn >= input.d_gates.size()) {
                    markOutOfPlace(__LINE__, "bad input (need inner)", in);
                }
                if (input.d_gates[gIn].d_op != e_And) {
                    markOutOfPlace(__LINE__, "bad input (need AND)", in);
                }
            }
        }
        else if (op == e_And) {
            if (!isCarryInput(w) && !isX00Y00(in1, in2)) {
                markOutOfPlace(__LINE__, "should only output to OR", w);
            }
        }
        else if (op == e_Xor) {
            if (isXY(in1)) {
                assert(isXY(in2));
                if (isX00Y00(in1, in2)) {
                    if (!isZ(w)) {
                        markOutOfPlace(__LINE__, "should be Z", w);
                    }
                }
                else if (isZ(w)) {
                    markOutOfPlace(__LINE__, "should not be Z", w);
                }
            }
            else if (!isZ(w)) {
                markOutOfPlace(__LINE__, "should be Z", w);
            }
        }
        else {
            assert(!"invalid op");
            std::unreachable();
        }
    }
    std::sort(outOfPlace.begin(), outOfPlace.end());
    outOfPlace.erase(std::unique(outOfPlace.begin(), outOfPlace.end()),
                     outOfPlace.end());
    assert(outOfPlace.size() == (numSwaps * 2));
    const char *sep = "";
    std::string swaps;
    for (std::uint16_t w : outOfPlace) {
        swaps += sep;
        swaps += input.d_wires[w];
        sep = ",";
    }
    return swaps;
}

int main()
{
    const auto num = solve(getInput(), 4);
    std::cout << num << std::endl;
    return 0;
}
