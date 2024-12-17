#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
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

std::string runSim(Computer& computer)
{
    while (computer.d_ip < computer.d_prog.size())
        execInstruction(computer);

    return displayOutput(computer.d_output);
}

void checkInstruction(int             lineNumber,
                      const Computer& expectedOutputState,
                      const Computer& inputState)
{
    Computer outputState = inputState;
    execInstruction(outputState);
    if (outputState != expectedOutputState) {
        std::cerr << "Unexpected output state! (" << lineNumber << ")"
                  << std::endl;
    }
}

void check(int                            lineNumber,
           const std::string_view&        expectedOutput,
           const std::string_view&        input,
           const std::optional<Computer>& expectedOutputState = std::nullopt)
{
    Computer          computer = getInput(input);
    const std::string output   = runSim(computer);
    if (output != expectedOutput) {
        std::cerr << "failure(" << lineNumber << "):"
                  << "\n> expected:\n" << expectedOutput
                  << "\n> actual:\n" << output
                  << std::endl;
    }
    if (expectedOutputState && (computer != *expectedOutputState)) {
        std::cerr << "Unexpected output state! (" << lineNumber << ")"
                  << std::endl;
    }
}

void runTests()
{
    checkInstruction(
        __LINE__,
        { .d_regB = 1, .d_regC = 9, .d_ip = 2, .d_prog = { 2,6 } },
        { .d_regB = 0, .d_regC = 9, .d_ip = 0, .d_prog = { 2,6 } }
    );
	check(
        __LINE__,
        "0,1,2",
        "Register A: 10\n"
        "Register B: 0\n"
        "Register C: 0\n"
        "\n"
        "Program: 5,0,5,1,5,4\n"
    );
	check(
        __LINE__,
        "4,2,5,6,7,7,7,7,3,1,0",
        "Register A: 2024\n"
        "Register B: 0\n"
        "Register C: 0\n"
        "\n"
        "Program: 0,1,5,4,3,0\n",
        Computer{ .d_regA = 0, .d_ip = 6, .d_prog = { 0,1,5,4,3,0 },
                  .d_output = { 4,2,5,6,7,7,7,7,3,1,0 } }
    );
    checkInstruction(
        __LINE__,
        { .d_regB = 26, .d_ip = 2, .d_prog = { 1,7 } },
        { .d_regB = 29, .d_ip = 0, .d_prog = { 1,7 } }
    );
    checkInstruction(
        __LINE__,
        { .d_regB = 44354, .d_regC = 43690, .d_ip = 2, .d_prog = { 4,0 } },
        { .d_regB = 2024,  .d_regC = 43690, .d_ip = 0, .d_prog = { 4,0 } }
    );
	check(
        __LINE__,
        "4,6,3,5,6,3,5,2,1,0",
        "Register A: 729\n"
        "Register B: 0\n"
        "Register C: 0\n"
        "\n"
        "Program: 0,1,5,4,3,0\n"
    );
}

int main()
{
    runTests();

    auto              computer = getInput();
    const std::string output   = runSim(computer);
    std::cout << output << std::endl;
    return 0;
}
