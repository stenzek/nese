#include "mmc1.h"
#include "../bus.h"
#include "YBaseLib/Error.h"

namespace Mappers {
MMC1::MMC1() = default;

MMC1::~MMC1() = default;

bool MMC1::Initialize(CartridgeData& data, Error* error)
{
  if (!Cartridge::Initialize(data, error))
    return false;

  if (!Common::IsAlignedPow2(m_prg_rom.size(), PRG_ROM_BANK_SIZE))
  {
    error->SetErrorUserFormatted(1, "PRG-ROM (%u) must be aligned to %u bytes.", u32(m_prg_rom.size()),
                                 PRG_ROM_BANK_SIZE);
    return false;
  }

  if (!Common::IsAlignedPow2(m_chr_rom.size(), CHR_ROM_BANK_SIZE))
  {
    error->SetErrorUserFormatted(1, "CHR-ROM (%u) must be aligned to %u bytes.", u32(m_chr_rom.size()),
                                 CHR_ROM_BANK_SIZE);
    return false;
  }

  return true;
}

void MMC1::Reset()
{
  WriteRegister(0x00, 0x0C);
  WriteRegister(0x01, 0x00);
  WriteRegister(0x02, 0x00);
  WriteRegister(0x03, 0x00);
  m_shift_register_value = 0;
  m_shift_register_count = 0;
  m_prg_ram_enable = false;
}

u8 MMC1::ReadCPUAddress(Bus* bus, u16 address)
{
  switch (address >> 12)
  {
    case 0x6:
    case 0x7:
      return m_prg_ram_enable ? m_prg_ram[address & 0x1FFF] : 0;

    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB:
      return m_prg_rom[m_base_prg_address_8000 | u32(address & 0x3FFF)];

    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
      return m_prg_rom[m_base_prg_address_C000 | u32(address & 0x3FFF)];

    default:
      return 0;
  }
}

void MMC1::WriteCPUAddress(Bus* bus, u16 address, u8 value)
{
  const u32 address_bank = address >> 12;
  if (address < 0x8000)
  {
    // PRG RAM at 0x6000-0x7FFF
    if (address >= 0x6000 && m_prg_ram_enable)
      m_prg_ram[address & 0x1FFF] = value;

    return;
  }

  // Writing a value with bit 7 set resets the shift register.
  if (value & 0x80)
  {
    WriteRegister(0x00, m_regs[0] | 0x0C);
    m_shift_register_value = 0;
    m_shift_register_count = 0;
    return;
  }

  m_shift_register_value |= (value & 0x01) << m_shift_register_count;
  m_shift_register_count++;

  if (m_shift_register_count == 5)
  {
    // Address only matters for the 5th write.
    WriteRegister(u8(address >> 13) & 0x03, m_shift_register_value);

    // Automatically reset afterwards.
    m_shift_register_value = 0;
    m_shift_register_count = 0;
  }
}

u8 MMC1::ReadPPUAddress(Bus* bus, u16 address)
{
  if (address & 0x2000)
  {
    u16 offset = MirrorAddress(m_mirror, address & 0x1FFF);
    return bus->ReadVRAM(offset & 0x7FF);
  }
  else
  {
    const u32 offset = ((address & 0x1000) ? m_base_chr_address_1000 : m_base_chr_address_0000) | (address & 0xFFF);
    return (m_chr_rom.empty()) ? m_chr_ram[offset] : m_chr_rom[offset];
  }
}

void MMC1::WritePPUAddress(Bus* bus, u16 address, u8 value)
{
  if (address & 0x2000)
  {
    u16 offset = MirrorAddress(m_mirror, address & 0x1FFF);
    bus->WriteVRAM(offset & 0x7FF, value);
  }
  else
  {
    const u32 offset = ((address & 0x1000) ? m_base_chr_address_1000 : m_base_chr_address_0000) | (address & 0xFFF);
    if (m_chr_rom.empty())
      m_chr_ram[offset] = value;
  }
}

void MMC1::WriteRegister(u8 reg, u8 value)
{
  // const u8 old_value = m_regs[reg];
  m_regs[reg] = value;

  switch (reg)
  {
    case 0x00:
    {
      UpdatePRGBaseAddresses();
      UpdateCHRBaseAddresses();

      // Update mirroring mode.
      switch (value & 0x03)
      {
        case 0x00:
          m_mirror = MirrorModeMirrorSingle0;
          break;
        case 0x01:
          m_mirror = MirrorModeMirrorSingle1;
          break;
        case 0x02:
          m_mirror = MirrorModeMirrorVertical;
          break;
        case 0x03:
          m_mirror = MirrorModeMirrorHorizontal;
          break;
      }
    }
    break;

    case 0x01:
    case 0x02:
    {
      UpdateCHRBaseAddresses();
    }
    break;

    case 0x03:
    {
      UpdatePRGBaseAddresses();

      // Bit 4 determines whether PRG RAM is enabled.
      m_prg_ram_enable = !m_prg_ram.empty() && ((value & 0x10) == 0);
    }
    break;
  }
}

void MMC1::UpdatePRGBaseAddresses()
{
  u8 bank = m_regs[3] & 0x0F;
  u8 bank_0;
  u8 bank_1;

  switch ((m_regs[0] >> 2) & 0x03)
  {
    case 0x00: // 0, 1: switch 32 KB at $8000, ignoring low bit of bank number
    case 0x01:
    {
      bank_0 = bank & 0x0E;
      bank_1 = bank_0 + 1;
    }
    break;

    case 0x02: // 2: fix first bank at $8000 and switch 16 KB bank at $C000;
    {
      bank_0 = 0x00;
      bank_1 = bank;
    }
    break;

    case 0x03: // 3: fix last bank at $C000 and switch 16 KB bank at $8000)
    {
      bank_0 = bank;
      bank_1 = m_prg_rom_bank_count - 1;
    }
    break;
  }

  m_base_prg_address_8000 = (u32(bank_0) << 14) % m_prg_rom.size();
  m_base_prg_address_C000 = (u32(bank_1) << 14) % m_prg_rom.size();

#if 0
  Log_DevPrintf("PRG 0x8000 -> Bank %u, %08X (of bank %u, %08X)", bank_0, m_base_prg_address_8000, m_prg_rom_bank_count,
                u32(m_prg_rom.size()));
  Log_DevPrintf("PRG 0xC000 -> Bank %u, %08X (of bank %u, %08X)", bank_1, m_base_prg_address_C000, m_prg_rom_bank_count,
                u32(m_prg_rom.size()));
#endif
}

void MMC1::UpdateCHRBaseAddresses()
{
  u8 bank_0, bank_1;
  if (m_regs[0] & 0x10)
  {
    // Switching 4KB CHR banks.
    bank_0 = m_regs[0x01];
    bank_1 = m_regs[0x02];
  }
  else
  {
    // Switching 8KB CHR banks.
    bank_0 = m_regs[0x01] & 0x1E;
    bank_1 = bank_0 + 1;
  }

  const u32 chr_rom_size = m_chr_rom.empty() ? m_chr_ram.size() : m_chr_rom.size();
  m_base_chr_address_0000 = (u32(bank_0) << 12) % chr_rom_size;
  m_base_chr_address_1000 = (u32(bank_1) << 12) % chr_rom_size;

#if 0
  Log_DevPrintf("CHR 0x0000 -> Bank %u, %08X (of bank %u, %08X)", bank_0, m_base_chr_address_0000, m_chr_rom_bank_count,
                chr_rom_size);
  Log_DevPrintf("CHR 0x0000 -> Bank %u, %08X (of bank %u, %08X)", bank_1, m_base_chr_address_1000, m_chr_rom_bank_count,
                chr_rom_size);
#endif
}

} // namespace Mappers