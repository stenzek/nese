#pragma once
#include "types.h"

class Bus;
class String;
class System;

class CPU
{
public:
  static const u16 STACK_BASE = 0x0100;

  enum class AddressingMode
  {
    Implicit,
    Accumulator,
    Immediate,
    ZeroPage,
    ZeroPageX,
    ZeroPageY,
    Relative,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    Direct,
    Indirect,
    IndexedIndirect,
    IndirectIndexed
  };

  enum FLAG
  {
    FLAG_C = (1 << 0), // Carry
    FLAG_Z = (1 << 1), // Zero
    FLAG_I = (1 << 2), // Interrupt
    FLAG_D = (1 << 3), // Decimal
    FLAG_B = (1 << 4), // BRK
    FLAG_U = (1 << 5), // Unused
    FLAG_V = (1 << 6), // Overflow
    FLAG_N = (1 << 7), // Sign
  };

  struct Registers
  {
    uint8 A; // Accumulator
    uint8 X; // Index
    uint8 Y; // Index
    uint8 P; // Status
    uint8 S; // Stack
    u16 PC;  // Program Counter

    void SetFlagC(bool on)
    {
      if (on)
      {
        P |= FLAG_C;
      }
      else
      {
        P &= ~FLAG_C;
      }
    }
    void SetFlagZ(bool on)
    {
      if (on)
      {
        P |= FLAG_Z;
      }
      else
      {
        P &= ~FLAG_Z;
      }
    }
    void SetFlagI(bool on)
    {
      if (on)
      {
        P |= FLAG_I;
      }
      else
      {
        P &= ~FLAG_I;
      }
    }
    void SetFlagD(bool on)
    {
      if (on)
      {
        P |= FLAG_D;
      }
      else
      {
        P &= ~FLAG_D;
      }
    }
    void SetFlagB(bool on)
    {
      if (on)
      {
        P |= FLAG_B;
      }
      else
      {
        P &= ~FLAG_B;
      }
    }
    void SetFlagU(bool on)
    {
      if (on)
      {
        P |= FLAG_U;
      }
      else
      {
        P &= ~FLAG_U;
      }
    }
    void SetFlagV(bool on)
    {
      if (on)
      {
        P |= FLAG_V;
      }
      else
      {
        P &= ~FLAG_V;
      }
    }
    void SetFlagN(bool on)
    {
      if (on)
      {
        P |= FLAG_N;
      }
      else
      {
        P &= ~FLAG_N;
      }
    }
    bool GetFlagC() const { return (P & FLAG_C) != 0; }
    bool GetFlagZ() const { return (P & FLAG_Z) != 0; }
    bool GetFlagI() const { return (P & FLAG_I) != 0; }
    bool GetFlagD() const { return (P & FLAG_D) != 0; }
    bool GetFlagB() const { return (P & FLAG_B) != 0; }
    bool GetFlagU() const { return (P & FLAG_U) != 0; }
    bool GetFlagV() const { return (P & FLAG_V) != 0; }
    bool GetFlagN() const { return (P & FLAG_N) != 0; }
    void UpdateFlagZ(uint8 value) { SetFlagZ((value == 0)); }
    void UpdateFlagN(uint8 value) { SetFlagN((value & 0x80) ? true : false); }
  };

public:
  CPU();
  ~CPU();

  // register access
  const Registers* GetRegisters() const { return &m_registers; }

  // Total number of cycles executed since reset.
  u32 GetCyclesSinceReset() const { return m_cycle_counter; }

  // reset
  void Initialize(System* system, Bus* bus);
  void Reset();

  // Executes cycles.
  void Execute(CycleCount cycles);

  // disassemble an instruction
  bool Disassemble(String* pDestination, u16 address, u16* size);

  // trigger a NMI, IRQ
  void SetNMILine(bool state);
  void SetIRQLine(bool state);

  // stall the cpu for a number of cycles
  void Stall(u32 cycles);

private:
  // helpers
  inline uint8 ReadOperandByte() { return MemoryReadByte(m_registers.PC++); }
  inline u16 ReadOperandWord()
  {
    u16 res = MemoryReadWord(m_registers.PC);
    m_registers.PC += 2;
    return res;
  }
  inline void IncrementPC(u16 offset) { m_registers.PC += offset; }
  inline void SetPC(u16 address) { m_registers.PC = address; }

  // determine whether a memory read crosses a page
  inline bool PageCrossed(u16 a, u16 b) { return (a & 0xFF00) != (b & 0xFF00); }

  // memory read/write
  uint8 MemoryReadByte(u16 address);
  void MemoryWriteByte(u16 address, uint8 value);
  u16 MemoryReadWord(u16 address);
  u16 MemoryReadWordBug(u16 address);
  void MemoryWriteWord(u16 address, u16 value);

  // cycle consumer
  void AddCycles(u32 cycles);

  // stack push/pop with protection
  void PushByte(uint8 value);
  void PushWord(u16 value);
  uint8 PopByte();
  u16 PopWord();

  // interrupt handlers
  void HandleNMI();
  void HandleIRQ();

  void ExecuteInstruction();

  // instruction handlers
  void INSTR_unhandled();
  void INSTR_invalid();

  // nop
  inline void INSTR_NOP();

