
#ifndef SIMULATOR_H
#define SIMULATOR_H
#include "CPU.h"
#include "InstructionTypes.h"
#include "Instructions.h"
#include "Memory.h"
#include "Registers.h"
#include <memory>
#include <string>
#include <vector>

class Simulator {
private:
  CPU C;
  Memory M;
  unsigned CodeSize;
  GPRegisters GPRegs;
  CustomRegisters CRegs;
  std::map<Address, std::unique_ptr<Instruction>> PCInstMap;

public:
  Simulator(const Simulator &) = delete;
  Simulator &operator=(const Simulator &) = delete;

  Simulator(std::istream &is) {
    // TODO: parse per 2 bytes for compressed instructions
    char Buff[4];
    Address P = DRAM_BASE;
    while (is.read(Buff, 4)) {
      unsigned InstVal = *(reinterpret_cast<unsigned *>(Buff));
      // Raw inst
      M.writeWord(P, InstVal);
      // TODO: separate this as Decoder
      unsigned Opcode = InstVal & 0x0000007f;
      unsigned Rd = (InstVal & 0x00000f80) >> 7;
      unsigned Rs1 = (InstVal & 0x000f8000) >> 15;
      unsigned Rs2 = (InstVal & 0x01f00000) >> 20;
      unsigned Funct3 = (InstVal & 0x00007000) >> 12;
      unsigned Funct7 = (InstVal & 0xfe000000) >> 25;

      // FIXME: enough to set just mnemonic? & mask for srai would be another
      // time
      switch (Opcode) {
      case 0b0010011:
        switch (unsigned Imm = InstVal >> 20; Funct3) {
        case 0b000: // addi rd, rs1, imm
          PCInstMap.insert(
              {P, std::make_unique<IInstruction>(
                      ITypeKinds.find("addi")->second, Rd, Rs1, Imm)});
          break;
        case 0b010: // slti rd, rs1, imm
          PCInstMap.insert(
              {P, std::make_unique<IInstruction>(
                      ITypeKinds.find("slti")->second, Rd, Rs1, Imm)});
          break;
        case 0b011: // sltiu rd, rs1, imm
          PCInstMap.insert(
              {P, std::make_unique<IInstruction>(
                      ITypeKinds.find("sltiu")->second, Rd, Rs1, Imm)});
          break;
        case 0b100: // xori
          PCInstMap.insert(
              {P, std::make_unique<IInstruction>(
                      ITypeKinds.find("xori")->second, Rd, Rs1, Imm)});
          break;
        case 0b110: // ori
          assert(false && "unimplemented!");
          break;
        case 0b111: // andi
          assert(false && "unimplemented!");
          break;
        case 0b001: // slli
          assert(false && "unimplemented!");
          break;
        case 0b101:
          if (Funct7 == 0) // srli
                           // this will be 6 bit for RV64I
            // Is.push_back(std::make_unique<IInstruction>(
            //     ITypeKinds.find("srli")->second, Rd, Rs1,
            //     (InstVal >> 20) & 0b11111));
            assert(false && "unimplemented!");
          else // srai
            // Is.push_back(std::make_unique<IInstruction>(
            //     ITypeKinds.find("srai")->second, Rd, Rs1,
            //     (InstVal >> 20) & 0b11111));
            assert(false && "unimplemented!");
          break;
        default:
          assert(false && "unimplemented!");
          break;
        }
        break;
      case 0b1100111: // jalr
        PCInstMap.insert(
            {P, std::make_unique<IInstruction>(ITypeKinds.find("jalr")->second,
                                               Rd, Rs1, InstVal >> 20)});
        break;
      case 0b0000011:
        switch (unsigned Imm = InstVal >> 20; Funct3) {
        // TODO: other load insts
        case 0b010: // lw rd ,offset(rs1)
          PCInstMap.insert(
              {P, std::make_unique<IInstruction>(ITypeKinds.find("lw")->second,
                                                 Rd, Rs1, Imm)});
          break;
        default:
          assert(false && "unimplemented!");
          break;
        }
        break;
      case 0b0010111: // auipc
        PCInstMap.insert(
            {P, std::make_unique<UInstruction>(UTypeKinds.find("auipc")->second,
                                               Rd, InstVal & 0xfffff000)});
        break;
      case 0b0100011:
        switch (unsigned Offset =
                    (InstVal & 0xfe000000) >> 20 | ((InstVal >> 7) & 0x1f);
                Funct3) {
        case 0b010: // sw
          PCInstMap.insert(
              {P, std::make_unique<SInstruction>(STypeKinds.find("sw")->second,
                                                 Rs1, Rs2, Offset)});
          break;
        default:
          assert(false && "unimplemented!");
          break;
        }
        break;
      default:
        assert(false && "unimplemented!");
        break;
      }
      P += 4;
      CodeSize += 4;
    }
  }
  GPRegisters &getGPRegs() { return GPRegs; }

  void exec() {
    Address PC = DRAM_BASE;
    while (auto &I = PCInstMap[PC]) {
      I->exec(PC, GPRegs, M, CRegs);
    }
  }

  void dumpGPRegs() { GPRegs.dump(); }
};

#endif
