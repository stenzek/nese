#pragma once
#include "../cartridge.h"

namespace Mappers {
class AxROM final : public Cartridge
{
public:
  AxROM();
  ~AxROM() override;

  void Reset() override;

  u8 ReadCPUAddress(Bus* bus, u16 address) override;
  void WriteCPUAddress(Bus* bus, u16 address, u8 value) override;

  u8 ReadPPUAddress(Bus* bus, u16 address) override;
  void WritePPUAddress(Bus* bus, u16 address, u8 value) override;

protected:
  bool Initialize(CartridgeData& data, Error* error) override;

private:
  static const u32 PRG_ROM_BANK_SIZE = 32768;
  static const u32 CHR_RAM_BANK_SIZE = 1024;
  static const u32 CHR_RAM_SIZE = 8192;

  u32 m_prg_base_address_8000 = 0;
  u32 m_nametable_select = 0;
};

} // namespace Mappers