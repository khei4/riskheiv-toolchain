#include "Simulator/Simulator.h"
#include <gtest/gtest.h>

TEST(SimulatorTest, ADDI) {
  const unsigned char BYTES[] = {
      0x13, 0x08, 0x50, 0x00, // addi, x16, x0, 5
      0x93, 0x08, 0x30, 0x00, // addi, x17, x0, 3
  };

  const GPRegisters EXPECTED = {{16, 5}, {17, 3}};
  std::stringstream ss;
  ss.write(reinterpret_cast<const char *>(BYTES), sizeof(BYTES));

  Simulator Sim(ss);
  Sim.execFromDRAMBASE();
  GPRegisters &Res = Sim.getGPRegs();

  for (unsigned i = 0; i < 32; ++i) {
    EXPECT_EQ(Res[i], EXPECTED[i]);
  }
}

TEST(SimulatorTest, SLTI) {
  const unsigned char BYTES[] = {
      0x13, 0x08, 0xd0, 0xff, // addi, x16, x0, -3
      0x93, 0x28, 0xe8, 0xff, // slti  x17, x16, -2
      0x93, 0x27, 0xd8, 0xff, // slti  x15, x16, -3
  };

  const GPRegisters EXPECTED = {{16, -3}, {17, 1}};
  std::stringstream ss;
  ss.write(reinterpret_cast<const char *>(BYTES), sizeof(BYTES));

  Simulator Sim(ss);
  Sim.execFromDRAMBASE();
  GPRegisters &Res = Sim.getGPRegs();

  for (unsigned i = 0; i < 32; ++i) {
    EXPECT_EQ(Res[i], EXPECTED[i])
        << "Register:" << i << ", expected: " << EXPECTED[i]
        << ", got: " << Res[i];
  }
}

TEST(SimulatorTest, SLTIU) {
  const unsigned char BYTES[] = {
      0x13, 0x08, 0x50, 0x00, // addi, x16, x0, 5
      0x93, 0x38, 0x68, 0x00, // sltiu, x17, x16, 6
      0x93, 0x37, 0x58, 0x00, // sltiu x15, x16, 5
  };

  const GPRegisters EXPECTED = {{16, 5}, {17, 1}};
  std::stringstream ss;
  ss.write(reinterpret_cast<const char *>(BYTES), sizeof(BYTES));

  Simulator Sim(ss);
  Sim.execFromDRAMBASE();
  GPRegisters &Res = Sim.getGPRegs();

  for (unsigned i = 0; i < 32; ++i) {
    EXPECT_EQ(Res[i], EXPECTED[i])
        << "Register:" << i << ", expected: " << EXPECTED[i]
        << ", got: " << Res[i];
  }
}

TEST(SimulatorTest, XORI) {
  const unsigned char BYTES[] = {
      0x13, 0x08, 0x50, 0x00, // addi x16, x0, 5
      0x93, 0x48, 0x38, 0x00, // xori x17, x16, 3
      0x93, 0x47, 0xf8, 0xff, // xori x15, x16, -1
  };

  const GPRegisters EXPECTED = {{15, -6}, {16, 5}, {17, 6}};
  std::stringstream ss;
  ss.write(reinterpret_cast<const char *>(BYTES), sizeof(BYTES));

  Simulator Sim(ss);
  Sim.execFromDRAMBASE();
  GPRegisters &Res = Sim.getGPRegs();

  for (unsigned i = 0; i < 32; ++i) {
    EXPECT_EQ(Res[i], EXPECTED[i])
        << "Register:" << i << ", expected: " << EXPECTED[i]
        << ", got: " << Res[i];
  }
}

TEST(SimulatorTest, AUIPC) {
  const unsigned char BYTES[] = {
      0x97, 0x08, 0x00, 0x00, // auipc x17, 0
      0x17, 0x28, 0x00, 0x00, // auipc x16, 2
  };

  const GPRegisters EXPECTED = {{16, DRAM_BASE + 4 + 0x2000}, {17, DRAM_BASE}};
  std::stringstream ss;
  ss.write(reinterpret_cast<const char *>(BYTES), sizeof(BYTES));

  Simulator Sim(ss);
  Sim.execFromDRAMBASE();
  GPRegisters &Res = Sim.getGPRegs();

  for (unsigned i = 0; i < 32; ++i) {
    EXPECT_EQ(Res[i], EXPECTED[i])
        << "Register:" << i << ", expected: " << EXPECTED[i]
        << ", got: " << Res[i];
  }
}

TEST(SimulatorTest, SWLW) {
  const unsigned char BYTES[] = {
      0x13, 0x08, 0x00, 0x80, // addi x16, x0, -2048
      0x93, 0x08, 0x30, 0x00, // addi x17, x0, 3
      0x23, 0x2e, 0x01, 0xff, // sw x16, -4(sp)
      0x03, 0x29, 0xc1, 0xff, // lw x18, -4(sp)
  };

  const GPRegisters EXPECTED = {{16, -2048}, {17, 3}, {18, -2048}};
  std::stringstream ss;
  ss.write(reinterpret_cast<const char *>(BYTES), sizeof(BYTES));

  Simulator Sim(ss);
  Sim.execFromDRAMBASE();
  GPRegisters &Res = Sim.getGPRegs();

  for (unsigned i = 0; i < 32; ++i) {
    EXPECT_EQ(Res[i], EXPECTED[i])
        << "Register:" << i << ", expected: " << EXPECTED[i]
        << ", got: " << Res[i];
  }
}

TEST(SimulatorTest, JALR) {
  const unsigned char BYTES[] = {
      0x67, 0x09, 0xc0, 0x02, // jalr x18, x0, 44
  };

  const GPRegisters EXPECTED = {{18, DRAM_BASE + 4}};
  std::stringstream ss;
  ss.write(reinterpret_cast<const char *>(BYTES), sizeof(BYTES));

  Simulator Sim(ss);
  Sim.execFromDRAMBASE();
  GPRegisters &Res = Sim.getGPRegs();

  for (unsigned i = 0; i < 32; ++i) {
    EXPECT_EQ(Res[i], EXPECTED[i])
        << "Register:" << i << ", expected: " << EXPECTED[i]
        << ", got: " << Res[i];
  }
  EXPECT_EQ(Sim.getPC(), 44)
      << "PC"
      << ", expected: " << 44 << ", got: " << Sim.getPC();
}