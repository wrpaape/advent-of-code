#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

enum Instruction {
    e_adv = 0,
    e_bxl = 1,
    e_bst = 2,
    e_jnz = 3,
    e_bxc = 4,
    e_out = 5,
    e_bdv = 6,
    e_cdv = 7,
};

struct Computer {
    std::int64_t             d_regA;
    std::int64_t             d_regB;
    std::int64_t             d_regC;
    std::int64_t             d_ip;
    std::vector<std::int8_t> d_prog;
    std::vector<std::int8_t> d_output;
    auto operator<=>(const Computer&) const = default;
};

void readRegister(std::int64_t& value, std::istream& stream, char reg)
{
    std::string line;
    std::getline(stream, line);
    assert(stream);
    const std::string prefix = std::string("Register ") + reg + ": ";
    assert(line.starts_with(prefix));
    value = std::stoll(line.substr(prefix.size()));
}

void readEmptyLine(std::istream& stream)
{
    std::string line;
    std::getline(stream, line);
    assert(stream);
    assert(line.empty());
}

void readProgram(std::vector<std::int8_t>& program, std::istream& stream)
{
    std::string buffer;
    std::getline(stream, buffer);
    assert(stream);
    const std::string_view prefix("Program: ");
    assert(buffer.starts_with(prefix));
    program.clear();
    for (std::istringstream pStream(buffer.substr(prefix.size()));
                                    std::getline(pStream, buffer, ','); ) {
        assert(buffer.size() == 1);
        assert(buffer[0] >= '0');
        assert(buffer[0] <= '7');
        program.push_back(buffer[0]-'0');
    }
}

auto getInput(std::istream& stream)
{
    Computer computer;
    readRegister(computer.d_regA, stream, 'A');
    readRegister(computer.d_regB, stream, 'B');
    readRegister(computer.d_regC, stream, 'C');
    readEmptyLine(stream);
    computer.d_ip = 0;
    readProgram(computer.d_prog, stream);
    return computer;
}

auto getInput()
{
    std::ifstream stream("day_17-chronospatial_computer-input.txt");
    return getInput(stream);
}

auto getInput(const std::string_view& input)
{
    std::istringstream stream{std::string(input)};
    return getInput(stream);
}

std::int64_t getLiteralOperand(const Computer& computer)
{
    const std::int64_t i = computer.d_ip + 1;
    assert(i >= 0);
    assert(i < computer.d_prog.size());
    return computer.d_prog[i];
}

std::int64_t getComboOperand(const Computer& computer)
{
    const std::int64_t i = computer.d_ip + 1;
    assert(i >= 0);
    assert(i < computer.d_prog.size());
    const std::int64_t operand = computer.d_prog[i];
    assert(operand >= 0);
    assert(operand < 7);
    switch (operand) {
      case 0:
      case 1:
      case 2:
      case 3:
        return operand;
      case 4:
        return computer.d_regA;
      case 5:
        return computer.d_regB;
      case 6:
        return computer.d_regC;
    }
    assert(!"invalid combo operand");
    std::unreachable();
}

void execInstruction(Computer& computer)
{
    assert(computer.d_ip >= 0);
    assert(computer.d_ip < computer.d_prog.size());
    const std::int64_t op = computer.d_prog[computer.d_ip];
    assert(op >= 0);
    assert(op <= 7);

    const auto rightShiftA = [&]() {
        const std::int64_t num      = computer.d_regA,
                           denShift = getComboOperand(computer);
        assert(denShift >= 0);
        assert(denShift <= 62);
        const std::int64_t den = 1LL << denShift;
        assert(den > 0);
        return num / den;
    };
    switch (op) {
      case e_adv: {
          computer.d_regA = rightShiftA();
      } break;
      case e_bxl: {
        computer.d_regB ^= getLiteralOperand(computer);
      } break;
      case e_bst: {
        computer.d_regB = getComboOperand(computer) % 8;
      } break;
      case e_jnz: {
        if (computer.d_regA != 0) {
            computer.d_ip = getLiteralOperand(computer);
            return;
        }
      } break;
      case e_bxc: {
        computer.d_regB ^= computer.d_regC;
      } break;
      case e_out: {
        computer.d_output.push_back(getComboOperand(computer) % 8);
      } break;
      case e_bdv: {
          computer.d_regB = rightShiftA();
      } break;
      case e_cdv: {
          computer.d_regC = rightShiftA();
      } break;
      default: {
        assert(!"invalid operator");
        std::unreachable();
      };
    }
    computer.d_ip += 2;
}

