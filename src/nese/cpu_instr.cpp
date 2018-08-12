#include "YBaseLib/Assert.h"
#include "YBaseLib/Log.h"
#include "YBaseLib/Memory.h"
#include "YBaseLib/String.h"
#include "nese/cpu.h"
#include <cstdio>
Log_SetChannel(CPU);

template<void (CPU::*instruction)(u8)>
void CPU::WrapReadAccumulator()
{
  u8 operand = m_registers.A;
  (this->*instruction)(operand);
}

template<void (CPU::*instruction)(u8)>
void CPU::WrapReadImmediate()
{
  u8 operand = ReadOperandByte();
  (this->*instruction)(operand);
}

template<void (CPU::*instruction)(u8)>
void CPU::WrapReadAbsolute()
{
  u16 address = ReadOperandWord();
  u8 operand = MemoryReadByte(address);
  (this->*instruction)(operand);
}

template<void (CPU::*instruction)(u8)>
void CPU::WrapReadAbsoluteX()
{
  u16 base = ReadOperandWord();
  u16 address = base + m_registers.X;
  u8 operand = MemoryReadByte(address);
  if (PageCrossed(base, address))
    AddCycles(1);

  (this->*instruction)(operand);
}

template<void (CPU::*instruction)(u8)>
void CPU::WrapReadAbsoluteY()
{
  u16 base = ReadOperandWord();
  u16 address = base + m_registers.Y;
  u8 operand = MemoryReadByte(address);
  if (PageCrossed(base, address))
    AddCycles(1);

  (this->*instruction)(operand);
}

template<void (CPU::*instruction)(u8)>
void CPU::WrapReadZeroPage()
{
  u16 address = (u16)ReadOperandByte();
  u8 operand = MemoryReadByte(address);
  (this->*instruction)(operand);
}

template<void (CPU::*instruction)(u8)>
void CPU::WrapReadZeroPageX()
{
  u8 offset = ReadOperandByte();
  u16 address = ((u16)offset + m_registers.X) & 0xFF;
  AddCycles(1);

  u8 operand = MemoryReadByte(address);
  (this->*instruction)(operand);
}

template<void (CPU::*instruction)(u8)>
void CPU::WrapReadZeroPageY()
{
  u8 offset = ReadOperandByte();
  u16 address = ((u16)offset + m_registers.Y) & 0xFF;
  AddCycles(1);

  u8 operand = MemoryReadByte(address);
  (this->*instruction)(operand);
}

template<void (CPU::*instruction)(u8)>
void CPU::WrapReadIndexedIndirect()
{
  u8 base = ReadOperandByte();
  u16 pointer_address = (base + m_registers.X) & 0xFF;
  AddCycles(1);

  // high byte wraps around to 00 in zero page if low byte is stored in 0xFF
  u16 address = MemoryReadWordBug(pointer_address);

  u8 operand = MemoryReadByte(address);
  (this->*instruction)(operand);
}

template<void (CPU::*instruction)(u8)>
void CPU::WrapReadIndirectIndexed()
{
  u8 base = ReadOperandByte();

  // high byte wraps around to 00 in zero page if low byte is stored in 0xFF
  u16 base_address = MemoryReadWordBug(static_cast<u16>(base));

  u16 address = base_address + m_registers.Y;
  if (PageCrossed(base_address, address))
    AddCycles(1);

  u8 operand = MemoryReadByte(address);
  (this->*instruction)(operand);
}

template<u8 (CPU::*instruction)(u8)>
void CPU::WrapModifyAccumulator()
{
  m_registers.A = (this->*instruction)(m_registers.A);
  AddCycles(1);
}

template<u8 (CPU::*instruction)(u8)>
void CPU::WrapModifyAbsolute()
{
  u16 address = ReadOperandWord();
  u8 value = MemoryReadByte(address);

  value = (this->*instruction)(value);
  AddCycles(1);

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)(u8)>
void CPU::WrapModifyAbsoluteX()
{
  u16 base = ReadOperandWord();
  u16 address = base + m_registers.X;
  u8 value = MemoryReadByte(address);
  AddCycles(1);

  value = (this->*instruction)(value);
  AddCycles(1);

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)(u8)>
void CPU::WrapModifyAbsoluteY()
{
  u16 base = ReadOperandWord();
  u16 address = base + m_registers.Y;
  u8 value = MemoryReadByte(address);
  AddCycles(1);

  value = (this->*instruction)(value);
  AddCycles(1);

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)(u8)>
void CPU::WrapModifyZeroPage()
{
  u8 offset = ReadOperandByte();
  u16 address = (u16)offset;
  u8 value = MemoryReadByte(address);

  value = (this->*instruction)(value);
  AddCycles(1);

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)(u8)>
void CPU::WrapModifyZeroPageX()
{
  u8 offset = ReadOperandByte();
  u16 address = ((u16)offset + m_registers.X) & 0xFF;
  AddCycles(1);

  u8 value = MemoryReadByte(address);

  value = (this->*instruction)(value);
  AddCycles(1);

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)(u8)>
void CPU::WrapModifyIndexedIndirect()
{
  u8 base = ReadOperandByte();
  u16 pointer_address = (base + m_registers.X) & 0xFF;
  AddCycles(1);

  // high byte wraps around to 00 in zero page if low byte is stored in 0xFF
  u16 address = MemoryReadWordBug(pointer_address);

  u8 value = MemoryReadByte(address);
  value = (this->*instruction)(value);

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)(u8)>
void CPU::WrapModifyIndirectIndexed()
{
  u8 base = ReadOperandByte();

  // high byte wraps around to 00 in zero page if low byte is stored in 0xFF
  u16 base_address = MemoryReadWordBug(static_cast<u16>(base));

  u16 address = base_address + m_registers.Y;
  AddCycles(1);

  u8 value = MemoryReadByte(address);
  value = (this->*instruction)(value);

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)()>
void CPU::WrapWriteAbsolute()
{
  u16 address = ReadOperandWord();

  u8 value = (this->*instruction)();

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)()>
void CPU::WrapWriteAbsoluteX()
{
  u16 base = ReadOperandWord();
  u16 address = base + m_registers.X;
  AddCycles(1);

  u8 value = (this->*instruction)();

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)()>
void CPU::WrapWriteAbsoluteY()
{
  u16 base = ReadOperandWord();
  u16 address = base + m_registers.Y;
  AddCycles(1);

  u8 value = (this->*instruction)();

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)()>
void CPU::WrapWriteZeroPage()
{
  u16 address = (u16)ReadOperandByte();

  u8 value = (this->*instruction)();

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)()>
void CPU::WrapWriteZeroPageX()
{
  u8 offset = ReadOperandByte();
  u16 address = ((u16)offset + m_registers.X) & 0xFF;
  AddCycles(1);

  u8 value = (this->*instruction)();

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)()>
void CPU::WrapWriteZeroPageY()
{
  u8 offset = ReadOperandByte();
  u16 address = ((u16)offset + m_registers.Y) & 0xFF;
  AddCycles(1);

  u8 value = (this->*instruction)();

  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)()>
