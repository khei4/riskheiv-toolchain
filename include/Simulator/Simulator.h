
#ifndef SIMULATOR_H
#define SIMULATOR_H
#include "CPU.h"
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
  Registers Regs;
  std::vector<std::unique_ptr<Instruction>> Is;

public:
  Simulator(const Simulator &) = delete;
  Simulator &operator=(const Simulator &) = delete;

  Simulator(std::istream &is) {
    // TODO: parse binary
  }

  void dumpRegs() { Regs.dump(); }
};

#endif