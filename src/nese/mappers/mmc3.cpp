#include "mmc3.h"
#include "../bus.h"
#include "YBaseLib/Error.h"
#include "YBaseLib/Log.h"
Log_SetChannel(Mappers::MMC3);

namespace Mappers {
MMC3::MMC3() = default;

MMC3::~MMC3() = default;

bool MMC3::Initialize(CartridgeData& data, Error* error)
{
  // Always provide PRG RAM on MMC3.
  data.prg_ram_size = 8192;
  if (!Cartridge::Initialize(data, error))
    return false;

  if (!Common::IsAlignedPow2(m_prg_rom.size(), 8192))
  {
    error->SetErrorUserFormatted(1, "PRG-ROM (%u) must be aligned to %u bytes.", u32(m_prg_rom.size()), 8192);
    return false;
  }

  if (!Common::IsAlignedPow2(m_chr_rom.size(), 2048))
  {
    error->SetErrorUserFormatted(1, "CHR-ROM (%u) must be aligned to %u bytes.", u32(m_chr_rom.size()), 2048);
    return false;
  }

  UpdatePRGBankPointers();
  UpdateCHRBankPointers();
  return true;
}

void MMC3::Reset()
{
  m_bank_select_register = 0;
  m_last_chr_a12 = 0;
  m_irq_counter = 0;
  m_irq_reload_value = 0;
  m_irq_enable = false;
  std::fill_n(m_bank_numbers, countof(m_bank_numbers), u8(0));
  m_prg_ram_enable = false;
  UpdatePRGBankPointers();
  UpdateCHRBankPointers();
}

u8 MMC3::ReadCPUAddress(Bus* bus, u16 address)
{
  u32 page = (address >> 12);
  if (page < 0x08)
    return m_prg_ram_enable ? m_prg_ram[address & 0x1FFF] : 0x00;

  return m_prg_banks[(address >> 13) & 0x03][address & 0x1FFF];
}

void MMC3::WriteCPUAddress(Bus* bus, u16 address, u8 value)
{
  const bool odd = ConvertToBoolUnchecked(address & 0x01);
  switch ((address >> 12) & 0x0F)
  {
    case 0x06:
    case 0x07:
    {
      // PRG RAM at 0x6000-0x7FFF
      if (m_prg_ram_writable)
        m_prg_ram[address & 0x1FFF] = value;

      return;
    }

    case 0x08:
    case 0x09:
    {
      // Bank select(even)/Bank data(odd)
      if (!odd)
        WriteBankSelectRegister(value);
      else
        WriteBankDataRegister(value);

      return;
    }

    case 0x0A:
    case 0x0B:
    {
      // Mirroring(even)/PRG RAM protect(odd)
      if (!odd)
        WriteMirroringRegister(value);
      else
        WritePRGRAMProtectRegister(value);

      return;
    }

    case 0x0C:
    case 0x0D:
    {
      // IRQ latch(even)/reload(odd).
      if (!odd)
        WriteIRQReloadValue(bus, value);
      else
        WriteIRQReload(bus, value);

      return;
    }

    case 0x0E:
    case 0x0F:
    {
      // IRQ disable(even)/enable(odd).
      if (!odd)
        WriteIRQDisable(bus, value);
      else
        WriteIRQEnable(bus, value);

      return;
    }
  }
}

u8 MMC3::ReadPPUAddress(Bus* bus, u16 address)
{
  IRQClock(bus, address);
  if (address & 0x2000)
  {
    u16 offset = MirrorAddress(m_mirror, address & 0x1FFF);
    return bus->ReadVRAM(offset & 0x7FF);
  }
  else
  {
    // Log_DevPrintf("Read %04X -> %08X", address, u32(m_chr_banks[address >> 10] - m_chr_ram.data()) + (address &
    // 0x3FF));
    return m_chr_banks[address >> 10][address & 0x03FF];
  }
}

void MMC3::WritePPUAddress(Bus* bus, u16 address, u8 value)
{
  IRQClock(bus, address);
  if (address & 0x2000)
  {
    u16 offset = MirrorAddress(m_mirror, address & 0x1FFF);
    bus->WriteVRAM(offset & 0x7FF, value);
  }
  else
  {
    if (m_chr_rom.empty())
    {
      // Log_DevPrintf("Write %04X -> %08X %02X", address, u32(m_chr_banks[address >> 10] - m_chr_ram.data()) + (address
      // & 0x3FF), value);
      m_chr_banks[address >> 10][address & 0x03FF] = value;
    }
  }
}

void MMC3::UpdatePRGBankPointers()
{
  const u8 eightk_bank_count = m_prg_rom_bank_count * 2;
  const u8 last_bank = eightk_bank_count - 1;
  const u8 second_last_bank = (last_bank > 0) ? (last_bank - 1) : last_bank;

  u8 bank_8000, bank_A000, bank_C000, bank_E000;
  if (m_bank_select_register & 0x40)
  {
    bank_8000 = second_last_bank;
    bank_A000 = m_bank_numbers[7];
    bank_C000 = m_bank_numbers[6];
    bank_E000 = last_bank;
  }
  else
  {
    bank_8000 = m_bank_numbers[6];
    bank_A000 = m_bank_numbers[7];
    bank_C000 = second_last_bank;
    bank_E000 = last_bank;
  }

  const u32 offset_8000 = ((u32(bank_8000) << 13) % m_prg_rom.size());
  const u32 offset_A000 = ((u32(bank_A000) << 13) % m_prg_rom.size());
  const u32 offset_C000 = ((u32(bank_C000) << 13) % m_prg_rom.size());
  const u32 offset_E000 = ((u32(bank_E000) << 13) % m_prg_rom.size());
  m_prg_banks[0] = &m_prg_rom[offset_8000];
  m_prg_banks[1] = &m_prg_rom[offset_A000];
  m_prg_banks[2] = &m_prg_rom[offset_C000];
  m_prg_banks[3] = &m_prg_rom[offset_E000];

#if 0
  Log_DevPrintf("PRG 0x8000 -> Bank %u, %08X (of bank %u, %08X)", bank_8000, offset_8000, eightk_bank_count, u32(m_prg_rom.size()));
  Log_DevPrintf("PRG 0xA000 -> Bank %u, %08X (of bank %u, %08X)", bank_A000, offset_A000, eightk_bank_count, u32(m_prg_rom.size()));
  Log_DevPrintf("PRG 0xC000 -> Bank %u, %08X (of bank %u, %08X)", bank_C000, offset_C000, eightk_bank_count, u32(m_prg_rom.size()));
  Log_DevPrintf("PRG 0xE000 -> Bank %u, %08X (of bank %u, %08X)", bank_E000, offset_E000, eightk_bank_count, u32(m_prg_rom.size()));
#endif
}

void MMC3::UpdateCHRBankPointers()
{
  // This could be simplified, since the hardware inverts bit A12.
  u8 bank_0000, bank_0400, bank_0800, bank_0C00;
  u8 bank_1000, bank_1400, bank_1800, bank_1C00;
  if (m_bank_select_register & 0x80)
  {
    bank_0000 = m_bank_numbers[2];
    bank_0400 = m_bank_numbers[3];
    bank_0800 = m_bank_numbers[4];
    bank_0C00 = m_bank_numbers[5];
    bank_1000 = m_bank_numbers[0] & 0xFE;
    bank_1400 = m_bank_numbers[0] | 0x01;
    bank_1800 = m_bank_numbers[1] & 0xFE;
    bank_1C00 = m_bank_numbers[1] | 0x01;
  }
  else
  {
    bank_0000 = m_bank_numbers[0] & 0xFE;
    bank_0400 = m_bank_numbers[0] | 0x01;
    bank_0800 = m_bank_numbers[1] & 0xFE;
    bank_0C00 = m_bank_numbers[1] | 0x01;
    bank_1000 = m_bank_numbers[2];
    bank_1400 = m_bank_numbers[3];
    bank_1800 = m_bank_numbers[4];
    bank_1C00 = m_bank_numbers[5];
  }

  const u32 size = static_cast<u32>(m_chr_rom.empty() ? m_chr_ram.size() : m_chr_rom.size());
  byte* base_ptr = m_chr_rom.empty() ? m_chr_ram.data() : m_chr_rom.data();

  const u32 offset_0000 = ((u32(bank_0000) << 10) % size);
  const u32 offset_0400 = ((u32(bank_0400) << 10) % size);
  const u32 offset_0800 = ((u32(bank_0800) << 10) % size);
  const u32 offset_0C00 = ((u32(bank_0C00) << 10) % size);
  const u32 offset_1000 = ((u32(bank_1000) << 10) % size);
  const u32 offset_1400 = ((u32(bank_1400) << 10) % size);
  const u32 offset_1800 = ((u32(bank_1800) << 10) % size);
  const u32 offset_1C00 = ((u32(bank_1C00) << 10) % size);
  m_chr_banks[0] = base_ptr + offset_0000;
  m_chr_banks[1] = base_ptr + offset_0400;
  m_chr_banks[2] = base_ptr + offset_0800;
  m_chr_banks[3] = base_ptr + offset_0C00;
  m_chr_banks[4] = base_ptr + offset_1000;
  m_chr_banks[5] = base_ptr + offset_1400;
  m_chr_banks[6] = base_ptr + offset_1800;
  m_chr_banks[7] = base_ptr + offset_1C00;

#if 0
  Log_DevPrintf("CHR 0x0000 -> Bank %u, %08X (of bank %u, %08X)", bank_0000, offset_0000, size / 1024, size);
  Log_DevPrintf("CHR 0x0400 -> Bank %u, %08X (of bank %u, %08X)", bank_0400, offset_0400, size / 1024, size);
  Log_DevPrintf("CHR 0x0800 -> Bank %u, %08X (of bank %u, %08X)", bank_0800, offset_0800, size / 1024, size);
  Log_DevPrintf("CHR 0x0C00 -> Bank %u, %08X (of bank %u, %08X)", bank_0C00, offset_0C00, size / 1024, size);
  Log_DevPrintf("CHR 0x1000 -> Bank %u, %08X (of bank %u, %08X)", bank_1000, offset_1000, size / 1024, size);
  Log_DevPrintf("CHR 0x1400 -> Bank %u, %08X (of bank %u, %08X)", bank_1400, offset_1400, size / 1024, size);
  Log_DevPrintf("CHR 0x1800 -> Bank %u, %08X (of bank %u, %08X)", bank_1800, offset_1800, size / 1024, size);
  Log_DevPrintf("CHR 0x1C00 -> Bank %u, %08X (of bank %u, %08X)", bank_1C00, offset_1C00, size / 1024, size);
#endif
}

void MMC3::WriteBankSelectRegister(u8 value)
{
  const u8 old_bank_select_register = m_bank_select_register;
  if (old_bank_select_register == value)
    return;

  m_bank_select_register = value;

  // This can be skipped if the upper bits don't change.
  if ((old_bank_select_register & 0xC0) != (value & 0xC0))
  {
    UpdatePRGBankPointers();
    UpdateCHRBankPointers();
  }
}

void MMC3::WriteBankDataRegister(u8 value)
{
  const u8 bank_num = (m_bank_select_register & 0x07);
  if (bank_num < 2)
  {
    // Writes to registers 0 and 1 always ignore bit 0, loading the value AND #$FE into the first half and the value OR
    // #$01 into the second half of the 2 KiB bank.
    value &= 0xFE;
  }
  else if (bank_num >= 6)
  {
    // Writes to registers 6 and 7 always ignore bits 6 and 7, as the MMC3 has only 6 PRG ROM address output lines.
    value &= 0x3F;
  }

  m_bank_numbers[bank_num] = value;
  if (bank_num >= 6)
    UpdatePRGBankPointers();
  else
    UpdateCHRBankPointers();
}

void MMC3::WriteMirroringRegister(u8 value)
{
  m_mirror = (value & 0x01) ? MirrorModeMirrorHorizontal : MirrorModeMirrorVertical;
}

void MMC3::WritePRGRAMProtectRegister(u8 value)
{
  const bool enable = ConvertToBoolUnchecked((value >> 7) & 0x01);
  const bool read_only = ConvertToBoolUnchecked((value >> 6) & 0x01);
  m_prg_ram_enable = enable;
  m_prg_ram_writable = enable && !read_only;
}

void MMC3::WriteIRQReloadValue(Bus* bus, u8 value)
{
  m_irq_reload_value = value;
}

void MMC3::WriteIRQReload(Bus* bus, u8 value)
{
  m_irq_counter = 0;
}

void MMC3::WriteIRQDisable(Bus* bus, u8 value)
{
  m_irq_enable = false;
  bus->SetCPUIRQLine(false);
}

void MMC3::WriteIRQEnable(Bus* bus, u8 value)
{
  m_irq_enable = true;
}

void MMC3::IRQClock(Bus* bus, u16 address)
{
#if 0
  const u8 a12 = u8((address >> 12) & 0x01);
  if (m_last_chr_a12 != a12)
  {
    // Rising edge of A12.
    if (!m_last_chr_a12)
    {
      if (m_irq_counter == 0)
      {
        m_irq_counter = m_irq_reload_value;
      }
      else
      {
        m_irq_counter--;
        if (m_irq_counter == 0 && m_irq_enable)
          bus->SetCPUIRQLine(true);
      }
    }

    m_last_chr_a12 = a12;
  }
#endif
}

void MMC3::PPUScanline(Bus* bus, u32 line, bool rendering_enabled)
{
  // Only visible lines.
  if ((line >= 240 && line < 261) || !rendering_enabled)
    return;

  // We should be using A12-based detection, but that's broken for now.
  if (m_irq_counter == 0)
  {
    m_irq_counter = m_irq_reload_value;
  }
  else
  {
    m_irq_counter--;
    if (m_irq_counter == 0 && m_irq_enable)
      bus->SetCPUIRQLine(true);
  }
}

} // namespace Mappers