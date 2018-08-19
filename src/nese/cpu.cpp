#include "nese/cpu.h"
#include "YBaseLib/Assert.h"
#include "YBaseLib/Log.h"
#include "YBaseLib/Memory.h"
#include "YBaseLib/String.h"
#include "nese/bus.h"
#include "nese/system.h"
Log_SetChannel(CPU);

CPU::CPU() = default;

CPU::~CPU() {}

void CPU::Initialize(System* system, Bus* bus)
{
  m_system = system;
  m_bus = bus;
}

void CPU::Reset()
{
  m_cycle_counter = 0;
  m_stall_cycles = 0;

  m_nmi_pending = false;

  // starting address, read it from $FFFC
  m_registers.A = 0x00;
  m_registers.X = 0x00;
  m_registers.Y = 0x00;
  m_registers.P = 0x24; // should be 0x34, but to match log 0x24, see wiki.nesdev.com
  m_registers.S = 0xFD;
  m_registers.PC = u16(m_bus->ReadCPUAddress(0xFFFC)) | (u16(m_bus->ReadCPUAddress(0xFFFD)) << 8);
  /// m_registers.PC = 0xC000;
}

static bool disasm_enabled = false;

void CPU::Execute(CycleCount cycles)
{
  CycleCount executed_cycles = 0;
  while (cycles > 0)
  {
    const u32 old_cycle_counter = m_cycle_counter;

    // Stall > NMI > IRQ > Normal Execution
    if (m_stall_cycles > 0)
    {
      const u32 stall_cycle_count = std::min(m_stall_cycles, u32(cycles));
      m_stall_cycles -= stall_cycle_count;
      AddCycles(stall_cycle_count);
    }
    else if (m_nmi_pending)
    {
      HandleNMI();
    }
    else if (m_irq_line_state && !m_registers.GetFlagI())
    {
      HandleIRQ();
    }
    else
    {
      // debug
      if (disasm_enabled)
      {
        SmallString disasm;
        if (Disassemble(&disasm, m_registers.PC, nullptr))
        {
          // Log_DevPrintf("exec: %s", disasm.GetCharArray());
          // Log_DevPrintf("%-48sA:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%3u SL:0", disasm.GetCharArray(),
          // m_registers.A, m_registers.X, m_registers.Y, m_registers.P, m_registers.S, 0, 0);
          std::fprintf(stdout, "%-48sA:%02X X:%02X Y:%02X P:%02X SP:%02X\n", disasm.GetCharArray(), m_registers.A,
                       m_registers.X, m_registers.Y, m_registers.P, m_registers.S);
        }
        else
        {
          std::fprintf(stdout, "disasm fail at %04X\n", m_registers.PC);
        }
      }

      ExecuteInstruction();
    }

    // Handle u32 overflow.
    s32 consumed_cycles = (m_cycle_counter >= old_cycle_counter) ?
                            (m_cycle_counter - old_cycle_counter) :
                            (0xFFFFFFFFu - old_cycle_counter + 1u + m_cycle_counter);
    executed_cycles += consumed_cycles;
    cycles -= consumed_cycles;
  }
}

void CPU::PushByte(uint8 value)
{
  MemoryWriteByte(STACK_BASE | m_registers.S, value);
  m_registers.S--;
}

void CPU::PushWord(u16 value)
{
  MemoryWriteByte(STACK_BASE | m_registers.S, (value >> 8) & 0xFF);
  m_registers.S--;
  MemoryWriteByte(STACK_BASE | m_registers.S, value & 0xFF);
  m_registers.S--;
}

uint8 CPU::PopByte()
{
  m_registers.S++;
  uint8 value = MemoryReadByte(STACK_BASE | m_registers.S);
  return value;
}

u16 CPU::PopWord()
{
  m_registers.S++;
  u16 value = (u16)MemoryReadByte(STACK_BASE | m_registers.S);
  m_registers.S++;
  value |= (u16)(MemoryReadByte(STACK_BASE | m_registers.S) << 8);
  return value;
}

uint8 CPU::MemoryReadByte(u16 address)
{
  // memory reads take one cycle
  uint8 value = m_bus->ReadCPUAddress(address);
  AddCycles(1);
  return value;
}

void CPU::MemoryWriteByte(u16 address, uint8 value)
{
  // memory writes take one cycle, with the write occurring afterwards
  AddCycles(1);
  m_bus->WriteCPUAddress(address, value);
}

u16 CPU::MemoryReadWord(u16 address)
{
  // read in little-endian order - correct to hw??
  uint8 low = MemoryReadByte(address);
  uint8 high = MemoryReadByte(address + 1);
  return ((u16)high << 8) | (u16)low;
}

u16 CPU::MemoryReadWordBug(u16 address)
{
  // 6502 bug where the low byte is incremented without incrementing the high byte
  u16 high_address = (address & 0xFF00) | ((address + 1) & 0xFF);
  uint8 low = MemoryReadByte(address);
  uint8 high = MemoryReadByte(high_address);
  return ((u16)high << 8) | (u16)low;
}

void CPU::MemoryWriteWord(u16 address, u16 value)
{
  // write in little-endian order
  MemoryWriteByte(address, static_cast<uint8>(value & 0xFF));
  MemoryWriteByte(address + 1, static_cast<uint8>((value >> 8) & 0xFF));
}

void CPU::AddCycles(u32 cycles)
{
  m_cycle_counter += cycles;
  m_bus->AddPendingCycles(cycles);
}

void CPU::SetNMILine(bool state)
{
  if (m_nmi_line_state == state)
    return;

  // NMI is edge sensitive.
  m_nmi_line_state = state;
  m_nmi_pending |= state;
}

void CPU::SetIRQLine(bool state)
{
  // if (m_irq_line_state != state)
  // Log_DevPrintf("CPU IRQ %s", state ? "on" : "off");

  // IRQ is level sensitive.
  m_irq_line_state = state;
}

void CPU::Stall(u32 cycles)
{
  m_stall_cycles += cycles;
}

void CPU::HandleNMI()
{
  // Log_DevPrintf("NMI");
  m_nmi_pending = false;
  PushWord(m_registers.PC);
  PushByte(m_registers.P);
  m_registers.PC = MemoryReadWord(0xFFFA);
  m_registers.SetFlagI(true);
  AddCycles(7);
}

void CPU::HandleIRQ()
{
  // Log_DevPrintf("IRQ");
  PushWord(m_registers.PC);
  PushByte(m_registers.P);
  m_registers.PC = MemoryReadWord(0xFFFE);
  m_registers.SetFlagI(true);
  AddCycles(7);
}