void runSim(Computer& computer)
{
    while (computer.d_ip < computer.d_prog.size())
        execInstruction(computer);
}

bool isValidOutput(const std::vector<std::int8_t>& output)
{
    return std::all_of(output.begin(), output.end(), [](std::int8_t value) {
                           return (value >= 0) && (value <= 7);
                       });
}

std::string displayOutput(const std::vector<std::int8_t>& output)
{
    assert(isValidOutput(output));

    std::string display(2*output.size()-!output.empty(), ',');
    for (std::size_t i = 0; i < output.size(); ++i) {
        const std::int8_t value = output[i];
        assert(value >= 0);
        assert(value <= 7);
        display[2*i] = value+'0';
    }
    return display;
}

std::vector<std::int8_t> tryRun(const Computer& computer, std::int64_t regA)
{
    Computer state = computer;
    state.d_regA   = regA;
    runSim(state);
    return std::move(state.d_output);
}

std::int64_t nextPowerOf8(std::int64_t value)
{
    std::int64_t power;
    for (power = 1; power <= value; power *= 8)
        continue;

    return power;
}

std::int64_t solve(const Computer& computer)
{
    std::int64_t low = 1, high = std::numeric_limits<std::int64_t>::max()-1;
    while (low < high) {
        const std::int64_t mid = std::midpoint(low, high+1);
        if (tryRun(computer, mid).size() < computer.d_prog.size()) {
            low  = mid;
        }
        else {
            high = mid - 1;
        }
    }
    std::int64_t base = nextPowerOf8(low), total = base;
    assert(tryRun(computer, base-1).size() == (computer.d_prog.size()-1));
    assert(tryRun(computer, base)  .size() == computer.d_prog.size());
    assert(tryRun(computer, base*8).size() == (computer.d_prog.size()+1));
    for (std::int64_t pos = std::int64_t(computer.d_prog.size())-1; pos >= 0; --pos, base /= 8) {
        assert(base > 0);
        for (std::int64_t mult = 0; ; ++mult) {
            const std::int64_t             nextTotal = total + (base * mult);
            const std::vector<std::int8_t> output    = tryRun(computer, nextTotal);
            assert(output.size() >= computer.d_prog.size());
            assert(output.size() <= computer.d_prog.size());
            if (output[pos] == computer.d_prog[pos]) {
                total = nextTotal;
                break;
            }
        }
    }
    assert(tryRun(computer, total) == computer.d_prog);
    return total;
}

void check(int                     lineNumber,
           std::int64_t            expectedRegA,
           const std::string_view& input)
{
    const Computer     computer = getInput(input);
    const std::int64_t regA     = solve(computer);

    if (regA != expectedRegA) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected:\n" << expectedRegA
                  << "\n> actual:\n" << regA
                  << std::endl;
    }
}

void runTests()
{
	check(
        __LINE__,
        117440,
        "Register A: 2024\n"
        "Register B: 0\n"
        "Register C: 0\n"
        "\n"
        "Program: 0,3,5,4,3,0\n"
    );
}

int main()
{
    runTests();

    const Computer     computer = getInput();
    const std::int64_t regA     = solve(computer);
    std::cout << regA << std::endl;
    return 0;
}
