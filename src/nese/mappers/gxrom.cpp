#include "gxrom.h"
#include "../bus.h"
#include "YBaseLib/Error.h"

namespace Mappers {
GxROM::GxROM() = default;

GxROM::~GxROM() = default;

bool GxROM::Initialize(CartridgeData& data, Error* error)
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

void GxROM::Reset()
{
  m_prg_base_address = 0;
  m_chr_base_address = 0;
}

u8 GxROM::ReadCPUAddress(Bus* bus, u16 address)
{
  return m_prg_rom[m_prg_base_address | (address & 0x7FFF)];
}

void GxROM::WriteCPUAddress(Bus* bus, u16 address, u8 value)
{
  if (address < 0x8000)
    return;

  WriteBankSelect(value);
}

u8 GxROM::ReadPPUAddress(Bus* bus, u16 address)
{
  // Default behavior for nametables.
  if (address & 0x2000)
    return Cartridge::ReadPPUAddress(bus, address);

  const u32 offset = m_chr_base_address | (address & 0x1FFF);
  return m_chr_rom.empty() ? m_chr_ram[offset] : m_chr_rom[offset];
}

void GxROM::WriteBankSelect(u8 value)
{
  m_prg_base_address = (((value >> 4) & 0x03) << 15) % m_prg_rom.size();
  m_chr_base_address = ((value & 0x03) << 13) % (m_chr_rom.empty() ? m_chr_ram.size() : m_chr_rom.size());
}

} // namespace Mappers