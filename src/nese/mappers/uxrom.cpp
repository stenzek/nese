#include "uxrom.h"
#include "../bus.h"

#include "common/align.h"
#include "common/error.h"

namespace Mappers {
UxROM::UxROM() = default;

UxROM::~UxROM() = default;

bool UxROM::Initialize(CartridgeData& data, Error* error)
{
  if (!Cartridge::Initialize(data, error))
    return false;

  if (!Common::IsAlignedPow2(m_prg_rom.size(), PRG_ROM_BANK_SIZE))
  {
    Error::SetStringFmt(error, "PRG-ROM ({}) must be aligned to {} bytes.", m_prg_rom.size(), PRG_ROM_BANK_SIZE);
    return false;
  }

  m_prg_base_address_C000 = ((m_prg_rom_bank_count - 1) << 14);
  return true;
}

void UxROM::Reset()
{
  // TODO: Is this correct?
  m_prg_base_address_8000 = 0;
}

u8 UxROM::ReadCPUAddress(Bus* bus, u16 address)
{
  if ((address & 0xC000) == 0xC000)
    return m_prg_rom[m_prg_base_address_C000 | (address & 0x3FFF)];
  else
    return m_prg_rom[m_prg_base_address_8000 | (address & 0x3FFF)];
}

void UxROM::WriteCPUAddress(Bus* bus, u16 address, u8 value)
{
  if (address < 0x8000)
    return;

  // 8000-FFFF - Bank Select.
  m_prg_base_address_8000 = ((value & 0x0F) << 14) % m_prg_rom.size();
}
} // namespace Mappers