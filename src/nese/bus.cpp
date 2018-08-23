#include "bus.h"
#include "apu.h"
#include "cartridge.h"
#include "controller.h"
#include "cpu.h"
#include "ppu.h"
#include <cstring>

Bus::Bus() = default;

Bus::~Bus() = default;

void Bus::Initialize(CPU* cpu, PPU* ppu, APU* apu)
{
  m_cpu = cpu;
  m_ppu = ppu;
  m_apu = apu;
}

void Bus::Reset()
{
  // TODO: All-zeros or all-ones?
  std::memset(m_wram, 0x00, sizeof(m_wram));
  std::memset(m_vram, 0x00, sizeof(m_vram));
  m_cpu->SetNMILine(false);
  m_cpu->SetIRQLine(false);
}

void Bus::ExecutePendingCycles()
{
  if (m_pending_cycles == 0)
    return;

  // 3 PPU cycles per CPU cycle.
  m_ppu->Execute(m_pending_cycles * 3);
  m_apu->Execute(m_pending_cycles);
  m_pending_cycles = 0;
}

void Bus::EndScanline()
{
  // TODO: This will eventually link up to Cartridge.
}

u8 Bus::ReadCPUAddress(u16 address)
{
  switch (address >> 12)
  {
    case 0x0: // 0x0000
    case 0x1: // 0x1000
    {
      // main memory/WRAM
      return m_wram[address & 0x7FF];
    }

    case 0x2: // 0x2000
    case 0x3: // 0x3000
    {
      // ppu registers
      ExecutePendingCycles();
      return m_ppu->ReadRegister(address & 0x7);
    }

    case 0x4: // 0x4000
    {
      // TODO: Open bus behavior.
      switch (address & 0xFF)
      {
        case 0x00: // $4000 - SQ1_VOL
        case 0x01: // $4001 - SQ1_SWEEP
        case 0x02: // $4002 - SQ1_LO
        case 0x03: // $4003 - SQ1_HI
        case 0x04: // $4004 - SQ2_VOL
        case 0x05: // $4005 - SQ2_SWEEP
        case 0x06: // $4006 - SQ1_LO
        case 0x07: // $4007 - SQ1_HI
        case 0x08: // $4008 - TRI_LINEAR
        case 0x0A: // $400A - TRI_LO
        case 0x0B: // $400B - TRI_HI
        case 0x0C: // $400C - NOISE_VOL
        case 0x0E: // $400E - NOISE_LO
        case 0x0F: // $400F - NOISE_HI
        case 0x10: // $4010 - DMC_FREQ
        case 0x11: // $4011 - DMC_RAW
        case 0x12: // $4012 - DMC_START
        case 0x13: // $4013 - DMC_LEN
        case 0x15: // $4015 - SND_CHN
        {
          ExecutePendingCycles();
          return m_apu->ReadRegister(address & 0xFF);
        }

        case 0x16: // $4016 - JOY1
        case 0x17: // $4017 - JOY2
        {
          // Controller ports.
          // TODO: Open bus bits.
          Controller* controller = m_controllers[(address & 0xFF) - 0x16];
          if (controller)
            return controller->ReadData();
          else
            return 0;
        }

        default:
        {
          // Unused.
          return 0;
        }
      }
    }

    default:
    {
      // Redirect rest to cartridge.
      return m_cartridge->ReadCPUAddress(this, address);
    }
  }
}

void Bus::WriteCPUAddress(u16 address, u8 value)
{
  switch (address >> 12)
  {
    case 0x0: // 0x0000
    case 0x1: // 0x1000
    {
      // main memory/WRAM
      m_wram[address & 0x7FF] = value;
      return;
    }

    case 0x2: // 0x2000
    case 0x3: // 0x3000
    {
      // ppu registers
      ExecutePendingCycles();
      m_ppu->WriteRegister(address & 0x7, value);
      return;
    }

    case 0x4: // 0x4000
    {
      // TODO: Open bus behavior.
      switch (address & 0xFF)
      {
        case 0x00: // $4000 - SQ1_VOL
        case 0x01: // $4001 - SQ1_SWEEP
        case 0x02: // $4002 - SQ1_LO
        case 0x03: // $4003 - SQ1_HI
        case 0x04: // $4004 - SQ2_VOL
        case 0x05: // $4005 - SQ2_SWEEP
        case 0x06: // $4006 - SQ1_LO
        case 0x07: // $4007 - SQ1_HI
        case 0x08: // $4008 - TRI_LINEAR
        case 0x0A: // $400A - TRI_LO
        case 0x0B: // $400B - TRI_HI
        case 0x0C: // $400C - NOISE_VOL
        case 0x0E: // $400E - NOISE_LO
        case 0x0F: // $400F - NOISE_HI
        case 0x10: // $4010 - DMC_FREQ
        case 0x11: // $4011 - DMC_RAW
        case 0x12: // $4012 - DMC_START
        case 0x13: // $4013 - DMC_LEN
        case 0x15: // $4013 - SND_CHN
        case 0x17: // $4017 - Frame counter control
        {
          ExecutePendingCycles();
          m_apu->WriteRegister(address & 0xFF, value);
          return;
        }

        case 0x14: // $4014 - OAMDMA
        {
          ExecutePendingCycles();
          m_ppu->WriteDMA(value);
          return;
        }

        case 0x16: // $4016 - Joystick Strobe
        {
          const bool active = (value != 0);
          for (Controller* controller : m_controllers)
          {
            if (controller)
              controller->WriteStrobe(active);
          }

          return;
        }

          // Unused.
        case 0x09:
        case 0x0D:
        default:
          return;
      }
    }

    default:
    {
      // Redirect rest to cartridge.
      return m_cartridge->WriteCPUAddress(this, address, value);
    }
  }
}

u8 Bus::ReadPPUAddress(u16 address)
{
  // All PPU accesses go to the cartridge.
  return m_cartridge->ReadPPUAddress(this, address);
}

void Bus::WritePPUAddress(u16 address, u8 value)
{
  m_cartridge->WritePPUAddress(this, address, value);
}

void Bus::StallCPU(u32 num_cycles)
{
  // Extra tick on odd cycles
  uint32 stall_cycles = num_cycles + (m_cpu->GetCyclesSinceReset() & 1);
  m_cpu->Stall(stall_cycles);
}

void Bus::SetCPUNMILine(bool active)
{
  m_cpu->SetNMILine(active);
}

void Bus::SetCPUIRQLine(bool active)
{
  m_cpu->SetIRQLine(active);
}

void Bus::PPUScanline(u32 line, bool rendering_enabled)
{
  m_cartridge->PPUScanline(this, line, rendering_enabled);
}
