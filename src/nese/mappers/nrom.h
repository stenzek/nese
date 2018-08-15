#pragma once
#include "../cartridge.h"

namespace Mappers {
class NROM final : public Cartridge
{
public:
  NROM();
  ~NROM() override;

  void Reset() override;

  u8 ReadCPUAddress(Bus* bus, u16 address) override;
  void WriteCPUAddress(Bus* bus, u16 address, u8 value) override;

protected:
  bool Initialize(CartridgeData& data, Error* error) override;

private:
  static const u32 PRG_ROM_BANK_SIZE = 16384;

  u32 m_prg_base_address_C000 = 0;
};

} // namespace Mappers