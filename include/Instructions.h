#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H
#include "InstructionTypes.h"
#include "Registers.h"
#include "Simulator/CustomRegisters.h"
#include "Simulator/Memory.h"
#include <cassert>
#include <iomanip>
#include <iostream>
#include <optional>
#include <vector>

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
  if (auto RI = GPRegMap.find(RegStr); RI != GPRegMap.end())
    return RI->second;
  assert(false && "invalid register name");
  return std::nullopt;
}

} // namespace
using PC = unsigned;
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
  virtual void exec(PC &, GPRegisters &, Memory &, CustomRegisters &) = 0;
  virtual ~Instruction() {}
};
namespace {
template <std::size_t N> int signExtend(const std::bitset<N> &bs) {
  if (bs[N - 1]) {
    std::bitset<sizeof(Reg) * 8> extended = ~0;
    for (std::size_t i = 0; i < N; ++i) {
      extended[i] = bs[i];
    }
    return static_cast<int>(extended.to_ulong());
  } else {
    return static_cast<int>(bs.to_ulong());
  }
}

} // namespace

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
  }

  IInstruction(const ISBType &IT, const unsigned Rd, const unsigned Rs1,
               const unsigned Imm)
      : IT(IT), Rd(Rd), Rs1(Rs1), Imm(Imm) {
    // FIXME: rename member as _XX?
    setVal((this->Imm.to_ulong() << 20) | (this->Rs1.to_ulong() << 15) |
           (IT.getFunct3().to_ulong() << 12) | (this->Rd.to_ulong() << 7) |
           IT.getOpcode().to_ulong());
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
  void exec(PC &P, GPRegisters &GPRegs, Memory &M, CustomRegisters &) override {
    std::string Mnemo = IT.getMnemo();
    int ImmI = signExtend(Imm);
    if (Mnemo == "addi")
      GPRegs[Rd.to_ulong()] = GPRegs[Rs1.to_ulong()] + ImmI;
    else if (Mnemo == "slti")
      GPRegs[Rd.to_ulong()] = (signed)GPRegs[Rs1.to_ulong()] < ImmI;
    else if (Mnemo == "sltiu")
      GPRegs[Rd.to_ulong()] =
          (unsigned)GPRegs[Rs1.to_ulong()] < ImmI; // FIXME: sext?
    else if (Mnemo == "xori")
      GPRegs[Rd.to_ulong()] =
          (unsigned)GPRegs[Rs1.to_ulong()] ^ ImmI; // FIXME: sext?
    else if (Mnemo == "ori")
      GPRegs[Rd.to_ulong()] =
          (unsigned)GPRegs[Rs1.to_ulong()] | ImmI; // FIXME: sext?
    else if (Mnemo == "andi")
      GPRegs[Rd.to_ulong()] =
          (unsigned)GPRegs[Rs1.to_ulong()] & ImmI; // FIXME: sext?
    else if (Mnemo == "jalr")
      assert(false && "unimplemented!");
    else if (Mnemo == "lb")
      assert(false && "unimplemented!");
    else if (Mnemo == "lh")
      assert(false && "unimplemented!");
    else if (Mnemo == "lw")
      assert(false && "unimplemented!");
    else if (Mnemo == "lbu")
      assert(false && "unimplemented!");
    else if (Mnemo == "lhu")
      assert(false && "unimplemented!");
    // FIXME: shamt
    else if (Mnemo == "slli")
      GPRegs[Rd.to_ulong()] = (unsigned)GPRegs[Rs1.to_ulong()]
                              << Imm.to_ulong(); // FIXME: sext?
    else if (Mnemo == "srli")
      GPRegs[Rd.to_ulong()] =
          (unsigned)GPRegs[Rs1.to_ulong()] >> Imm.to_ulong(); // FIXME: sext?
    else if (Mnemo == "srai")
      GPRegs[Rd.to_ulong()] =
          (signed)GPRegs[Rs1.to_ulong()] >> Imm.to_ulong(); // FIXME: sext?
    else
      assert(false && "unimplemented! or not exist");
  }
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
  void exec(PC &, GPRegisters &, Memory &, CustomRegisters &) override {
    assert(false && "unimplemented!");
  }
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
  void exec(PC &, GPRegisters &, Memory &, CustomRegisters &) override {
    assert(false && "unimplemented!");
  }
};

class JInstruction : public Instruction {
private:
  const UJType &JT;