void CPU::WrapWriteIndexedIndirect()
{
  u8 base = ReadOperandByte();
  u16 pointer_address = (base + m_registers.X) & 0xFF;
  AddCycles(1);

  // high byte wraps around to 00 in zero page if low byte is stored in 0xFF
  u16 address = MemoryReadWordBug(pointer_address);

  u8 value = (this->*instruction)();
  MemoryWriteByte(address, value);
}

template<u8 (CPU::*instruction)()>
void CPU::WrapWriteIndirectIndexed()
{
  u8 base = ReadOperandByte();

  // high byte wraps around to 00 in zero page if low byte is stored in 0xFF
  u16 base_address = MemoryReadWordBug(static_cast<u16>(base));

  u16 address = base_address + m_registers.Y;
  AddCycles(1);

  u8 value = (this->*instruction)();
  MemoryWriteByte(address, value);
}

void CPU::INSTR_unhandled()
{
  SmallString disasm;
  u16 cur = m_registers.PC - 1;
  u16 size;
  for (u16 i = 0; i < 3; i++)
  {
    if (Disassemble(&disasm, cur, &size))
      Log_DevPrint(disasm.GetCharArray());
    else
      break;

    cur += size;
  }

  Panic("Invalid instruction");
}

void CPU::INSTR_invalid()
{
  SmallString disasm;
  if (Disassemble(&disasm, m_registers.PC - 1, nullptr))
    Log_DevPrint(disasm.GetCharArray());

  Panic("Unimplemented instruction");
}

// 2 cycles
void CPU::INSTR_NOP()
{
  AddCycles(1);
}

void CPU::INSTR_LDA(u8 value)
{
  m_registers.A = value;
  m_registers.SetFlagN(!!(value & 0x80));
  m_registers.SetFlagZ(!(value));
}

void CPU::INSTR_LDX(u8 value)
{
  m_registers.X = value;
  m_registers.SetFlagN(!!(value & 0x80));
  m_registers.SetFlagZ(!(value));
}

void CPU::INSTR_LDY(u8 value)
{
  m_registers.Y = value;
  m_registers.SetFlagN(!!(value & 0x80));
  m_registers.SetFlagZ(!(value));
}

u8 CPU::INSTR_STA()
{
  return m_registers.A;
}

u8 CPU::INSTR_STX()
{
  return m_registers.X;
}

u8 CPU::INSTR_STY()
{
  return m_registers.Y;
}

void CPU::INSTR_TAX()
{
  m_registers.X = m_registers.A;
  m_registers.SetFlagZ((m_registers.X == 0));
  m_registers.SetFlagN((m_registers.X & 0x80) != 0);
  AddCycles(1);
}

void CPU::INSTR_TAY()
{
  m_registers.Y = m_registers.A;
  m_registers.SetFlagZ((m_registers.Y == 0));
  m_registers.SetFlagN((m_registers.Y & 0x80) != 0);
  AddCycles(1);
}

void CPU::INSTR_TSX()
{
  m_registers.X = m_registers.S;
  m_registers.SetFlagZ((m_registers.X == 0));
  m_registers.SetFlagN((m_registers.X & 0x80) != 0);
  AddCycles(1);
}

void CPU::INSTR_TXA()
{
  m_registers.A = m_registers.X;
  m_registers.SetFlagZ((m_registers.A == 0));
  m_registers.SetFlagN((m_registers.A & 0x80) != 0);
  AddCycles(1);
}

void CPU::INSTR_TXS()
{
  m_registers.S = m_registers.X;
  AddCycles(1);
}

// 2 cycles
void CPU::INSTR_TYA()
{
  m_registers.A = m_registers.Y;
  m_registers.SetFlagZ((m_registers.A == 0));
  m_registers.SetFlagN((m_registers.A & 0x80) != 0);
  AddCycles(1);
}

// 7 cycles
void CPU::INSTR_BRK()
{
  PushWord(m_registers.PC);
  PushByte(m_registers.P | FLAG_B | FLAG_U);
  m_registers.SetFlagB(true);
  m_registers.SetFlagI(true);
  AddCycles(1);

  // quirk
  if (m_nmi_pending)
  {
    m_registers.PC = MemoryReadWord(0xFFFA);
    m_nmi_pending = false;
  }
  else
  {
    m_registers.PC = MemoryReadWord(0xFFFE);
  }
}

// 6 cycles
void CPU::INSTR_RTI()
{
  // TODO: Fixme
  // m_registers.P = PopByte() & ~(FLAG_U);
  m_registers.P = PopByte() | FLAG_U & (~0x10);
  m_registers.PC = PopWord();
  AddCycles(2);
}

