#include <array>
#include <functional>
#include <cstdint>
#include <vector>

enum class Reg16 : uint8_t {
    AX, BX, CX, DX, 
    AH, AL, BL, BH, 
    CH, CL, DH, DL, 
    DI, SI, BP, SP
    COUNT // always keep at end
};

enum class Flag : uint8_t {
    CF, PF, AF, ZF, SF, TF, IF, DF, OF,
    COUNT
};

class CPU8086 {
private:
    // Register and flag storage
    std::array<uint16_t, static_cast<size_t>(Reg16::COUNT)> reg{};
    std::array<bool, static_cast<size_t>(Flag::COUNT)> flags{};
    std::array<uint8_t, static_cast<size_t>(4096)> memory;
public:
    // Access helpers
    uint16_t& getReg(Reg16 r) { return reg[static_cast<size_t>(r)]; }
    void setReg(Reg16 r, uint8_t val) { reg[static_cast<size_t>(r)] = val; }

    bool& getFlag(Flag f) { return flags[static_cast<size_t>(f)]; }
    void setFlag(Flag f, bool val) { flags[static_cast<size_t>(f)] = val; }

    uint8_t getMemory(uint16_t addr) { return memory[static_cast<size_t>(addr)]; }
    uint8_t getMemory(uint16_t addr, uint8_t val) { return memory[static_cast<size_t>(addr)] = val;}
};

enum class Mnemonic {
  AAA,AAD,AAM,AAS,ADC,ADD,AND,CALL,CBW,CLC,CLD,CLI,CMC,CMP,CMPSB,CMPSW,
  CWD,DAA,DAS,DEC,DIV,HLT,IDIV,IMUL,IN,INC,INT,INTO,IRET,
  JA,JAE,JB,JBE,JC,JCXZ,JE,JG,JGE,JL,JLE,JMP,
  JNA,JNAE,JNB,JNBE,JNC,JNE,JNG,JNGE,JNL,JNLE,JNO,JNP,JNS,JNZ,JO,JP,JPE,JPO,JS,JZ,
  LAHF,LDS,LEA,LES,LODSB,LODSW,LOOP,LOOPE,LOOPNE,LOOPNZ,LOOPZ,MOV,MOVSB,MOVSW,MUL,
  NEG,NOP,NOT,OR,OUT,POP,POPA,POPF,PUSH,PUSHA,PUSHF,
  RCL,RCR,REP,REPE,REPNE,REPNZ,REPZ,RET,RETF,ROL,ROR,
  SAHF,SAL,SAR,SBB,SCASB,SCASW,SHL,SHR,STC,STD,STI,STOSB,STOSW,SUB,
  TEST,XCHG,XLATB,XOR,
  COUNT
};

class Operand {
private:
  enum class OperandType { REGISTER, IMMEDIATE, SEGMENTED_MEMORY, MEMORY, LABEL };
  OperandType type;     
  auto value;

  //helper functions
  void match_to_reg16_enum(string op_str){
      if (op_str == "AX") {
          value = Reg16::AX;
      } else if (op_str == "BX") {
          value = Reg16::BX;
      } else if (op_str == "CX") {
          value = Reg16::CX;
      } else if (op_str == "DX") {
          value = Reg16::DX;
      } else if (op_str == "AH") {
          value = Reg16::AH;
      } else if (op_str == "AL") {
          value = Reg16::AL;
      } else if (op_str == "BH") {
          value = Reg16::BH;
      } else if (op_str == "BL") {
          value = Reg16::BL;
      } else if (op_str == "CH") {
          value = Reg16::CH;
      } else if (op_str == "CL") {
          value = Reg16::CL;
      } else if (op_str == "DH") {
          value = Reg16::CH;
      } else if (op_str == "DL") {
          value = Reg16::DL;
      } else if (op_str == "DI") {
          value = Reg16::DI;
      } else if (op_str == "SI") {
          value = Reg16::SI;
      } else if (op_str == "BP") {
          value = Reg16::BP;
      } else if (op_str == "SP") {
          value = Reg16::SP;
      }
    }

  int assemblyNumberToIntIntel(const std::string& numberToken) {
    std::smatch match;

    // Regex for hexadecimal (optional 0x, hex digits, optional h)
    std::regex hex_regex("^(0x)?([0-9a-fA-F]+)h?$");

    // Regex for decimal (one or more digits)
    std::regex dec_regex("^([0-9]+)$");

    // Regex for binary (one or more 0s or 1s, ending with b)
    std::regex bin_regex("^([01]+)b$");

    if (std::regex_match(numberToken, match, hex_regex)) {
        std::string hex_part = match[2].str();
        std::stringstream ss;
        ss << std::hex << hex_part;
        int decimalValue;
        ss >> decimalValue;
        return decimalValue;
    } else if (std::regex_match(numberToken, match, bin_regex)) {
        std::string bin_part = match[1].str();
        int decimalValue = 0;
        int power = 1;
        for (int i = bin_part.length() - 1; i >= 0; --i) {
            if (bin_part[i] == '1') {
                decimalValue += power;
            }
            power *= 2;
        }
        return decimalValue;
    } else if (std::regex_match(numberToken, match, dec_regex)) {
        std::stringstream ss(numberToken);
        int decimalValue;
        ss >> decimalValue;
        return decimalValue;
    } else {
        // If it doesn't match, return 0 or handle the error as you see fit
        return 0;
    }
  } 

public:
    Operand(string op){
      // Pseudo-patterns for parsing operands
        const std::regex regOperand(R"(\b(AL|AH|AX|BL|BH|BX|CL|CH|CX|DL|DH|DX|SI|DI|BP|SP|IP|CS|DS|ES|SS)\b)");
        const std::regex immOperand(R"((\d+|0x[0-9A-Fa-f]+|\d+[Hh]))");
        const std::regex memOperand(R"(\[([^\]]+)\])"); // matches [BX+SI], [1234H], etc.
        const std::regex segMemOperand(R"((CS|DS|ES|SS):\[(.*?)\])"); // optional segment prefix
    
      // Set type of Operand
        if (std::regex_match(op, regOperand)) {
            type = REGISTER;
            match_to_reg16_enum(op);
        } else if (std::regex_match(op, immOperand)) {
            type = IMMEDIATE;
            value = assemblyNumberToIntIntel(op);
        } else if (std::regex_match(op, segMemOperand)) {
            type = SEGMENTED_MEMORY;
            value = op;
        } else if (std::regex_match(op, memOperand)) {
            type = MEMORY;
            value = op;
        } else {
            type = LABEL; // fallback for control flow
        }
    }
};

class ASTNode {
public:
    Mnemonic mnemonic;
    std::vector<Operand> operands;

    ASTNode(string asm_line){
      std::istringstream iss(asm_line);
      std::string word;
      std::vector<std::string> tokens;

      while (iss >> word) {
        tokens.push_back(word);
      }

      size_t i = 0;
      mnemonic = tokens.at(i)
      //TODO parse mnemonic
      //TODO add operands to ASTNode
    } 

};

