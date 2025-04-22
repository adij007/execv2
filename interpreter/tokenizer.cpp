#include "tokenizer.h"
#include "cpu.h"

OperandResolver::OperandResolver(CPU8086 &cpuRef) : cpu(cpuRef) {}

Operand OperandResolver::resolveOperand(const std::string &operandRaw) {
  std::string operand = trim(operandRaw);

  if (isImmediate(operand)) {
    return {AddressingMode::Immediate, parseImmediate(operand), operand};
  }

  if (isRegister(operand)) {
    return {AddressingMode::Register, cpu.getRegister(toUpper(operand)),
            operand};
  }

  if (isMemoryOperand(operand)) {
    std::string content =
        toUpper(trim(operand.substr(1, operand.length() - 2)));

    if (isImmediate(content)) {
      // Direct memory access
      uint16_t addr = parseImmediate(content);
      return {AddressingMode::DirectMemory, cpu.readMemory(addr), content};
    }

    // Register Indirect
    if (isRegister(content)) {
      uint16_t addr = cpu.getRegister(content);
      return {AddressingMode::RegisterIndirect, cpu.readMemory(addr),
              "[" + content + "]"};
    }

    // Based-Indexed
    static const std::regex basedIndexed(
        R"(^(BX|BP)\s*\+\s*(SI|DI)(\s*\+\s*([0-9A-F]+H?))?$)",
        std::regex::icase);
    std::smatch match;
    if (std::regex_match(content, match, basedIndexed)) {
      uint16_t base = cpu.getRegister(match[1]);
      uint16_t index = cpu.getRegister(match[2]);
      uint16_t disp = match[4].matched ? parseImmediate(match[4]) : 0;
      uint16_t addr = base + index + disp;
      return {AddressingMode::BasedIndexed, cpu.readMemory(addr), content};
    }

    // Based
    static const std::regex based(R"(^(BX|BP)\s*\+\s*([0-9A-F]+H?)$)",
                                  std::regex::icase);
    if (std::regex_match(content, match, based)) {
      uint16_t base = cpu.getRegister(match[1]);
      uint16_t disp = parseImmediate(match[2]);
      return {AddressingMode::Based, cpu.readMemory(base + disp), content};
    }

    // Indexed
    static const std::regex indexed(R"(^(SI|DI)\s*\+\s*([0-9A-F]+H?)$)",
                                    std::regex::icase);
    if (std::regex_match(content, match, indexed)) {
      uint16_t index = cpu.getRegister(match[1]);
      uint16_t disp = parseImmediate(match[2]);
      return {AddressingMode::Indexed, cpu.readMemory(index + disp), content};
    }
  }

  return {AddressingMode::Unknown, 0xFFFF, operand};
}

std::string OperandResolver::trim(const std::string &str) {
  const auto begin = str.find_first_not_of(" \t\n\r");
  const auto end = str.find_last_not_of(" \t\n\r");
  return (begin == std::string::npos) ? "" : str.substr(begin, end - begin + 1);
}

std::string OperandResolver::toUpper(const std::string &str) {
  std::string up = str;
  std::transform(up.begin(), up.end(), up.begin(), ::toupper);
  return up;
}

bool OperandResolver::isRegister(const std::string &op) {
  static const std::regex reg(
      R"(^(AX|BX|CX|DX|SI|DI|BP|SP|AL|AH|BL|BH|CL|CH|DL|DH)$)",
      std::regex::icase);
  return std::regex_match(op, reg);
}

bool OperandResolver::isImmediate(const std::string &op) {
  static const std::regex imm(R"(^[0-9A-F]+H?$)", std::regex::icase);
  return std::regex_match(op, imm);
}

uint16_t OperandResolver::parseImmediate(const std::string &op) {
  std::string val = toUpper(op);
  if (val.back() == 'H')
    val.pop_back();
  return static_cast<uint16_t>(std::stoi(val, nullptr, 16));
}

bool OperandResolver::isMemoryOperand(const std::string &op) {
  return op.front() == '[' && op.back() == ']';
}

Tokenizer::Tokenizer(CPU8086 &cpuRef) : cpu(cpuRef), operandHelper(cpuRef) {}

Instruction Tokenizer::tokenizeLine(std::string &line) {

  Instruction instruction;
  std::istringstream iss(line);

  line = operandHelper.trim(line);

  size_t pos = line.find(' ');
  std::string mnemonic =
      (pos == std::string::npos) ? line : line.substr(0, pos);
  std::vector<std::string> operandTokens = splitOperands(line);

  std::vector<Operand> resolvedOperands;
  for (const std::string &op : operandTokens) {
    resolvedOperands.push_back(operandHelper.resolveOperand(op));
  }

  instruction = {operandHelper.toUpper(mnemonic), resolvedOperands};

  return instruction;
}

std::vector<Instruction> Tokenizer::tokenizeBlock(std::string &asmbl) {
  std::istringstream code(asmbl);
  std::vector<Instruction> Instructions;
  std::string line;
  while (std::getline(code, line)) {
    Instructions.push_back(tokenizeLine(line));
  }
  return Instructions;
}

std::vector<std::string> Tokenizer::splitOperands(const std::string &line) {
  size_t pos = line.find(' ');
  if (pos == std::string::npos)
    return {};

  std::string operandsPart = line.substr(pos + 1);
  std::vector<std::string> result;
  std::istringstream iss(operandsPart);
  std::string token;

  while (std::getline(iss, token, ',')) {
    result.push_back(operandHelper.trim(token));
  }

  return result;
}

// tokenizeBlock is the main function that return vector of instruction