// 6 cycles
void CPU::INSTR_JSR(u16 address)
{
  PushWord(m_registers.PC - 1);
  m_registers.PC = address;
  AddCycles(1);
}

// 6 cycles
void CPU::INSTR_RTS()
{
  m_registers.PC = PopWord() + 1;
  AddCycles(3);
}

// 3 cycles
void CPU::INSTR_PHA()
{
  PushByte(m_registers.A);
  AddCycles(1);
}

// 3 cycles
void CPU::INSTR_PHP()
{
  // FLAG_U is set if from a PHP/BRK, 0 if from interrupt
  PushByte(m_registers.P | FLAG_B | FLAG_U);
  AddCycles(1);
}

// 4 cycles
void CPU::INSTR_PLA()
{
  // N, Z flags
  m_registers.A = PopByte();
  m_registers.SetFlagN((m_registers.A & 0x80) != 0);
  m_registers.SetFlagZ((m_registers.A == 0x00));
  AddCycles(2);
}

void CPU::INSTR_PLP()
{
  // should clear these flags
  m_registers.P = PopByte();
  m_registers.P &= 0xEF;
  m_registers.P |= FLAG_U;
  AddCycles(2);
}

void CPU::INSTR_JMP(u16 address)
{
  m_registers.PC = address;
}

void CPU::INSTR_BRx(bool predicate, int8 displacement)
{
  if (predicate)
  {
    u16 old_address = m_registers.PC;
    u16 new_address = (displacement < 0) ? (m_registers.PC - (u16)-displacement) : (m_registers.PC + (u16)displacement);
    m_registers.PC = new_address;

    if (PageCrossed(old_address, new_address))
      AddCycles(2);
    else
      AddCycles(1);
  }
}

void CPU::INSTR_CMP(u8 value)
{
  s16 res = (s16)m_registers.A - (s16)value;
  m_registers.SetFlagN((res & 0x80) != 0);
  m_registers.SetFlagZ((res & 0xFF) == 0);
  m_registers.SetFlagC((res >= 0));
}

void CPU::INSTR_CPX(u8 value)
{
  s16 res = (s16)m_registers.X - (s16)value;
  m_registers.SetFlagN((res & 0x80) != 0);
  m_registers.SetFlagZ((res & 0xFF) == 0);
  m_registers.SetFlagC((res >= 0));
}

void CPU::INSTR_CPY(u8 value)
{
  s16 res = (s16)m_registers.Y - (s16)value;
  m_registers.SetFlagN((res & 0x80) != 0);
  m_registers.SetFlagZ((res & 0xFF) == 0);
  m_registers.SetFlagC((res >= 0));
}

void CPU::INSTR_BIT(u8 value)
{
  m_registers.SetFlagN((value & 0x80) != 0);
  m_registers.SetFlagV((value & 0x40) != 0);
  m_registers.SetFlagZ((value & m_registers.A) == 0);
}

void CPU::INSTR_CLx(u8 flag)
{
  m_registers.P &= ~flag;
  AddCycles(1);
}

void CPU::INSTR_SEx(u8 flag)
{
  m_registers.P |= flag;
  AddCycles(1);
}

u8 CPU::INSTR_INC(u8 value)
{
  value = value + 1;
  m_registers.SetFlagN((value & 0x80) != 0);
  m_registers.SetFlagZ(value == 0);
  return value;
}

void CPU::INSTR_INX()
{
  m_registers.X++;
  m_registers.SetFlagN((m_registers.X & 0x80) != 0);
  m_registers.SetFlagZ(m_registers.X == 0);
}

void CPU::INSTR_INY()
{
  m_registers.Y++;
  m_registers.SetFlagN((m_registers.Y & 0x80) != 0);
  m_registers.SetFlagZ(m_registers.Y == 0);
}

u8 CPU::INSTR_DEC(u8 value)
{
  value = value - 1;
  m_registers.SetFlagN((value & 0x80) != 0);
  m_registers.SetFlagZ(value == 0);
  return value;
}

void CPU::INSTR_DEX()
{
  m_registers.X--;
  m_registers.SetFlagN((m_registers.X & 0x80) != 0);
  m_registers.SetFlagZ(m_registers.X == 0);
}

void CPU::INSTR_DEY()
{
  m_registers.Y--;
  m_registers.SetFlagN((m_registers.Y & 0x80) != 0);
  m_registers.SetFlagZ(m_registers.Y == 0);
}

void CPU::INSTR_ADC(u8 value)
{
  u16 result = (u16)m_registers.A + value + (u16)m_registers.GetFlagC();
  m_registers.SetFlagV((~(m_registers.A ^ value) & (m_registers.A ^ result) & 0x80) != 0);
  m_registers.SetFlagC(result > 0xFF);
  m_registers.SetFlagZ((result & 0xFF) == 0);
  m_registers.SetFlagN((result & 0x80) != 0);
  m_registers.A = result & 0xFF;
}

void CPU::INSTR_SBC(u8 value)
{
  u16 result = (u16)m_registers.A + (u8)(~value) + u16(m_registers.GetFlagC());
  m_registers.SetFlagV((~(m_registers.A ^ (u8)(~value)) & (m_registers.A ^ result) & 0x80) != 0);
  m_registers.SetFlagC(result > 0xFF);
  m_registers.SetFlagZ((result & 0xFF) == 0);
  m_registers.SetFlagN((result & 0x80) != 0);
  m_registers.A = result & 0xFF;
}

void CPU::INSTR_AND(u8 value)
{
  m_registers.A &= value;
  m_registers.SetFlagZ(m_registers.A == 0);
  m_registers.SetFlagN((m_registers.A & 0x80) != 0);
}

void CPU::INSTR_ORA(u8 value)
{
  m_registers.A = m_registers.A | value;
  m_registers.SetFlagZ(m_registers.A == 0);
  m_registers.SetFlagN((m_registers.A & 0x80) != 0);
}

