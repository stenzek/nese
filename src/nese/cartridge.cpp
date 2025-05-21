#include "cartridge.h"
#include "bus.h"
#include "mappers/axrom.h"
#include "mappers/gxrom.h"
#include "mappers/mmc1.h"
#include "mappers/mmc3.h"
#include "mappers/nrom.h"
#include "mappers/uxrom.h"
#include "system.h"

#include "common/error.h"
#include "common/log.h"

#include "retro2/retro2_error.hpp"
#include "retro2/retro2_filesystem.hpp"

LOG_CHANNEL(Cartridge);

#pragma pack(push, 1)
struct INES_HEADER
{
  uint32 Magic;
  uint8 NumPRG;
  uint8 NumCHR;
  uint8 Control1;
  uint8 Control2;
  uint8 NumRam;
  uint8 Padding[7];
};
#pragma pack(pop)
static const uint32 INES_MAGIC = 0x1a53454e;

Cartridge::Cartridge() = default;

Cartridge::~Cartridge() = default;

bool Cartridge::Initialize(CartridgeData& data, Error* error)
{
  if (data.chr_rom.empty() && data.chr_ram_size == 0)
  {
    Error::SetStringView(error, "Cartridge data has no CHR-ROM and no CHR-RAM.");
    return false;
  }

  m_prg_rom = std::move(data.prg_rom);
  m_prg_rom_bank_count = u8(m_prg_rom.size() / INES_PRG_ROM_BANK_SIZE);
  m_chr_rom = std::move(data.chr_rom);
  m_chr_rom_bank_count = u8(m_chr_rom.size() / INES_CHR_ROM_BANK_SIZE);
  m_prg_ram.resize(data.prg_ram_size);
  m_chr_ram.resize(data.chr_ram_size);
  m_mapper = data.mapper_id;
  m_mirror = static_cast<MirrorMode>(data.mirror);
  m_battery = data.battery;
  return true;
}

void Cartridge::Reset()
{
}

uint8 Cartridge::ReadCPUAddress(Bus* bus, u16 address)
{
  return m_chr_rom[address & 0x3FFF];
}

void Cartridge::WriteCPUAddress(Bus* bus, u16 address, u8 value)
{
}

uint8 Cartridge::ReadPPUAddress(Bus* bus, u16 address)
{
  if (address < 0x2000)
  {
    return (m_chr_rom.empty()) ? m_chr_ram[address & 0x1FFF] : m_chr_rom[address & 0x1FFF];
  }
  else
  {
    u16 offset = MirrorAddress(m_mirror, address - 0x2000);
    return bus->ReadVRAM(offset & 0x7FF);
  }
}

void Cartridge::WritePPUAddress(Bus* bus, u16 address, u8 value)
{
  if (address < 0x2000)
  {
    // Ignore writes to CHR-ROM.
    if (m_chr_rom.empty())
      m_chr_ram[address & 0x1FFF] = value;

    return;
  }
  else
  {
    u16 offset = MirrorAddress(m_mirror, address - 0x2000);
    bus->WriteVRAM(offset & 0x7FF, value);
  }
}

void Cartridge::PPUScanline(Bus* bus, u32 line, bool rendering_enabled)
{
}

std::unique_ptr<Cartridge> Cartridge::Load(const char* path, Error* error)
{
  Retro2ErrorWrap r2error(R2Error);
  Retro2FileHandleWrap fh(R2FileSystem, R2FileSystem.OpenFile(path, "rb", &r2error));

  // read magic
  uint32 magic;
  if (!fh.Read(&magic, sizeof(magic), &r2error))
  {
    Error::SetStringFmt(error, "Failed to read magic: {}", r2error.GetMessage());
    return nullptr;
  }

  // check known magic
  if (magic == INES_MAGIC)
    return LoadINES(fh, error);

  // not known file type
  Error::SetStringFmt(error, "Unknown file type");
  return nullptr;
}