  // loads/stores
  inline void INSTR_LDA(uint8 value);
  inline void INSTR_LDX(uint8 value);
  inline void INSTR_LDY(uint8 value);
  inline uint8 INSTR_STA();
  inline uint8 INSTR_STX();
  inline uint8 INSTR_STY();

  // register transfers
  inline void INSTR_TAX();
  inline void INSTR_TAY();
  inline void INSTR_TSX();
  inline void INSTR_TXA();
  inline void INSTR_TXS();
  inline void INSTR_TYA();

  // interrupts
  inline void INSTR_BRK();
  inline void INSTR_RTI();

  // control
  inline void INSTR_JSR(u16 address);
  inline void INSTR_JMP(u16 address);
  inline void INSTR_RTS();
  inline void INSTR_BRx(bool predicate, int8 displacement);
  inline void INSTR_CMP(uint8 value);
  inline void INSTR_CPX(uint8 value);
  inline void INSTR_CPY(uint8 value);
  inline void INSTR_BIT(uint8 value);
  inline void INSTR_CLx(uint8 flag);
  inline void INSTR_SEx(uint8 flag);

  // alu
  inline uint8 INSTR_INC(uint8 value);
  inline void INSTR_INX();
  inline void INSTR_INY();
  inline uint8 INSTR_DEC(uint8 value);
  inline void INSTR_DEX();
  inline void INSTR_DEY();
  inline void INSTR_ADC(uint8 value);
  inline void INSTR_SBC(uint8 value);
  inline void INSTR_AND(uint8 value);
  inline void INSTR_ORA(uint8 value);
  inline void INSTR_EOR(uint8 value);
  inline uint8 INSTR_ASL(uint8 value);
  inline uint8 INSTR_LSR(uint8 value);
  inline uint8 INSTR_ROL(uint8 value);
  inline uint8 INSTR_ROR(uint8 value);

  // stack
  inline void INSTR_PHA();
  inline void INSTR_PHP();
  inline void INSTR_PLA();
  inline void INSTR_PLP();

  // undocumented instructions
  inline void INSTR_LAX(uint8 value);
  inline uint8 INSTR_SAX();
  inline uint8 INSTR_DCP(uint8 value);
  inline uint8 INSTR_ISB(uint8 value);
  inline uint8 INSTR_SLO(uint8 value);
  inline uint8 INSTR_RLA(uint8 value);
  inline uint8 INSTR_SRE(uint8 value);
  inline uint8 INSTR_RRA(uint8 value);

  // pointer to rest of system
  System* m_system = nullptr;
  Bus* m_bus = nullptr;

  // registers
  Registers m_registers = {};

  // clock values
  u32 m_cycle_counter = 0;
  u32 m_stall_cycles = 0;

  // nmi/irq pending
  bool m_nmi_pending = false;
  bool m_nmi_line_state = false;
  bool m_irq_line_state = false;

  // instruction wrappers
  template<void (CPU::*instruction)(uint8)>
  inline void WrapReadAccumulator();
  template<void (CPU::*instruction)(uint8)>
  inline void WrapReadImmediate();
  template<void (CPU::*instruction)(uint8)>
  inline void WrapReadAbsolute();
  template<void (CPU::*instruction)(uint8)>
  inline void WrapReadAbsoluteX();
  template<void (CPU::*instruction)(uint8)>
  inline void WrapReadAbsoluteY();
  template<void (CPU::*instruction)(uint8)>
  inline void WrapReadZeroPage();
  template<void (CPU::*instruction)(uint8)>
  inline void WrapReadZeroPageX();
  template<void (CPU::*instruction)(uint8)>
  inline void WrapReadZeroPageY();
  template<void (CPU::*instruction)(uint8)>
  inline void WrapReadIndexedIndirect();
  template<void (CPU::*instruction)(uint8)>
  inline void WrapReadIndirectIndexed();
  template<uint8 (CPU::*instruction)(uint8)>
  inline void WrapModifyAccumulator();
  template<uint8 (CPU::*instruction)(uint8)>
  inline void WrapModifyAbsolute();
  template<uint8 (CPU::*instruction)(uint8)>
  inline void WrapModifyAbsoluteX();
  template<uint8 (CPU::*instruction)(uint8)>
  inline void WrapModifyAbsoluteY();
  template<uint8 (CPU::*instruction)(uint8)>
  inline void WrapModifyZeroPage();
  template<uint8 (CPU::*instruction)(uint8)>
  inline void WrapModifyZeroPageX();
  template<uint8 (CPU::*instruction)(uint8)>
  inline void WrapModifyIndexedIndirect();
  template<uint8 (CPU::*instruction)(uint8)>
  inline void WrapModifyIndirectIndexed();
  template<uint8 (CPU::*instruction)()>
  inline void WrapWriteAbsolute();
  template<uint8 (CPU::*instruction)()>
  inline void WrapWriteAbsoluteX();
  template<uint8 (CPU::*instruction)()>
  inline void WrapWriteAbsoluteY();
  template<uint8 (CPU::*instruction)()>
  inline void WrapWriteZeroPage();
  template<uint8 (CPU::*instruction)()>
  inline void WrapWriteZeroPageX();
  template<uint8 (CPU::*instruction)()>
  inline void WrapWriteZeroPageY();
  template<uint8 (CPU::*instruction)()>
  inline void WrapWriteIndexedIndirect();
  template<uint8 (CPU::*instruction)()>
  inline void WrapWriteIndirectIndexed();
};
