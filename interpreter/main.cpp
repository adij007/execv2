#include "cpu.h"
#include "tokenizer.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string simulate8086(std::string &assemblyCode) {
  CPU8086 cpu;
  Tokenizer tokenizer(cpu);
  std::vector<Instruction> instructions = tokenizer.tokenizeBlock(assemblyCode);
  std::stringstream jsonStream;
  jsonStream << "[";

  bool firstInstruction = true;
  for (const auto &instruction : instructions) {
    if (!firstInstruction) {
      jsonStream << ",";
    }
    jsonStream << cpu.dumpStateAsJson();
    // For now, we'll just print the instruction.
    // In a real simulator, we would execute the instruction here
    std::cout << "Executing: " << instruction.mnemonic;
    for (const auto &operand : instruction.operands) {
      std::cout << " " << operand.text;
    }
    std::cout << std::endl;

    // Placeholder for instruction execution
    if (instruction.mnemonic == "MOV") {
      if (instruction.operands.size() == 2) {
        if (instruction.operands[0].mode == AddressingMode::Register &&
            instruction.operands[1].mode == AddressingMode::Immediate) {
          cpu.setRegister(instruction.operands[0].text,
                          instruction.operands[1].value);
        } else if (instruction.operands[0].mode == AddressingMode::Register &&
                   instruction.operands[1].mode == AddressingMode::Register) {
          cpu.setRegister(instruction.operands[0].text,
                          cpu.getRegister(instruction.operands[1].text));
        }
        // Add more MOV operand combinations as needed
      }
    } else if (instruction.mnemonic == "ADD") {
      if (instruction.operands.size() == 2 &&
          instruction.operands[0].mode == AddressingMode::Register &&
          instruction.operands[1].mode == AddressingMode::Immediate) {
        uint16_t regValue = cpu.getRegister(instruction.operands[0].text);
        cpu.setRegister(instruction.operands[0].text,
                        regValue + instruction.operands[1].value);
        // Implement flag updates here in a real simulator
      }
      // Add more ADD operand combinations as needed
    } else if (instruction.mnemonic == "PUSH") {
      if (instruction.operands.size() == 1 &&
          instruction.operands[0].mode == AddressingMode::Register) {
        cpu.pushWord(cpu.getRegister(instruction.operands[0].text));
      }
    } else if (instruction.mnemonic == "POP") {
      if (instruction.operands.size() == 1 &&
          instruction.operands[0].mode == AddressingMode::Register) {
        cpu.setRegister(instruction.operands[0].text, cpu.popWord());
      }
    }
    // Add implementations for other instructions

    firstInstruction = false;
  }

  jsonStream << "]";
  return jsonStream.str();
}

int main() {
  std::string assemblyCode = R"(
        MOV AX, 1234
        MOV BX, AX
        ADD AX, 5678
        PUSH BX
        POP CX
    )";

  std::string simulationResult = simulate8086(assemblyCode);
  std::cout << "Simulation Results (JSON):\n" << simulationResult << std::endl;

  return 0;
}
