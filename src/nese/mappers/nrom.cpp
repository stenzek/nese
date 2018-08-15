#include "nrom.h"
#include "../bus.h"
#include "YBaseLib/Error.h"

namespace Mappers {
NROM::NROM() = default;

NROM::~NROM() = default;

bool NROM::Initialize(CartridgeData& data, Error* error)
{
  if (!Cartridge::Initialize(data, error))
    return false;

  if (!Common::IsAlignedPow2(m_prg_rom.size(), PRG_ROM_BANK_SIZE))
  {
    error->SetErrorUserFormatted(1, "PRG-ROM (%u) must be aligned to %u bytes.", u32(m_prg_rom.size()),
                                 PRG_ROM_BANK_SIZE);
    return false;
  }

  // Supports one or two banks.
  if (m_prg_rom_bank_count > 1)
    m_prg_base_address_C000 = ((m_prg_rom_bank_count - 1) << 14);
  else
    m_prg_base_address_C000 = 0;

  return true;
}

void NROM::Reset() {}

u8 NROM::ReadCPUAddress(Bus* bus, u16 address)
{
  if ((address & 0xC000) == 0xC000)
    return m_prg_rom[m_prg_base_address_C000 | (address & 0x3FFF)];
  else
    return m_prg_rom[address & 0x3FFF];
}

void NROM::WriteCPUAddress(Bus* bus, u16 address, u8 value) {}
} // namespace Mappers
