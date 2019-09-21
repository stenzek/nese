#include "axrom.h"
#include "../bus.h"
#include "YBaseLib/Error.h"

namespace Mappers {
AxROM::AxROM() = default;

AxROM::~AxROM() = default;

bool AxROM::Initialize(CartridgeData& data, Error* error)
{
  if (!Cartridge::Initialize(data, error))
    return false;

  if (!Common::IsAlignedPow2(m_prg_rom.size(), PRG_ROM_BANK_SIZE))
  {
    error->SetErrorUserFormatted(1, "PRG-ROM (%u) must be aligned to %u bytes.", u32(m_prg_rom.size()),
                                 PRG_ROM_BANK_SIZE);
    return false;
  }

  if (m_chr_ram.empty())
  {
    error->SetErrorUserFormatted(1, "CHR-RAM must be present.");
    return false;
  }

  m_prg_base_address_8000 = PRG_ROM_BANK_SIZE;
  return true;
}

void AxROM::Reset()
{
  // TODO: Is this correct?
  m_prg_base_address_8000 = PRG_ROM_BANK_SIZE;
}

u8 AxROM::ReadCPUAddress(Bus* bus, u16 address)
{
  if ((address & 0x8000) == 0x8000)
    return m_prg_rom[m_prg_base_address_8000 | (address & 0x7FFF)];
  else
    return m_prg_rom[(address & 0x7FFF)];
}

void AxROM::WriteCPUAddress(Bus* bus, u16 address, u8 value)
{
  if (address < 0x8000)
    return;

  // 8000-FFFF - Bank Select.
  m_prg_base_address_8000 = (ZeroExtend32(value & 0x0F) << 15) % static_cast<u32>(m_prg_rom.size());
  m_nametable_select = ZeroExtend32(value >> 4) & u32(0x01);
}

u8 AxROM::ReadPPUAddress(Bus* bus, u16 address)
{
  if (address < 0x2000)
    return m_chr_ram[address & 0x1FFF];

  const u16 offset = (m_nametable_select << 10) | (address & 0x3FF);
  return bus->ReadVRAM(offset);
}

void AxROM::WritePPUAddress(Bus* bus, u16 address, u8 value)
{
  if (address < 0x2000)
  {
    m_chr_ram[address & 0x1FFF] = value;
    return;
  }

  const u16 offset = (m_nametable_select << 10) | (address & 0x3FF);
  bus->WriteVRAM(offset, value);
}

} // namespace Mappers