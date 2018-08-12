#pragma once
#include "../cartridge.h"

namespace Mappers {

class MMC1 final : public Cartridge
{
public:
  MMC1();
  ~MMC1() override final;

  void Reset() override final;

  u8 ReadCPUAddress(Bus* bus, u16 address) override final;
  void WriteCPUAddress(Bus* bus, u16 address, u8 value) override final;

  u8 ReadPPUAddress(Bus* bus, u16 address) override final;
  void WritePPUAddress(Bus* bus, u16 address, u8 value) override final;

protected:
  virtual bool Initialize(CartridgeData& data, Error* error) override final;

private:
  static const u32 PRG_ROM_BANK_SIZE = 16384;
  static const u32 CHR_ROM_BANK_SIZE = 4096;

  void WriteRegister(u8 reg, u8 value);
  void UpdatePRGBaseAddresses();
  void UpdateCHRBaseAddresses();

  u8 m_shift_register_value = 0;
  u8 m_shift_register_count = 0;

  u8 m_regs[4] = {};

  bool m_prg_ram_enable = 0;

  // Cached address bases.
  u32 m_base_prg_address_8000 = 0;
  u32 m_base_prg_address_C000 = 0;
  u32 m_base_chr_address_0000 = 0;
  u32 m_base_chr_address_1000 = 0;
};
} // namespace Mappers