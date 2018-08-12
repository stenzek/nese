#pragma once
#include "../cartridge.h"

namespace Mappers {
class GxROM final : public Cartridge
{
public:
  GxROM();
  ~GxROM() override;

  void Reset() override;

  u8 ReadCPUAddress(Bus* bus, u16 address) override;
  void WriteCPUAddress(Bus* bus, u16 address, u8 value) override;

  u8 ReadPPUAddress(Bus* bus, u16 address) override;

protected:
  bool Initialize(CartridgeData& data, Error* error) override;

private:
  static const u32 PRG_ROM_BANK_SIZE = 32768;
  static const u32 CHR_ROM_BANK_SIZE = 8192;

  void WriteBankSelect(u8 value);

  u32 m_prg_base_address = 0;
  u32 m_chr_base_address = 0;
};

} // namespace Mappers