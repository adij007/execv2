#include "cpu.h"
#include <algorithm>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

enum class AddressingMode {
  Immediate,
  Register,
  DirectMemory,
  RegisterIndirect,
  Based,
  Indexed,
  BasedIndexed,
  Implicit,
  Unknown
};

struct Operand {
  AddressingMode mode;
  uint16_t value;
  std::string text;
};

class OperandResolver {
public:
  OperandResolver(CPU8086 &cpuRef);
  Operand resolveOperand(const std::string &operandRaw);
  CPU8086 &cpu;
  static std::string trim(const std::string &str);
  static std::string toUpper(const std::string &str);
  static bool isRegister(const std::string &op);
  static bool isImmediate(const std::string &op);
  static uint16_t parseImmediate(const std::string &op);
  static bool isMemoryOperand(const std::string &op);
};

struct Instruction {
  std::string mnemonic;
  std::vector<Operand> operands;
};

class Tokenizer {
public:
  Tokenizer(CPU8086 &cpuRef);
  Instruction tokenizeLine(std::string &line);
  std::vector<Instruction> tokenizeBlock(std::string &line);

private:
  OperandResolver operandHelper;
  CPU8086 &cpu;
  std::string trim(const std::string &str);
  std::string toUpper(const std::string &str);
  std::vector<std::string> splitOperands(const std::string &line);
};
