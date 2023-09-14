#include "Instructions.h"
void IInstruction::exec(Address &P, GPRegisters &GPRegs, Memory &M,
                        CustomRegisters &) {
  std::string Mnemo = IT.getMnemo();
  int ImmI = signExtend(Imm);
  if (Mnemo == "addi") {
    GPRegs[Rd.to_ulong()] = GPRegs[Rs1.to_ulong()] + ImmI;
    P += 4;
  } else if (Mnemo == "slti") {
    GPRegs[Rd.to_ulong()] = (signed)GPRegs[Rs1.to_ulong()] < ImmI;
    P += 4;
  } else if (Mnemo == "sltiu") {
    GPRegs[Rd.to_ulong()] = (unsigned)GPRegs[Rs1.to_ulong()] < ImmI;
    P += 4;
  } else if (Mnemo == "xori") {
    GPRegs[Rd.to_ulong()] = (unsigned)GPRegs[Rs1.to_ulong()] ^ ImmI;
    P += 4;
  } else if (Mnemo == "ori") {
    GPRegs[Rd.to_ulong()] =
        (unsigned)GPRegs[Rs1.to_ulong()] | ImmI; // FIXME: sext?
    P += 4;
  } else if (Mnemo == "andi") {
    GPRegs[Rd.to_ulong()] =
        (unsigned)GPRegs[Rs1.to_ulong()] & ImmI; // FIXME: sext?
    P += 4;
  } else if (Mnemo == "jalr") {
    // FIXME: should addresss calculation be wrapped?
    GPRegs[Rd.to_ulong()] = P + 4;
    P = (GPRegs[Rs1.to_ulong()] + ImmI) & ~1;
  } else if (Mnemo == "lb")
    assert(false && "unimplemented!");
  else if (Mnemo == "lh")
    assert(false && "unimplemented!");
  else if (Mnemo == "lw") {
    Word V = M.readWord(GPRegs[Rs1.to_ulong()] + ImmI);
    GPRegs[Rd.to_ulong()] = V;
    P += 4;
  } else if (Mnemo == "lbu")
    assert(false && "unimplemented!");
  else if (Mnemo == "lhu")
    assert(false && "unimplemented!");
  // FIXME: shamt
  else if (Mnemo == "slli") {
    GPRegs[Rd.to_ulong()] = (unsigned)GPRegs[Rs1.to_ulong()]
                            << Imm.to_ulong(); // FIXME: sext?
    P += 4;
  } else if (Mnemo == "srli") {
    GPRegs[Rd.to_ulong()] =
        (unsigned)GPRegs[Rs1.to_ulong()] >> Imm.to_ulong(); // FIXME: sext?
    P += 4;
  } else if (Mnemo == "srai") {
    GPRegs[Rd.to_ulong()] =
        (signed)GPRegs[Rs1.to_ulong()] >> Imm.to_ulong(); // FIXME: sext?
    P += 4;
  } else
    assert(false && "unimplemented! or not exist");
}
void UInstruction::exec(Address &P, GPRegisters &GPRegs, Memory &M,
                        CustomRegisters &) {
  std::string Mnemo = UT.getMnemo();
  int ImmI = signExtend(Imm);
  if (Mnemo == "auipc") {
    GPRegs[Rd.to_ulong()] = P + (ImmI & 0xfffff000);
    P += 4;
  } else
    assert(false && "unimplemented! or not exist");
}

void SInstruction::exec(Address &P, GPRegisters &GPRegs, Memory &M,
                        CustomRegisters &) {
  std::string Mnemo = ST.getMnemo();

  int ImmI = signExtend(Imm);
  if (Mnemo == "sw") {
    // FIXME: wrap?
    Address Ad = GPRegs[Rs1.to_ulong()] + ImmI;
    M.writeWord(Ad, GPRegs[Rs2.to_ulong()]);
    P += 4;
  } else
    assert(false && "unimplemented! or not exist");
}
