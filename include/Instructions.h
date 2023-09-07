#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H
#include "InstructionTypes.h"
#include "Registers.h"
#include <cassert>
#include <iomanip>
#include <iostream>

namespace {

static const std::pair<int, std::string>
parseOffsetReg(const std::string &input) {
  std::string numStr, identifier;

  size_t i = 0;
  while (i < input.size() && (std::isdigit(input[i]) || input[i] == '-')) {
    numStr.push_back(input[i]);
    i++;
  }

  while (i < input.size() && (std::isalnum(input[i]) || input[i] == '_')) {
    identifier.push_back(input[i]);
    i++;
  }

  int num;
  try {
    num = std::stoi(numStr);
  } catch (const std::exception &e) {
    throw std::runtime_error("Failed to parse number from string.");
  }

  return {num, identifier};
}

static const std::optional<std::bitset<5>> findReg(const std::string &RegStr) {
  if (auto RI = GPRegs.find(RegStr); RI != GPRegs.end())
    return RI->second;
  assert(false && "invalid register name");
  return std::nullopt;
}

} // namespace
class Instruction {
  unsigned Val;

public:
  void setVal(unsigned V) { Val = V; }
  unsigned getVal() { return Val; }
  void emitBinary(std::ostream &os) {
    os.write(reinterpret_cast<char *>(&Val), 4);
  }

  void dumpHex() {
    std::cerr << "Hex(LE): ";
    for (int i = 0; i < sizeof(Val); ++i) {
      unsigned char byte = (Val >> (i * 8)) & 0xFF;
      std::cerr << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte) << ' ';
    }
    std::cerr << "\n\n";
  }

  void dumpBin() {
    std::bitset<32> binaryValue(Val);
    std::cerr << "Binary(BE): ";
    for (int i = 0; i < 32; ++i) {
      std::cerr << ((binaryValue >> (31 - i)).to_ulong() & 1);
      if (i % 8 == 7)
        std::cerr << ' ';
    }
  }
  virtual void pprint(std::ostream &) = 0;
  virtual void exec(void) = 0;
  virtual ~Instruction() {}
};

class IInstruction : public Instruction {
private:
  const ISBType &IT;

  std::bitset<5> Rd, Rs1;
  std::bitset<12> Imm;

public:
  /// This is expected to be used on asm.
  IInstruction(const ISBType &IT, const std::vector<std::string> &Toks)
      : IT(IT) {
    Rd = *findReg(Toks[1]);

    // handle offset for loads
    if (Toks.size() == 3) {
      assert((IT.getMnemo() == "lb" || IT.getMnemo() == "lh" ||
              IT.getMnemo() == "lw" || IT.getMnemo() == "lbu" ||
              IT.getMnemo() == "lhu") &&
             "invarid offset(reg) notation except loads");
      // op rd, offset(rs1)
      auto OffReg = parseOffsetReg(Toks[2]);
      Rs1 = std::bitset<5>(OffReg.second);
      Imm = OffReg.first;
    } else {
      // op rd,rs1,imm
      Rs1 = *findReg(Toks[2]);
      Imm = stoi(Toks[3]);
    }

    // srai immediate
    if (Toks[0] == "srai")
      Imm |= 1 << 10;

    setVal((Imm.to_ulong() << 20) | (Rs1.to_ulong() << 15) |
           (IT.getFunct3().to_ulong() << 12) | (Rd.to_ulong() << 7) |
           IT.getOpcode().to_ulong());
    std::cerr << "Imm is " << getVal() << "\n";
  }
  void pprint(std::ostream &) override {
    assert(false && "unimplemented!");
    std::cerr << IT.getMnemo() << "\n";
    std::cerr << "TODO: pretty print for \n";
    unsigned V = getVal();
    for (int i = 0; i < 32; ++i) {
      if (i == 7 || i == 12 || i == 17 || i == 20 || i == 25)
        std::cerr << ' ';
      std::cerr << (V >> (31 - i) & 1);
    }
  }
  void exec(void) override { assert(false && "unimplemented!"); }
};

class RInstruction : public Instruction {
private:
  const RType &RT;

  std::bitset<5> Rd, Rs1, Rs2;

public:
  /// This is expected to be used on asm.
  RInstruction(const RType &RT, const std::vector<std::string> &Toks) : RT(RT) {
    Rd = *findReg(Toks[1]);
    Rs1 = *findReg(Toks[2]);
    Rs2 = *findReg(Toks[3]);
    setVal((RT.getFunct7().to_ulong() << 25) | (Rs2.to_ulong() << 20) |
           (Rs1.to_ulong() << 15) | (RT.getFunct3().to_ulong() << 12) |
           (Rd.to_ulong() << 7) | RT.getOpcode().to_ulong());
  }
  void pprint(std::ostream &) override {
    std::cerr << RT.getMnemo() << "\n";
    std::cerr << "TODO: clean \n";
    unsigned V = getVal();
    for (int i = 0; i < 32; ++i) {
      if (i == 7 || i == 12 || i == 17 || i == 20 || i == 25)
        std::cerr << ' ';
      std::cerr << (V >> (31 - i) & 1);
    }
    assert(false && "unimplemented!");
  }
  void exec(void) override { assert(false && "unimplemented!"); }
};

class UInstruction : public Instruction {
private:
  const UJType &UT;

  std::bitset<5> Rd;
  std::bitset<20> Imm;

public:
  /// This is expected to be used on asm.
  UInstruction(const UJType &UT, const std::vector<std::string> &Toks)
      : UT(UT) {
    Rd = *findReg(Toks[1]);
    Imm = stoi(Toks[2]);
    setVal((Imm.to_ulong() << 12) | (Rd.to_ulong() << 7) |
           UT.getOpcode().to_ulong());
  }
  void pprint(std::ostream &) override {
    std::cerr << UT.getMnemo() << "\n";
    std::cerr << "TODO: clean \n";
    unsigned V = getVal();
    for (int i = 0; i < 32; ++i) {
      if (i == 7 || i == 12 || i == 17 || i == 20 || i == 25)
        std::cerr << ' ';
      std::cerr << (V >> (31 - i) & 1);
    }
    assert(false && "unimplemented!");
  }
  void exec(void) override { assert(false && "unimplemented!"); }
};

#endif