std::unique_ptr<Cartridge> Cartridge::LoadINES(Retro2FileHandleWrap& fh, Error* error)
{
  Retro2ErrorWrap r2error(R2Error);

  // return to start, read ines header
  INES_HEADER header;
  if (!fh.Seek(RETRO2_SEEK_MODE_ABSOLUTE, 0, &r2error) || !fh.Read(&header, sizeof(header), &r2error))
  {
    Error::SetStringFmt(error, "Failed to read header: {}", r2error.GetMessage());
    return {};
  }

  // find mapper
  CartridgeData data;
  data.mapper_id = (header.Control1 >> 4) | ((header.Control2 >> 4) << 4);
  data.mirror = (header.Control1 & 1);
  data.battery = (header.Control1 >> 1) & 1;

  // four-screen/vram
  if (header.Control1 & 0x08)
    data.mirror = MirrorModeMirrorFour;

  // trainer.. wtf?
  if (header.Control1 & 4)
  {
    if (!fh.Seek(RETRO2_SEEK_MODE_RELATIVE, 512, &r2error))
    {
      Error::SetStringFmt(error, "Failed to skip trainer: {}", r2error.GetMessage());
      return {};
    }
  }

  // if there is no CHR ROM, assume CHR RAM
  data.chr_ram_size = data.chr_rom.empty() ? 8192 : 0;

  // PRG RAM
  if (header.Control1 & 0x02)
  {
    if (header.NumRam == 0)
      data.prg_ram_size = INES_PRG_RAM_BANK_SIZE;
    else
      data.prg_ram_size = header.NumRam * INES_PRG_RAM_BANK_SIZE;
  }
  else
  {
    data.prg_ram_size = 0;
  }

  // read prg banks
  data.prg_rom.resize(INES_PRG_ROM_BANK_SIZE * header.NumPRG);
  for (uint32 i = 0; i < header.NumPRG; i++)
  {
    if (!fh.Read(&data.prg_rom[i * INES_PRG_ROM_BANK_SIZE], INES_PRG_ROM_BANK_SIZE, &r2error))
    {
      Error::SetStringFmt(error, "Failed to read PRG-ROM bank {}: {}", i, r2error.GetMessage());
      return {};
    }
  }

  // read chr banks
  data.chr_rom.resize(INES_CHR_ROM_BANK_SIZE * header.NumCHR);
  for (uint32 i = 0; i < header.NumCHR; i++)
  {
    if (!fh.Read(&data.chr_rom[i * INES_CHR_ROM_BANK_SIZE], INES_CHR_ROM_BANK_SIZE, &r2error))
    {
      Error::SetStringFmt(error, "Failed to read CHR-ROM bank {}: {}", i, r2error.GetMessage());
      return {};
    }
  }

  INFO_LOG("Parsing INES file:");
  INFO_LOG("  Mapper ID: {}", data.mapper_id);
  INFO_LOG("  Mirroring: {}", data.mirror);
  INFO_LOG("  Battery: {}", data.battery ? "yes" : "no");
  INFO_LOG("  CHR ROM: {} (0x{:x}) bytes, {} 8KB banks", data.chr_rom.size(), data.prg_rom.size(), header.NumCHR);
  INFO_LOG("  CHR RAM: {} (0x{:x}) bytes", data.chr_ram_size, data.chr_ram_size);
  INFO_LOG("  PRG ROM: {} (0x{:x}) bytes, {} 16K banks", data.prg_rom.size(), data.prg_rom.size(), header.NumPRG);
  INFO_LOG("  PRG RAM: {} (0x{:x}) bytes", data.prg_ram_size, data.prg_ram_size);

  // allocate cartridge
  std::unique_ptr<Cartridge> cart;
  switch (data.mapper_id)
  {
    case 0:
      cart = std::make_unique<Mappers::NROM>();
      break;

    case 1:
      cart = std::make_unique<Mappers::MMC1>();
      break;

    case 2:
      cart = std::make_unique<Mappers::UxROM>();
      break;

    case 4:
      cart = std::make_unique<Mappers::MMC3>();
      break;

    case 7:
      cart = std::make_unique<Mappers::AxROM>();
      break;

    case 66:
      cart = std::make_unique<Mappers::GxROM>();
      break;

    default:
      Error::SetStringFmt(error, "Unknown mapper {}", data.mapper_id);
      return nullptr;
  }

  if (!cart->Initialize(data, error))
    cart.reset();

  return cart;
}

uint16 Cartridge::MirrorAddress(MirrorMode mode, uint16 address)
{
  static constexpr uint32 lut[5][4] = {{0, 0, 1, 1}, {0, 1, 0, 1}, {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 1, 2, 3}};

  address = address % 0x1000;
  uint16 table = address / 0x0400;
  uint16 offset = address % 0x0400;
  return lut[mode][table] * 0x0400 + offset;
}
