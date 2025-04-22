#pragma once

#include <cstdint>
#include <iomanip> // for std::hex
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class CPU8086 {
public:
  CPU8086();

  // Getter for a 16-bit register
  uint16_t getRegister(const std::string &regName) const;

  // Setter for a 16-bit register
  void setRegister(const std::string &regName, uint16_t value);

  // Getter for a flag (0 or 1)
  uint8_t getFlag(const std::string &flagName) const;

  // Setter for a flag
  void setFlag(const std::string &flagName, uint8_t value);

  // Read memory at physical address
  uint8_t readMemory(uint32_t addr) const;

  // Write memory at physical address
  void setMemory(uint32_t addr, uint8_t value);

  // Stack operations
  void pushWord(uint16_t value);
  uint16_t popWord();

  // Memory and stack helpers
  uint32_t getPhysicalAddress(uint16_t segment, uint16_t offset) const;

  // Dump state as json
  std::string dumpStateAsJson() const;

private:
  std::unordered_map<std::string, uint16_t> registers;
  std::unordered_map<std::string, uint8_t> flags;

  std::vector<uint8_t> memory; // 1 MB main memory
  std::vector<uint8_t> stack;  // 64 KB stack
};
