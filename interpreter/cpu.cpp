#include "cpu.h"

CPU8086::Intel8086CPU()
    : memory(1024 * 1024, 0), stack(64 * 1024, 0) // 1MB memory, 64KB stack
{
    // Initialize general-purpose and segment registers
    for (const auto& reg : {
        "AX", "BX", "CX", "DX", "SI", "DI", "BP", "SP",
        "IP", "CS", "DS", "SS", "ES"
    }) {
        registers[reg] = 0;
    }

    // Initialize FLAGS register bits
    for (const auto& flag : {
        "CF", "PF", "AF", "ZF", "SF", "TF", "IF", "DF", "OF"
    }) {
        flags[flag] = 0;
    }

    // Initialize SP to top of stack
    registers["SP"] = 0xFFFE;
}

uint16_t CPU8086::getRegister(const std::string& regName) const {
    if (registers.contains(regName)) {
        return registers.at(regName);
    }

    // Byte-level access (e.g., AH, AL, BH, BL)
    std::string upper = regName.substr(0, 1) + "X";
    if (!registers.contains(upper))
        throw std::invalid_argument("Invalid register: " + regName);

    uint16_t full = registers.at(upper);

    if (regName[1] == 'H') {
        return (full >> 8) & 0xFF;
    } else if (regName[1] == 'L') {
        return full & 0xFF;
    } else {
        throw std::invalid_argument("Invalid 8-bit register: " + regName);
    }
}

void CPU8086::setRegister(const std::string& regName, uint16_t value) {
    if (registers.contains(regName)) {
        registers[regName] = value;
        return;
    }

    // Byte-level access
    std::string upper = regName.substr(0, 1) + "X";
    if (!registers.contains(upper))
        throw std::invalid_argument("Invalid register: " + regName);

    uint16_t full = registers[upper];

    if (regName[1] == 'H') {
        full = (full & 0x00FF) | ((value & 0xFF) << 8);
    } else if (regName[1] == 'L') {
        full = (full & 0xFF00) | (value & 0xFF);
    } else {
        throw std::invalid_argument("Invalid 8-bit register: " + regName);
    }

    registers[upper] = full;
}

uint8_t CPU8086::getFlag(const std::string& flagName) const {
    auto it = flags.find(flagName);
    if (it == flags.end()) throw std::invalid_argument("Invalid flag: " + flagName);
    return it->second;
}

void CPU8086::setFlag(const std::string& flagName, uint8_t value) {
    if (value != 0 && value != 1) throw std::invalid_argument("Flag must be 0 or 1");
    auto it = flags.find(flagName);
    if (it == flags.end()) throw std::invalid_argument("Invalid flag: " + flagName);
    it->second = value;
}

uint8_t CPU8086::readMemory(uint32_t addr) const {
    if (addr >= memory.size()) throw std::out_of_range("Memory read out of bounds");
    return memory[addr];
}

void CPU8086::setMemory(uint32_t addr, uint8_t value) {
    if (addr >= memory.size()) throw std::out_of_range("Memory write out of bounds");
    memory[addr] = value;
}

void CPU8086::pushWord(uint16_t value) {
    uint16_t& sp = registers["SP"];
    if (sp < 2) throw std::overflow_error("Stack overflow");
    sp -= 2;
    stack[sp] = value & 0xFF;
    stack[sp + 1] = (value >> 8) & 0xFF;
}

uint16_t CPU8086::popWord() {
    uint16_t& sp = registers["SP"];
    if (sp > 0xFFFC) throw std::underflow_error("Stack underflow");
    uint16_t low = stack[sp];
    uint16_t high = stack[sp + 1];
    sp += 2;
    return (high << 8) | low;
}

uint32_t Intel8086CPU::getPhysicalAddress(uint16_t segment, uint16_t offset) const {
    return ((uint32_t)segment << 4) + offset;
}