void CPU::INSTR_EOR(u8 value)
{
  m_registers.A ^= value;
  m_registers.SetFlagZ(m_registers.A == 0);
  m_registers.SetFlagN((m_registers.A & 0x80) != 0);
}

u8 CPU::INSTR_ASL(u8 value)
{
  m_registers.SetFlagC(!!(value & 0x80));
  value <<= 1;
  m_registers.SetFlagN(!!(value & 0x80));
  m_registers.SetFlagZ(!(value));
  return value;
}

u8 CPU::INSTR_LSR(u8 value)
{
  m_registers.SetFlagC(!!(value & 0x01));
  value >>= 1;
  m_registers.SetFlagN(!!(value & 0x80));
  m_registers.SetFlagZ(!(value));
  return value;
}

u8 CPU::INSTR_ROL(u8 value)
{
  u8 carry = (u8)m_registers.GetFlagC();
  m_registers.SetFlagC(!!(value & 0x80));
  value = (value << 1) | carry;
  m_registers.SetFlagN(!!(value & 0x80));
  m_registers.SetFlagZ(!(value));
  return value;
}

u8 CPU::INSTR_ROR(u8 value)
{
  u8 carry = (u8)m_registers.GetFlagC();
  m_registers.SetFlagC(!!(value & 0x01));
  value = (carry << 7) | (value >> 1);
  m_registers.SetFlagN(!!(value & 0x80));
  m_registers.SetFlagZ(!(value));
  return value;
}

void CPU::INSTR_LAX(u8 value)
{
  m_registers.A = value;
  m_registers.X = value;
  m_registers.SetFlagN(!!(value & 0x80));
  m_registers.SetFlagZ(!(value));
  AddCycles(1);
}

u8 CPU::INSTR_SAX()
{
  u8 result = m_registers.A & m_registers.X;
  return result;
}

u8 CPU::INSTR_DCP(u8 value)
{
  // DEC
  value = value - 1;

  // CMP
  s16 res = (s16)m_registers.A - (s16)value;
  m_registers.SetFlagN((res & 0x80) != 0);
  m_registers.SetFlagZ((res & 0xFF) == 0);
  m_registers.SetFlagC((res >= 0));

  return value;
}

u8 CPU::INSTR_ISB(u8 value)
{
  // INC
  value = value + 1;

  // SBC
  u16 result = (u16)m_registers.A + (u8)(~value) + u16(m_registers.GetFlagC());
  m_registers.SetFlagV((~(m_registers.A ^ (u8)(~value)) & (m_registers.A ^ result) & 0x80) != 0);
  m_registers.SetFlagC(result > 0xFF);
  m_registers.SetFlagZ((result & 0xFF) == 0);
  m_registers.SetFlagN((result & 0x80) != 0);
  m_registers.A = result & 0xFF;

  return value;
}

u8 CPU::INSTR_SLO(u8 value)
{
  // ASL
  m_registers.SetFlagC(!!(value & 0x80));
  value <<= 1;

  // ORA
  m_registers.A = m_registers.A | value;
  m_registers.SetFlagZ(m_registers.A == 0);
  m_registers.SetFlagN((m_registers.A & 0x80) != 0);

  return value;
}

u8 CPU::INSTR_RLA(u8 value)
{
  // ROL
  u8 carry = (u8)m_registers.GetFlagC();
  m_registers.SetFlagC(!!(value & 0x80));
  value = (value << 1) | carry;

  // AND
  m_registers.A &= value;
  m_registers.SetFlagZ(m_registers.A == 0);
  m_registers.SetFlagN((m_registers.A & 0x80) != 0);

  return value;
}

u8 CPU::INSTR_SRE(u8 value)
{
  // LSR
  m_registers.SetFlagC(!!(value & 0x01));
  value >>= 1;

  // AND
  m_registers.A ^= value;
  m_registers.SetFlagZ(m_registers.A == 0);
  m_registers.SetFlagN((m_registers.A & 0x80) != 0);

  return value;
}

u8 CPU::INSTR_RRA(u8 value)
{
  // ROR
  u8 carry = (u8)m_registers.GetFlagC();
  m_registers.SetFlagC(!!(value & 0x01));
  value = (carry << 7) | (value >> 1);

  // ADC
  u16 result = (u16)m_registers.A + value + (u16)m_registers.GetFlagC();
  m_registers.SetFlagV((~(m_registers.A ^ value) & (m_registers.A ^ result) & 0x80) != 0);
  m_registers.SetFlagC(result > 0xFF);
  m_registers.SetFlagZ((result & 0xFF) == 0);
  m_registers.SetFlagN((result & 0x80) != 0);
  m_registers.A = result & 0xFF;

  return value;
}

