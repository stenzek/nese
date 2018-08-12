#include "YBaseLib/String.h"
#include "nese/bus.h"
#include "nese/cpu.h"

struct DisassemblyTableEntry
{
  const char* instruction_name;
  CPU::AddressingMode addressing_mode;
};

static const DisassemblyTableEntry opcodes[256] = {
#define INSTRUCTION_IMP(name) {#name, CPU::AddressingMode::Implicit},
#define INSTRUCTION_ACC(name) {#name, CPU::AddressingMode::Accumulator},
#define INSTRUCTION_IMM(name) {#name, CPU::AddressingMode::Immediate},
#define INSTRUCTION_ZP(name) {#name, CPU::AddressingMode::ZeroPage},
#define INSTRUCTION_ZPX(name) {#name, CPU::AddressingMode::ZeroPageX},
#define INSTRUCTION_ZPY(name) {#name, CPU::AddressingMode::ZeroPageY},
#define INSTRUCTION_REL(name) {#name, CPU::AddressingMode::Relative},
#define INSTRUCTION_ABS(name) {#name, CPU::AddressingMode::Absolute},
#define INSTRUCTION_ABX(name) {#name, CPU::AddressingMode::AbsoluteX},
#define INSTRUCTION_ABY(name) {#name, CPU::AddressingMode::AbsoluteY},
#define INSTRUCTION_DIR(name) {#name, CPU::AddressingMode::Direct},
#define INSTRUCTION_IND(name) {#name, CPU::AddressingMode::Indirect},
#define INSTRUCTION_ABS(name) {#name, CPU::AddressingMode::Absolute},
#define INSTRUCTION_IZX(name) {#name, CPU::AddressingMode::IndexedIndirect},
#define INSTRUCTION_IZY(name) {#name, CPU::AddressingMode::IndirectIndexed},
#include "nese/cpu_instruction_list.h"
#undef INSTRUCTION_IMP
#undef INSTRUCTION_ACC
#undef INSTRUCTION_IMM
#undef INSTRUCTION_ZP
#undef INSTRUCTION_ZPX
#undef INSTRUCTION_ZPY
#undef INSTRUCTION_REL
#undef INSTRUCTION_ABS
#undef INSTRUCTION_ABX
#undef INSTRUCTION_ABY
#undef INSTRUCTION_IND
#undef INSTRUCTION_ABS
#undef INSTRUCTION_IZX
#undef INSTRUCTION_IZY
};

