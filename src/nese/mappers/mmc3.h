#pragma once
#include "../cartridge.h"

namespace Mappers {

class MMC3 final : public Cartridge
{
public:
  MMC3();
  ~MMC3() override final;

  void Reset() override final;

  u8 ReadCPUAddress(Bus* bus, u16 address) override final;
  void WriteCPUAddress(Bus* bus, u16 address, u8 value) override final;

  u8 ReadPPUAddress(Bus* bus, u16 address) override final;
  void WritePPUAddress(Bus* bus, u16 address, u8 value) override final;

protected:
  virtual bool Initialize(CartridgeData& data, Error* error) override final;

private:
  static const u8 NUM_PRG_BANKS = 4;
  static const u8 NUM_CHR_BANKS = 8;

  void UpdatePRGBankPointers();
  void UpdateCHRBankPointers();
  void IRQClock(Bus* bus, u16 address);

  void WriteBankSelectRegister(u8 value);
  void WriteBankDataRegister(u8 value);
  void WriteMirroringRegister(u8 value);
  void WritePRGRAMProtectRegister(u8 value);
  void WriteIRQReloadValue(Bus* bus, u8 value);
  void WriteIRQReload(Bus* bus, u8 value);
  void WriteIRQDisable(Bus* bus, u8 value);
  void WriteIRQEnable(Bus* bus, u8 value);

  // Cached address bases.
  const byte* m_prg_banks[NUM_PRG_BANKS] = {};
  byte* m_chr_banks[NUM_CHR_BANKS] = {};

  u8 m_bank_select_register = 0;
  u8 m_bank_numbers[8] = {};
  bool m_prg_ram_enable = false;
  bool m_prg_ram_writable = false;

  u8 m_last_chr_a12 = 0;
  u8 m_irq_reload_value = 0;
  u8 m_irq_counter = 0;
  bool m_irq_enable = false;
};
} // namespace Mappers