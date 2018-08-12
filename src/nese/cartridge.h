#pragma once
#include "types.h"
#include <memory>
#include <vector>

class Bus;
class ByteStream;
class Error;

class Cartridge
{
public:
  static const uint32 INES_PRG_ROM_BANK_SIZE = 0x4000; // 16KB
  static const uint32 INES_CHR_ROM_BANK_SIZE = 0x2000; // 8KB
  static const uint32 INES_PRG_RAM_BANK_SIZE = 0x2000; // 8KB

  using DataType = std::vector<byte>;

  enum MirrorMode
  {
    MirrorModeMirrorHorizontal,
    MirrorModeMirrorVertical,
    MirrorModeMirrorSingle0,
    MirrorModeMirrorSingle1,
    MirrorModeMirrorFour
  };

public:
  Cartridge();
  virtual ~Cartridge();

  const DataType& GetPRGROM() const { return m_prg_rom; }
  const DataType& GetCHRROM() const { return m_chr_rom; }
  const DataType& GetPRGRAM() const { return m_prg_ram; }
  const DataType& GetCHRRAM() const { return m_chr_ram; }
  const MirrorMode GetMirrorMode() const { return m_mirror; }

  // mapper
  virtual void Reset();
  virtual u8 ReadCPUAddress(Bus* bus, u16 address);
  virtual void WriteCPUAddress(Bus* bus, u16 address, u8 value);
  virtual u8 ReadPPUAddress(Bus* bus, u16 address);
  virtual void WritePPUAddress(Bus* bus, u16 address, u8 value);

  static std::unique_ptr<Cartridge> Load(ByteStream* stream, Error* error);

private:
  static std::unique_ptr<Cartridge> LoadINES(ByteStream* stream, Error* error);

protected:
  struct CartridgeData
  {
    DataType prg_rom;
    DataType chr_rom;
    u32 prg_ram_size;
    u32 chr_ram_size;
    u8 mapper_id;
    u8 mirror;
    bool battery;
  };

  virtual bool Initialize(CartridgeData& data, Error* error);

  static u16 MirrorAddress(MirrorMode mode, u16 offset);

  DataType m_prg_rom;
  DataType m_chr_rom;
  DataType m_prg_ram;
  DataType m_chr_ram;

  u8 m_prg_rom_bank_count = 0; // in 16KB banks
  u8 m_chr_rom_bank_count = 0; // in 8KB banks

  u8 m_mapper = 0;
  MirrorMode m_mirror = MirrorModeMirrorVertical;
  bool m_battery = false;
};