bool CPU::Disassemble(String* pString, u16 address, u16* size)
{
  const u8 opcode = m_bus->ReadCPUAddress(address);
  const DisassemblyTableEntry* entry = &opcodes[opcode];
  u8 operand_1 = 0;
  u8 operand_2 = 0;
  u16 length = 1;

  pString->Clear();
  pString->AppendFormattedString("%04X  ", address);

  switch (entry->addressing_mode)
  {
    case CPU::AddressingMode::Immediate:
    case CPU::AddressingMode::ZeroPage:
    case CPU::AddressingMode::ZeroPageX:
    case CPU::AddressingMode::ZeroPageY:
    case CPU::AddressingMode::IndexedIndirect:
    case CPU::AddressingMode::IndirectIndexed:
    case CPU::AddressingMode::Relative:
      length = 2;
      operand_1 = m_bus->ReadCPUAddress(address + 1);
      pString->AppendFormattedString("%02X %02X     ", opcode, operand_1);
      break;

    case CPU::AddressingMode::Absolute:
    case CPU::AddressingMode::AbsoluteX:
    case CPU::AddressingMode::AbsoluteY:
    case CPU::AddressingMode::Direct:
    case CPU::AddressingMode::Indirect:
      length = 3;
      operand_1 = m_bus->ReadCPUAddress(address + 1);
      operand_2 = m_bus->ReadCPUAddress(address + 2);
      pString->AppendFormattedString("%02X %02X %02X  ", opcode, operand_1, operand_2);
      break;

    default:
      pString->AppendFormattedString("%02X        ", opcode);
      break;
  }

  pString->AppendString(entry->instruction_name);

#if 0
  u16 pointer_address, temp_address;
  switch (entry->addressing_mode)
  {
    case CPU::AddressingMode::Immediate:
      pString->AppendFormattedString(" #$%02X", operand_1);
      break;

    case CPU::AddressingMode::ZeroPage:
      pString->AppendFormattedString(" $%02X = %02X", operand_1, m_bus->ReadCPUAddress(u16(operand_1)));
      break;

    case CPU::AddressingMode::ZeroPageX:
      temp_address = (u16(operand_1) + m_registers.X) & 0xFF;
      pString->AppendFormattedString(" $%02X,X @ %02X = %02X", operand_1, temp_address,
                                     m_bus->ReadCPUAddress(temp_address));
      break;

    case CPU::AddressingMode::ZeroPageY:
      temp_address = (u16(operand_1) + m_registers.Y) & 0xFF;
      pString->AppendFormattedString(" $%02X,Y @ %02X = %02X", operand_1, temp_address,
                                     m_bus->ReadCPUAddress(temp_address));
      break;

    case CPU::AddressingMode::IndexedIndirect:
      pointer_address = (operand_1 + m_registers.X) & 0xFF;
      temp_address = u16(m_bus->ReadCPUAddress(pointer_address)) |
                     ((u16(m_bus->ReadCPUAddress((pointer_address & 0xFF00) | ((pointer_address + 1) & 0xFF)))) << 8);
      pString->AppendFormattedString(" ($%02X,X) @ %02X = %04X = %02X", operand_1, pointer_address, temp_address,
                                     m_bus->ReadCPUAddress(temp_address));
      break;

    case CPU::AddressingMode::IndirectIndexed:
      pointer_address =
        u16(m_bus->ReadCPUAddress(operand_1)) | (u16(m_bus->ReadCPUAddress((u16(operand_1) + 1) & 0xFF)) << 8);
      temp_address = pointer_address + m_registers.Y;
      pString->AppendFormattedString(" ($%02X),Y = %04X @ %04X = %02X", operand_1, pointer_address, temp_address,
                                     MemoryReadByte(temp_address));
      break;

    case CPU::AddressingMode::Relative:
      pString->AppendFormattedString(" $%04X", (address + length + u16(int16(int8(operand_1)))) & 0xFFFF);
      break;

    case CPU::AddressingMode::Absolute:
      temp_address = u16(operand_1) | (u16(operand_2) << 8);
      pString->AppendFormattedString(" $%04X = %02X", temp_address, m_bus->ReadCPUAddress(temp_address));
      break;

    case CPU::AddressingMode::AbsoluteX:
      pointer_address = (u16(operand_1) | (u16(operand_2) << 8));
      temp_address = pointer_address + m_registers.X;
      pString->AppendFormattedString(" $%04X,X @ %04X = %02X", pointer_address, temp_address,
                                     m_bus->ReadCPUAddress(temp_address));
      break;

    case CPU::AddressingMode::AbsoluteY:
      pointer_address = (u16(operand_1) | (u16(operand_2) << 8));
      temp_address = pointer_address + m_registers.Y;
      pString->AppendFormattedString(" $%04X,Y @ %04X = %02X", pointer_address, temp_address,
                                     m_bus->ReadCPUAddress(temp_address));
      break;

    case CPU::AddressingMode::Direct:
      pString->AppendFormattedString(" $%04X", u16(operand_1) | (u16(operand_2) << 8));
      break;

    case CPU::AddressingMode::Indirect:
      pointer_address = (u16(operand_1) | (u16(operand_2) << 8));
      temp_address =
        u16(m_bus->ReadCPUAddress(pointer_address)) | (u16(m_bus->ReadCPUAddress(pointer_address + 1)) << 8);
      pString->AppendFormattedString(" ($%04X) = %04X", pointer_address, temp_address);
      break;

    case CPU::AddressingMode::Accumulator:
      pString->AppendFormattedString(" A");
      break;

    case CPU::AddressingMode::Implicit:
      break;

    default:
      break;
  }
#else
  u16 pointer_address, temp_address;
  switch (entry->addressing_mode)
  {
    case CPU::AddressingMode::Immediate:
      pString->AppendFormattedString(" #$%02X", operand_1);
      break;

    case CPU::AddressingMode::ZeroPage:
      pString->AppendFormattedString(" $%02X = %02X", operand_1, m_bus->ReadCPUAddress(u16(operand_1)));
      break;

    case CPU::AddressingMode::ZeroPageX:
      temp_address = (u16(operand_1) + m_registers.X) & 0xFF;
      pString->AppendFormattedString(" $%02X,X @ %02X", operand_1, temp_address);
      break;

    case CPU::AddressingMode::ZeroPageY:
      temp_address = (u16(operand_1) + m_registers.Y) & 0xFF;
      pString->AppendFormattedString(" $%02X,Y @ %02X", operand_1, temp_address);
      break;

    case CPU::AddressingMode::IndexedIndirect:
      pointer_address = (operand_1 + m_registers.X) & 0xFF;
      temp_address = u16(m_bus->ReadCPUAddress(pointer_address)) |
                     ((u16(m_bus->ReadCPUAddress((pointer_address & 0xFF00) | ((pointer_address + 1) & 0xFF)))) << 8);
      pString->AppendFormattedString(" ($%02X,X) @ %02X = %04X", operand_1, pointer_address, temp_address);
      break;

    case CPU::AddressingMode::IndirectIndexed:
      pointer_address =
        u16(m_bus->ReadCPUAddress(operand_1)) | (u16(m_bus->ReadCPUAddress((u16(operand_1) + 1) & 0xFF)) << 8);
      temp_address = pointer_address + m_registers.Y;
      pString->AppendFormattedString(" ($%02X),Y = %04X @ %04X", operand_1, pointer_address, temp_address);
      break;

    case CPU::AddressingMode::Relative:
      pString->AppendFormattedString(" $%04X", (address + length + u16(int16(int8(operand_1)))) & 0xFFFF);
      break;

    case CPU::AddressingMode::Absolute:
      temp_address = u16(operand_1) | (u16(operand_2) << 8);
      pString->AppendFormattedString(" $%04X", temp_address);
      break;

    case CPU::AddressingMode::AbsoluteX:
      pointer_address = (u16(operand_1) | (u16(operand_2) << 8));
      temp_address = pointer_address + m_registers.X;
      pString->AppendFormattedString(" $%04X,X @ %04X", pointer_address, temp_address);
      break;

    case CPU::AddressingMode::AbsoluteY:
      pointer_address = (u16(operand_1) | (u16(operand_2) << 8));
      temp_address = pointer_address + m_registers.Y;
      pString->AppendFormattedString(" $%04X,Y @ %04X", pointer_address, temp_address);
      break;

    case CPU::AddressingMode::Direct:
      pString->AppendFormattedString(" $%04X", u16(operand_1) | (u16(operand_2) << 8));
      break;

    case CPU::AddressingMode::Indirect:
      pointer_address = (u16(operand_1) | (u16(operand_2) << 8));
      temp_address =
        u16(m_bus->ReadCPUAddress(pointer_address)) | (u16(m_bus->ReadCPUAddress(pointer_address + 1)) << 8);
      pString->AppendFormattedString(" ($%04X) = %04X", pointer_address, temp_address);
      break;

    case CPU::AddressingMode::Accumulator:
      pString->AppendFormattedString(" A");
      break;

    case CPU::AddressingMode::Implicit:
      break;

    default:
      break;
  }
#endif

  if (size != nullptr)
    *size = length;

  return true;
}