  std::bitset<5> Rd;
  std::bitset<21> Imm;

public:
  /// This is expected to be used on asm.
  JInstruction(const UJType &JT, const std::vector<std::string> &Toks)
      : JT(JT) {
    unsigned M0 = 0b100000000000000000000;
    unsigned M1 = 0b000000000011111111110;
    unsigned M2 = 0b000000000100000000000;
    unsigned M3 = 0b011111111000000000000;
    Rd = *findReg(Toks[1]);
    Imm = stoi(Toks[2]);
    unsigned ImmU = Imm.to_ulong();
    setVal((((ImmU & M0) >> 20) << 31) | (((ImmU & M1) >> 1) << 21) |
           (((ImmU & M2) >> 11) << 20) | (((ImmU & M3) >> 12) << 12) |
           (Rd.to_ulong() << 7) | JT.getOpcode().to_ulong());
  }
  void pprint(std::ostream &) override {
    std::cerr << JT.getMnemo() << "\n";
    std::cerr << "TODO: clean \n";
    unsigned V = getVal();
    for (int i = 0; i < 32; ++i) {
      if (i == 7 || i == 12 || i == 17 || i == 20 || i == 25)
        std::cerr << ' ';
      std::cerr << (V >> (31 - i) & 1);
    }
    assert(false && "unimplemented!");
  }
  void exec(PC &, GPRegisters &, Memory &, CustomRegisters &) override {
    assert(false && "unimplemented!");
  }
};

class SInstruction : public Instruction {
private:
  const ISBType &ST;

  std::bitset<5> Rs1, Rs2;
  std::bitset<12> Imm;

public:
  /// This is expected to be used on asm.
  SInstruction(const ISBType &ST, const std::vector<std::string> &Toks)
      : ST(ST) {

    // sb/h/w rs2,offset(rs1)
    Rs2 = *findReg(Toks[1]);
    auto OffReg = parseOffsetReg(Toks[2]);
    Rs1 = std::bitset<5>(OffReg.second);
    Imm = OffReg.first;
    unsigned M0 = 0b111111100000;
    unsigned M1 = 0b000000011111;
    unsigned ImmU = Imm.to_ulong();
    setVal((((ImmU & M0) >> 5) << 25) | (Rs2.to_ulong() << 20) |
           (Rs1.to_ulong() << 15) | (ST.getFunct3().to_ulong() << 12) |
           ((ImmU & M1) << 7) | ST.getOpcode().to_ulong());
  }
  void pprint(std::ostream &) override {
    std::cerr << ST.getMnemo() << "\n";
    std::cerr << "TODO: clean \n";
    unsigned V = getVal();
    for (int i = 0; i < 32; ++i) {
      if (i == 7 || i == 12 || i == 17 || i == 20 || i == 25)
        std::cerr << ' ';
      std::cerr << (V >> (31 - i) & 1);
    }
    assert(false && "unimplemented!");
  }
  void exec(PC &, GPRegisters &, Memory &, CustomRegisters &) override {
    assert(false && "unimplemented!");
  }
};

class BInstruction : public Instruction {
private:
  const ISBType &BT;

  std::bitset<5> Rs1, Rs2;
  std::bitset<13> Imm;

public:
  /// This is expected to be used on asm.
  BInstruction(const ISBType &BT, const std::vector<std::string> &Toks)
      : BT(BT) {

    Rs1 = *findReg(Toks[1]);
    // TODO: handle label branch
    Rs2 = *findReg(Toks[2]);

    std::bitset<13> Imm;
    unsigned M0 = 0b1000000000000;
    unsigned M1 = 0b0011111100000;
    unsigned M2 = 0b0000000011110;
    unsigned M3 = 0b0100000000000;
    Imm = stoi(Toks[3]);
    unsigned ImmU = Imm.to_ulong();
    setVal((((ImmU & M0) >> 12) << 31) | (((ImmU & M1) >> 5) << 25) |
           (Rs2.to_ulong() << 20) | (Rs1.to_ulong() << 15) |
           (BT.getFunct3().to_ulong() << 12) | (((ImmU & M2) >> 1) << 8) |
           (((ImmU & M3) >> 11) << 7) | BT.getOpcode().to_ulong());
  }
  void pprint(std::ostream &) override {
    std::cerr << BT.getMnemo() << "\n";
    std::cerr << "TODO: clean \n";
    unsigned V = getVal();
    for (int i = 0; i < 32; ++i) {
      if (i == 7 || i == 12 || i == 17 || i == 20 || i == 25)
        std::cerr << ' ';
      std::cerr << (V >> (31 - i) & 1);
    }
    assert(false && "unimplemented!");
  }
  void exec(PC &, GPRegisters &, Memory &, CustomRegisters &) override {
    assert(false && "unimplemented!");
  }
};

#endif
