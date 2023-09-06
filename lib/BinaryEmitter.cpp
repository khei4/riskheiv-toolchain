#include "BinaryEmitter.h"
#include "InstructionTypes.h"
#include "Instructions.h"
#include "Registers.h"
#include <cassert>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>

#ifdef DEBUG
#include "Debug.h"
#endif

void BinaryEmitter::emitBinary(std::ostream &os) {
  while (AP.parseLine()) {
    auto &Toks = AP.getTokens();
    auto &Mnemo = Toks[0];

    unsigned Inst = 0;
    std::unique_ptr<Instruction> InstT;
    if (auto IT = ITypeKinds.find(Mnemo); IT != ITypeKinds.end()) {
      assert((Toks.size() == 4 || Toks.size() == 3) &&
             "Wrong token number I-Type Inst.");
      auto UI = std::make_unique<IInstruction>(IT->second, Toks);
      InstT = std::move(UI);
    } else if (auto IT = RTypeKinds.find(Mnemo); IT != RTypeKinds.end()) {
      assert(Toks.size() == 4 && "Wrong token number R-Type Inst.");
      auto UI = std::make_unique<RInstruction>(IT->second, Toks);
      InstT = std::move(UI);
    } else if (auto IT = UTypeKinds.find(Mnemo); IT != UTypeKinds.end()) {
      assert(Toks.size() == 3 && "Wrong token number for U-Type Inst.");
      UJType UTemp = IT->second;
      std::bitset<7> Opcode = UTemp.getOpcode();
      std::bitset<5> Rd = *findReg(Toks[1]);
      std::bitset<20> Imm = stoi(Toks[2]);
      Inst = (Imm.to_ulong() << 12) | (Rd.to_ulong() << 7) | Opcode.to_ulong();
    } else if (auto IT = JTypeKinds.find(Mnemo); IT != JTypeKinds.end()) {
      assert(Toks.size() == 3 && "Wrong token number for J-Type Inst.");
      UJType UTemp = IT->second;
      std::bitset<7> Opcode = UTemp.getOpcode();
      std::bitset<5> Rd = *findReg(Toks[1]);
      std::bitset<21> Imm = stoi(Toks[2]);
      unsigned M0 = 0b100000000000000000000;
      unsigned M1 = 0b000000000011111111110;
      unsigned M2 = 0b000000000100000000000;
      unsigned M3 = 0b011111111000000000000;
      unsigned ImmU = Imm.to_ulong();
      // FIXME: fold?
      Inst = (((ImmU & M0) >> 20) << 31) | (((ImmU & M1) >> 1) << 21) |
             (((ImmU & M2) >> 11) << 20) | (((ImmU & M3) >> 12) << 12) |
             (Rd.to_ulong() << 7) | Opcode.to_ulong();
    } else if (auto IT = STypeKinds.find(Mnemo); IT != STypeKinds.end()) {
      assert(Toks.size() == 3 && "Wrong token number for S-Type Inst.");
      ISBType STemp = IT->second;
      std::bitset<3> Funct3 = STemp.getFunct3();
      std::bitset<7> Opcode = STemp.getOpcode();

      std::bitset<5> Rs1, Rs2;
      std::bitset<12> Imm;
      // sb/h/w rs2,offset(rs1)
      Rs2 = *findReg(Toks[1]);
      auto OffReg = parseOffsetReg(Toks[2]);
      Rs1 = std::bitset<5>(OffReg.second);
      Imm = OffReg.first;
      unsigned M0 = 0b111111100000;
      unsigned M1 = 0b000000011111;
      unsigned ImmU = Imm.to_ulong();
      Inst = (((ImmU & M0) >> 5) << 25) | (Rs2.to_ulong() << 20) |
             (Rs1.to_ulong() << 15) | (Funct3.to_ulong() << 12) |
             ((ImmU & M1) << 7) | Opcode.to_ulong();
    } else if (auto IT = BTypeKinds.find(Mnemo); IT != BTypeKinds.end()) {
      ISBType BTemp = IT->second;
      std::bitset<3> Funct3 = BTemp.getFunct3();
      std::bitset<7> Opcode = BTemp.getOpcode();

      std::bitset<5> Rs1, Rs2;
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
      // FIXME: am I wrong?
      Inst = (((ImmU & M0) >> 12) << 31) | (((ImmU & M1) >> 5) << 25) |
             (Rs2.to_ulong() << 20) | (Rs1.to_ulong() << 15) |
             (Funct3.to_ulong() << 12) | (((ImmU & M2) >> 1) << 8) |
             (((ImmU & M3) >> 11) << 7) | Opcode.to_ulong();
    } else {
      std::cerr << Mnemo << "\n";
      assert(false && "unimplemented!");
    }

// TODO: create DEBUGEXPR MACRO to shorten this
    if (InstT != nullptr) {
      InstT->emitBinary(os);
#ifdef DEBUG
      debugInst(Toks, InstT->getVal());
#endif
      InstT.reset();
    } else {
#ifdef DEBUG
    debugInst(Toks, Inst);
#endif
    os.write(reinterpret_cast<char *>(&Inst), 4);
    }
  }
}