void CPU::ExecuteInstruction()
{
  // read opcode
  byte opcode = MemoryReadByte(m_registers.PC++);

  // decoding table
  switch (opcode)
  {
    case 0x69:
      WrapReadImmediate<&CPU::INSTR_ADC>();
      return; // ADC
    case 0x65:
      WrapReadZeroPage<&CPU::INSTR_ADC>();
      return; // ADC
    case 0x75:
      WrapReadZeroPageX<&CPU::INSTR_ADC>();
      return; // ADC
    case 0x6D:
      WrapReadAbsolute<&CPU::INSTR_ADC>();
      return; // ADC
    case 0x7D:
      WrapReadAbsoluteX<&CPU::INSTR_ADC>();
      return; // ADC
    case 0x79:
      WrapReadAbsoluteY<&CPU::INSTR_ADC>();
      return; // ADC
    case 0x61:
      WrapReadIndexedIndirect<&CPU::INSTR_ADC>();
      return; // ADC
    case 0x71:
      WrapReadIndirectIndexed<&CPU::INSTR_ADC>();
      return; // ADC
    case 0x29:
      WrapReadImmediate<&CPU::INSTR_AND>();
      return; // AND
    case 0x25:
      WrapReadZeroPage<&CPU::INSTR_AND>();
      return; // AND
    case 0x35:
      WrapReadZeroPageX<&CPU::INSTR_AND>();
      return; // AND
    case 0x2D:
      WrapReadAbsolute<&CPU::INSTR_AND>();
      return; // AND
    case 0x3D:
      WrapReadAbsoluteX<&CPU::INSTR_AND>();
      return; // AND
    case 0x39:
      WrapReadAbsoluteY<&CPU::INSTR_AND>();
      return; // AND
    case 0x21:
      WrapReadIndexedIndirect<&CPU::INSTR_AND>();
      return; // AND
    case 0x31:
      WrapReadIndirectIndexed<&CPU::INSTR_AND>();
      return; // AND
    case 0x0A:
      WrapModifyAccumulator<&CPU::INSTR_ASL>();
      return; // ASL
    case 0x06:
      WrapModifyZeroPage<&CPU::INSTR_ASL>();
      return; // ASL
    case 0x16:
      WrapModifyZeroPageX<&CPU::INSTR_ASL>();
      return; // ASL
    case 0x0E:
      WrapModifyAbsolute<&CPU::INSTR_ASL>();
      return; // ASL
    case 0x1E:
      WrapModifyAbsoluteX<&CPU::INSTR_ASL>();
      return; // ASL
    case 0x90:
      INSTR_BRx(!m_registers.GetFlagC(), (int8)ReadOperandByte());
      return; // BCC
    case 0xB0:
      INSTR_BRx(m_registers.GetFlagC(), (int8)ReadOperandByte());
      return; // BCS
    case 0xF0:
      INSTR_BRx(m_registers.GetFlagZ(), (int8)ReadOperandByte());
      return; // BEQ
    case 0x24:
      WrapReadZeroPage<&CPU::INSTR_BIT>();
      return; // BIT
    case 0x2C:
      WrapReadAbsolute<&CPU::INSTR_BIT>();
      return; // BIT
    case 0x30:
      INSTR_BRx(m_registers.GetFlagN(), (int8)ReadOperandByte());
      return; // BMI
    case 0xD0:
      INSTR_BRx(!m_registers.GetFlagZ(), (int8)ReadOperandByte());
      return; // BNE
    case 0x10:
      INSTR_BRx(!m_registers.GetFlagN(), (int8)ReadOperandByte());
      return; // BPL
    case 0x00:
      INSTR_BRK();
      return; // BRK
    case 0x50:
      INSTR_BRx(!m_registers.GetFlagV(), (int8)ReadOperandByte());
      return; // BVC
    case 0x70:
      INSTR_BRx(m_registers.GetFlagV(), (int8)ReadOperandByte());
      return; // BVS
    case 0x18:
      INSTR_CLx(FLAG_C);
      return; // CLC
    case 0xD8:
      INSTR_CLx(FLAG_D);
      return; // CLD
    case 0x58:
      INSTR_CLx(FLAG_I);
      return; // CLI
    case 0xB8:
      INSTR_CLx(FLAG_V);
      return; // CLV
    case 0xC9:
      WrapReadImmediate<&CPU::INSTR_CMP>();
      return; // CMP
    case 0xC5:
      WrapReadZeroPage<&CPU::INSTR_CMP>();
      return; // CMP
    case 0xD5:
      WrapReadZeroPageX<&CPU::INSTR_CMP>();
      return; // CMP
    case 0xCD:
      WrapReadAbsolute<&CPU::INSTR_CMP>();
      return; // CMP
    case 0xDD:
      WrapReadAbsoluteX<&CPU::INSTR_CMP>();
      return; // CMP
    case 0xD9:
      WrapReadAbsoluteY<&CPU::INSTR_CMP>();
      return; // CMP
    case 0xC1:
      WrapReadIndexedIndirect<&CPU::INSTR_CMP>();
      return; // CMP
    case 0xD1:
      WrapReadIndirectIndexed<&CPU::INSTR_CMP>();
      return; // CMP
    case 0xE0:
      WrapReadImmediate<&CPU::INSTR_CPX>();
      return; // CPX
    case 0xE4:
      WrapReadZeroPage<&CPU::INSTR_CPX>();
      return; // CPX
    case 0xEC:
      WrapReadAbsolute<&CPU::INSTR_CPX>();
      return; // CPX
    case 0xC0:
      WrapReadImmediate<&CPU::INSTR_CPY>();
      return; // CPY
    case 0xC4:
      WrapReadZeroPage<&CPU::INSTR_CPY>();
      return; // CPY
    case 0xCC:
      WrapReadAbsolute<&CPU::INSTR_CPY>();
      return; // CPY
    case 0xC6:
      WrapModifyZeroPage<&CPU::INSTR_DEC>();
      return; // DEC
    case 0xD6:
      WrapModifyZeroPageX<&CPU::INSTR_DEC>();
      return; // DEC
    case 0xCE:
      WrapModifyAbsolute<&CPU::INSTR_DEC>();
      return; // DEC
    case 0xDE:
      WrapModifyAbsoluteX<&CPU::INSTR_DEC>();
      return; // DEC
    case 0xCA:
      INSTR_DEX();
      return; // DEX
    case 0x88:
      INSTR_DEY();
      return; // DEY
    case 0x49:
      WrapReadImmediate<&CPU::INSTR_EOR>();
      return; // EOR
    case 0x45:
      WrapReadZeroPage<&CPU::INSTR_EOR>();
      return; // EOR
    case 0x55:
      WrapReadZeroPageX<&CPU::INSTR_EOR>();
      return; // EOR
    case 0x4D:
      WrapReadAbsolute<&CPU::INSTR_EOR>();
      return; // EOR
    case 0x5D:
      WrapReadAbsoluteX<&CPU::INSTR_EOR>();
      return; // EOR
    case 0x59:
      WrapReadAbsoluteY<&CPU::INSTR_EOR>();
      return; // EOR
    case 0x41:
      WrapReadIndexedIndirect<&CPU::INSTR_EOR>();
      return; // EOR
    case 0x51:
      WrapReadIndirectIndexed<&CPU::INSTR_EOR>();
      return; // EOR
    case 0xE6:
      WrapModifyZeroPage<&CPU::INSTR_INC>();
      return; // INC
    case 0xF6:
      WrapModifyZeroPageX<&CPU::INSTR_INC>();
      return; // INC
    case 0xEE:
      WrapModifyAbsolute<&CPU::INSTR_INC>();
      return; // INC
    case 0xFE:
      WrapModifyAbsoluteX<&CPU::INSTR_INC>();
      return; // INC
    case 0xE8:
      INSTR_INX();
      return; // INX
    case 0xC8:
      INSTR_INY();
      return; // INY
    case 0x4C:
      INSTR_JMP(ReadOperandWord());
      return; // JMP
    case 0x6C:
      INSTR_JMP(MemoryReadWordBug(ReadOperandWord()));
      return; // JMP
    case 0x20:
      INSTR_JSR(ReadOperandWord());
      return; // JSR
    case 0xA9:
      WrapReadImmediate<&CPU::INSTR_LDA>();
      return; // LDA
    case 0xA5:
      WrapReadZeroPage<&CPU::INSTR_LDA>();
      return; // LDA
    case 0xB5:
      WrapReadZeroPageX<&CPU::INSTR_LDA>();
      return; // LDA
    case 0xAD:
      WrapReadAbsolute<&CPU::INSTR_LDA>();
      return; // LDA
    case 0xBD:
      WrapReadAbsoluteX<&CPU::INSTR_LDA>();
      return; // LDA
    case 0xB9:
      WrapReadAbsoluteY<&CPU::INSTR_LDA>();
      return; // LDA
    case 0xA1:
      WrapReadIndexedIndirect<&CPU::INSTR_LDA>();
      return; // LDA
    case 0xB1:
      WrapReadIndirectIndexed<&CPU::INSTR_LDA>();
      return; // LDA
    case 0xA2:
      WrapReadImmediate<&CPU::INSTR_LDX>();
      return; // LDX
    case 0xA6:
      WrapReadZeroPage<&CPU::INSTR_LDX>();
      return; // LDX
    case 0xB6:
      WrapReadZeroPageY<&CPU::INSTR_LDX>();
      return; // LDX
    case 0xAE:
      WrapReadAbsolute<&CPU::INSTR_LDX>();
      return; // LDX
    case 0xBE:
      WrapReadAbsoluteY<&CPU::INSTR_LDX>();
      return; // LDX
    case 0xA0:
      WrapReadImmediate<&CPU::INSTR_LDY>();
      return; // LDY
    case 0xA4:
      WrapReadZeroPage<&CPU::INSTR_LDY>();
      return; // LDY
    case 0xB4:
      WrapReadZeroPageX<&CPU::INSTR_LDY>();
      return; // LDY
    case 0xAC:
      WrapReadAbsolute<&CPU::INSTR_LDY>();
      return; // LDY
    case 0xBC:
      WrapReadAbsoluteX<&CPU::INSTR_LDY>();
      return; // LDY
    case 0x4A:
      WrapModifyAccumulator<&CPU::INSTR_LSR>();
      return; // LSR
    case 0x46:
      WrapModifyZeroPage<&CPU::INSTR_LSR>();
      return; // LSR
    case 0x56:
      WrapModifyZeroPageX<&CPU::INSTR_LSR>();
      return; // LSR
    case 0x4E:
      WrapModifyAbsolute<&CPU::INSTR_LSR>();
      return; // LSR
    case 0x5E:
      WrapModifyAbsoluteX<&CPU::INSTR_LSR>();
      return; // LSR
    case 0xEA:
      INSTR_NOP();
      return; // NOP
    case 0x09:
      WrapReadImmediate<&CPU::INSTR_ORA>();
      return; // ORA
    case 0x05:
      WrapReadZeroPage<&CPU::INSTR_ORA>();
      return; // ORA
    case 0x15:
      WrapReadZeroPageX<&CPU::INSTR_ORA>();
      return; // ORA
    case 0x0D:
      WrapReadAbsolute<&CPU::INSTR_ORA>();
      return; // ORA
    case 0x1D:
      WrapReadAbsoluteX<&CPU::INSTR_ORA>();
      return; // ORA
    case 0x19:
      WrapReadAbsoluteY<&CPU::INSTR_ORA>();
      return; // ORA
    case 0x01:
      WrapReadIndexedIndirect<&CPU::INSTR_ORA>();
      return; // ORA
    case 0x11:
      WrapReadIndirectIndexed<&CPU::INSTR_ORA>();
      return; // ORA
    case 0x48:
      INSTR_PHA();
      return; // PHA
    case 0x08:
      INSTR_PHP();
      return; // PHP
    case 0x68:
      INSTR_PLA();
      return; // PLA
    case 0x28:
      INSTR_PLP();
      return; // PLP
    case 0x2A:
      WrapModifyAccumulator<&CPU::INSTR_ROL>();
      return; // ROL
    case 0x26:
      WrapModifyZeroPage<&CPU::INSTR_ROL>();
      return; // ROL
    case 0x36:
      WrapModifyZeroPageX<&CPU::INSTR_ROL>();
      return; // ROL
    case 0x2E:
      WrapModifyAbsolute<&CPU::INSTR_ROL>();
      return; // ROL
    case 0x3E:
      WrapModifyAbsoluteX<&CPU::INSTR_ROL>();
      return; // ROL
    case 0x6A:
      WrapModifyAccumulator<&CPU::INSTR_ROR>();
      return; // ROR
    case 0x66:
      WrapModifyZeroPage<&CPU::INSTR_ROR>();
      return; // ROR
    case 0x76:
      WrapModifyZeroPageX<&CPU::INSTR_ROR>();
      return; // ROR
    case 0x6E:
      WrapModifyAbsolute<&CPU::INSTR_ROR>();
      return; // ROR
    case 0x7E:
      WrapModifyAbsoluteX<&CPU::INSTR_ROR>();
      return; // ROR
    case 0x40:
      INSTR_RTI();
      return; // RTI
    case 0x60:
      INSTR_RTS();
      return; // RTS
    case 0xE9:
      WrapReadImmediate<&CPU::INSTR_SBC>();
      return; // SBC
    case 0xE5:
      WrapReadZeroPage<&CPU::INSTR_SBC>();
      return; // SBC
    case 0xF5:
      WrapReadZeroPageX<&CPU::INSTR_SBC>();
      return; // SBC
    case 0xED:
      WrapReadAbsolute<&CPU::INSTR_SBC>();
      return; // SBC
    case 0xFD:
      WrapReadAbsoluteX<&CPU::INSTR_SBC>();
      return; // SBC
    case 0xF9:
      WrapReadAbsoluteY<&CPU::INSTR_SBC>();
      return; // SBC
    case 0xE1:
      WrapReadIndexedIndirect<&CPU::INSTR_SBC>();
      return; // SBC
    case 0xF1:
      WrapReadIndirectIndexed<&CPU::INSTR_SBC>();
      return; // SBC
    case 0x38:
      INSTR_SEx(FLAG_C);
      return; // SEC
    case 0xF8:
      INSTR_SEx(FLAG_D);
      return; // SED
    case 0x78:
      INSTR_SEx(FLAG_I);
      return; // SEI
    case 0x85:
      WrapWriteZeroPage<&CPU::INSTR_STA>();
      return; // STA
    case 0x95:
      WrapWriteZeroPageX<&CPU::INSTR_STA>();
      return; // STA
    case 0x8D:
      WrapWriteAbsolute<&CPU::INSTR_STA>();
      return; // STA
    case 0x9D:
      WrapWriteAbsoluteX<&CPU::INSTR_STA>();
      return; // STA
    case 0x99:
      WrapWriteAbsoluteY<&CPU::INSTR_STA>();
      return; // STA
    case 0x81:
      WrapWriteIndexedIndirect<&CPU::INSTR_STA>();
      return; // STA
    case 0x91:
      WrapWriteIndirectIndexed<&CPU::INSTR_STA>();
      return; // STA
    case 0x86:
      WrapWriteZeroPage<&CPU::INSTR_STX>();
      return; // STX
    case 0x96:
      WrapWriteZeroPageY<&CPU::INSTR_STX>();
      return; // STX
    case 0x8E:
      WrapWriteAbsolute<&CPU::INSTR_STX>();
      return; // STX
    case 0x84:
      WrapWriteZeroPage<&CPU::INSTR_STY>();
      return; // STY
    case 0x94:
      WrapWriteZeroPageX<&CPU::INSTR_STY>();
      return; // STY
    case 0x8C:
      WrapWriteAbsolute<&CPU::INSTR_STY>();
      return; // STY
    case 0xAA:
      INSTR_TAX();
      return; // TAX
    case 0xA8:
      INSTR_TAY();
      return; // TAY
    case 0xBA:
      INSTR_TSX();
      return; // TSX
    case 0x8A:
      INSTR_TXA();
      return; // TXA
    case 0x9A:
      INSTR_TXS();
      return; // TXS
    case 0x98:
      INSTR_TYA();
      return; // TYA

      // Undocumented opcodes
    case 0x1A:
      INSTR_NOP();
      return; // NOP
    case 0x3A:
      INSTR_NOP();
      return; // NOP
    case 0x5A:
      INSTR_NOP();
      return; // NOP
    case 0x7A:
      INSTR_NOP();
      return; // NOP
    case 0xDA:
      INSTR_NOP();
      return; // NOP
    case 0xFA:
      INSTR_NOP();
      return; // NOP
    case 0x04:
      ReadOperandByte();
      INSTR_NOP();
      return; // NOP
    case 0x44:
      ReadOperandByte();
      INSTR_NOP();
      return; // NOP
    case 0x64:
      ReadOperandByte();
      INSTR_NOP();
      return; // NOP
    case 0x0C:
      ReadOperandWord();
      INSTR_NOP();
      return; // NOP
    case 0x14:
      ReadOperandByte();
      INSTR_NOP();
      return; // NOP
    case 0x34:
      ReadOperandByte();
      INSTR_NOP();
      return; // NOP
    case 0x54:
      ReadOperandByte();
      INSTR_NOP();
      return; // NOP
    case 0x74:
      ReadOperandByte();
      INSTR_NOP();
      return; // NOP
    case 0xD4:
      ReadOperandByte();
      INSTR_NOP();
      return; // NOP
    case 0xF4:
      ReadOperandByte();
      INSTR_NOP();
      return; // NOP
    case 0x80:
      ReadOperandByte();
      INSTR_NOP();
      return; // NOP
    case 0x1C:
      ReadOperandWord();
      INSTR_NOP();
      return; // NOP
    case 0x3C:
      ReadOperandWord();
      INSTR_NOP();
      return; // NOP
    case 0x5C:
      ReadOperandWord();
      INSTR_NOP();
      return; // NOP
    case 0x7C:
      ReadOperandWord();
      INSTR_NOP();
      return; // NOP
    case 0xDC:
      ReadOperandWord();
      INSTR_NOP();
      return; // NOP
    case 0xFC:
      ReadOperandWord();
      INSTR_NOP();
      return; // NOP

    case 0xA7:
      WrapReadZeroPage<&CPU::INSTR_LAX>();
      return; // LAX
    case 0xB7:
      WrapReadZeroPageY<&CPU::INSTR_LAX>();
      return; // LAX
    case 0xAF:
      WrapReadAbsolute<&CPU::INSTR_LAX>();
      return; // LAX
    case 0xBF:
      WrapReadAbsoluteY<&CPU::INSTR_LAX>();
      return; // LAX
    case 0xA3:
      WrapReadIndexedIndirect<&CPU::INSTR_LAX>();
      return; // LAX
    case 0xB3:
      WrapReadIndirectIndexed<&CPU::INSTR_LAX>();
      return; // LAX

    case 0x87:
      WrapWriteZeroPage<&CPU::INSTR_SAX>();
      return; // SAX
    case 0x97:
      WrapWriteZeroPageY<&CPU::INSTR_SAX>();
      return; // SAX
    case 0x83:
      WrapWriteIndexedIndirect<&CPU::INSTR_SAX>();
      return; // SAX
    case 0x8F:
      WrapWriteAbsolute<&CPU::INSTR_SAX>();
      return; // SAX

    case 0xEB:
      WrapReadImmediate<&CPU::INSTR_SBC>();
      return; // SBC

    case 0xCF: // DCP
      WrapModifyAbsolute<&CPU::INSTR_DCP>();
      return;

    case 0xDF: // DCP
      WrapModifyAbsoluteX<&CPU::INSTR_DCP>();
      return;

    case 0xDB: // DCP
      WrapModifyAbsoluteY<&CPU::INSTR_DCP>();
      return;

    case 0xC7: // DCP
      WrapModifyZeroPage<&CPU::INSTR_DCP>();
      return;

    case 0xD7: // DCP
      WrapModifyZeroPageX<&CPU::INSTR_DCP>();
      return;

    case 0xC3: // DCP
      WrapModifyIndexedIndirect<&CPU::INSTR_DCP>();
      return;

    case 0xD3: // DCP
      WrapModifyIndirectIndexed<&CPU::INSTR_DCP>();
      return;

    case 0xEF: // ISC
      WrapModifyAbsolute<&CPU::INSTR_ISB>();
      return;

    case 0xFF: // ISC
      WrapModifyAbsoluteX<&CPU::INSTR_ISB>();
      return;

    case 0xFB: // ISC
      WrapModifyAbsoluteY<&CPU::INSTR_ISB>();
      return;

    case 0xE7: // ISC
      WrapModifyZeroPage<&CPU::INSTR_ISB>();
      return;

    case 0xF7: // ISC
      WrapModifyZeroPageX<&CPU::INSTR_ISB>();
      return;

    case 0xE3: // ISC
      WrapModifyIndexedIndirect<&CPU::INSTR_ISB>();
      return;

    case 0xF3: // ISC
      WrapModifyIndirectIndexed<&CPU::INSTR_ISB>();
      return;

    case 0x0F: // SLO
      WrapModifyAbsolute<&CPU::INSTR_SLO>();
      return;

    case 0x1F: // SLO
      WrapModifyAbsoluteX<&CPU::INSTR_SLO>();
      return;

    case 0x1B: // SLO
      WrapModifyAbsoluteY<&CPU::INSTR_SLO>();
      return;

    case 0x07: // SLO
      WrapModifyZeroPage<&CPU::INSTR_SLO>();
      return;

    case 0x17: // SLO
      WrapModifyZeroPageX<&CPU::INSTR_SLO>();
      return;

    case 0x03: // SLO
      WrapModifyIndexedIndirect<&CPU::INSTR_SLO>();
      return;

    case 0x13: // SLO
      WrapModifyIndirectIndexed<&CPU::INSTR_SLO>();
      return;

    case 0x2F: // RLA
      WrapModifyAbsolute<&CPU::INSTR_RLA>();
      return;

    case 0x3F: // RLA
      WrapModifyAbsoluteX<&CPU::INSTR_RLA>();
      return;

    case 0x3B: // RLA
      WrapModifyAbsoluteY<&CPU::INSTR_RLA>();
      return;

    case 0x27: // RLA
      WrapModifyZeroPage<&CPU::INSTR_RLA>();
      return;

    case 0x37: // RLA
      WrapModifyZeroPageX<&CPU::INSTR_RLA>();
      return;

    case 0x23: // RLA
      WrapModifyIndexedIndirect<&CPU::INSTR_RLA>();
      return;

    case 0x33: // RLA
      WrapModifyIndirectIndexed<&CPU::INSTR_RLA>();
      return;

    case 0x4F: // SRE
      WrapModifyAbsolute<&CPU::INSTR_SRE>();
      return;

    case 0x5F: // SRE
      WrapModifyAbsoluteX<&CPU::INSTR_SRE>();
      return;

    case 0x5B: // SRE
      WrapModifyAbsoluteY<&CPU::INSTR_SRE>();
      return;

    case 0x47: // SRE
      WrapModifyZeroPage<&CPU::INSTR_SRE>();
      return;

    case 0x57: // SRE
      WrapModifyZeroPageX<&CPU::INSTR_SRE>();
      return;

    case 0x43: // SRE
      WrapModifyIndexedIndirect<&CPU::INSTR_SRE>();
      return;

    case 0x53: // SRE
      WrapModifyIndirectIndexed<&CPU::INSTR_SRE>();
      return;

    case 0x6F: // RRA
      WrapModifyAbsolute<&CPU::INSTR_RRA>();
      return;

    case 0x7F: // RRA
      WrapModifyAbsoluteX<&CPU::INSTR_RRA>();
      return;

    case 0x7B: // RRA
      WrapModifyAbsoluteY<&CPU::INSTR_RRA>();
      return;

    case 0x67: // RRA
      WrapModifyZeroPage<&CPU::INSTR_RRA>();
      return;

    case 0x77: // RRA
      WrapModifyZeroPageX<&CPU::INSTR_RRA>();
      return;

    case 0x63: // RRA
      WrapModifyIndexedIndirect<&CPU::INSTR_RRA>();
      return;

    case 0x73: // RRA
      WrapModifyIndirectIndexed<&CPU::INSTR_RRA>();
      return;
  }

  INSTR_unhandled();
}
