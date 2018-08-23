#include "ppu.h"
#include "YBaseLib/Assert.h"
#include "YBaseLib/Error.h"
#include "YBaseLib/Log.h"
#include "YBaseLib/Memory.h"
#include "bus.h"
#include "common/display.h"
#include "cpu.h"
#include "system.h"
Log_SetChannel(PPU);

#if 0
static const uint32 PALETTE[] = {
  0x666666, 0x002A88, 0x1412A7, 0x3B00A4, 0x5C007E, 0x6E0040, 0x6C0600, 0x561D00, 0x333500, 0x0B4800, 0x005200,
  0x004F08, 0x00404D, 0x000000, 0x000000, 0x000000, 0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE, 0xA01ACC, 0xB71E7B,
  0xB53120, 0x994E00, 0x6B6D00, 0x388700, 0x0C9300, 0x008F32, 0x007C8D, 0x000000, 0x000000, 0x000000, 0xFFFEFF,
  0x64B0FF, 0x9290FF, 0xC676FF, 0xF36AFF, 0xFE6ECC, 0xFE8170, 0xEA9E22, 0xBCBE00, 0x88D800, 0x5CE430, 0x45E082,
  0x48CDDE, 0x4F4F4F, 0x000000, 0x000000, 0xFFFEFF, 0xC0DFFF, 0xD3D2FF, 0xE8C8FF, 0xFBC2FF, 0xFEC4EA, 0xFECCC5,
  0xF7D8A5, 0xE4E594, 0xCFEF96, 0xBDF4AB, 0xB3F3CC, 0xB5EBF2, 0xB8B8B8, 0x000000, 0x000000};
#else
static const uint32 PALETTE[] = {
  0x666666, 0x882A00, 0xA71214, 0xA4003B, 0x7E005C, 0x40006E, 0x00066C, 0x001D56, 0x003533, 0x00480B, 0x005200,
  0x084F00, 0x4D4000, 0x000000, 0x000000, 0x000000, 0xADADAD, 0xD95F15, 0xFF4042, 0xFE2775, 0xCC1AA0, 0x7B1EB7,
  0x2031B5, 0x004E99, 0x006D6B, 0x008738, 0x00930C, 0x328F00, 0x8D7C00, 0x000000, 0x000000, 0x000000, 0xFFFEFF,
  0xFFB064, 0xFF9092, 0xFF76C6, 0xFF6AF3, 0xCC6EFE, 0x7081FE, 0x229EEA, 0x00BEBC, 0x00D888, 0x30E45C, 0x82E045,
  0xDECD48, 0x4F4F4F, 0x000000, 0x000000, 0xFFFEFF, 0xFFDFC0, 0xFFD2D3, 0xFFC8E8, 0xFFC2FB, 0xEAC4FE, 0xC5CCFE,
  0xA5D8F7, 0x94E5E4, 0x96EFCF, 0xABF4BD, 0xCCF3B3, 0xF2EBB5, 0xB8B8B8, 0x000000, 0x000000};
#endif

PPU::PPU() = default;

PPU::~PPU() = default;

void PPU::Initialize(System* system, Bus* bus, Display* display)
{
  m_system = system;
  m_bus = bus;
  m_display = display;

  m_display->ResizeFramebuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
}

void PPU::Reset()
{
  m_current_cycle = 340;
  m_current_scanline = 240;

  Y_memzero(m_palette_ram, sizeof(m_palette_ram));
  std::memset(m_oam_ram, 0xFF, sizeof(m_oam_ram));
  Y_memzero(&m_regs, sizeof(m_regs));
  m_f = 0;
  m_register = 0;
  m_nmi_enable = false;
  m_nmi_hold = false;
  m_nmi_flag = false;
  m_vram_increment = 0;
  m_sprite_table_address = 0;
  m_background_table_address = 0;
  m_flagMasterSlave = 0;
  m_grayscale_flag = 0;
  m_flagShowLeftBackground = 0;
  m_flagShowLeftSprites = 0;
  m_flagShowBackground = 0;
  m_flagShowSprites = 0;
  m_flagRedTint = 0;
  m_flagGreenTint = 0;
  m_flagBlueTint = 0;
  m_flagSpriteZeroHit = 0;
  m_flagSpriteOverflow = 0;
  m_oamAddress = 0;
  m_ppu_bus_value = 0;

  WriteControl(0);
  WriteMask(0);
  WriteOAMAddress(0);
}

u8 PPU::ReadRegister(u8 address)
{
  switch (address)
  {
    case 0x0002: // 0x2002
      return ReadStatus();

    case 0x0004: // 0x2004
      return ReadOAMData();

    case 0x0007: // 0x2007
      return ReadData();

    default:
      return 0;
  }
}

void PPU::WriteRegister(u8 address, u8 value)
{
  switch (address)
  {
    case 0x0000: // 0x2000
      WriteControl(value);
      break;
    case 0x0001: // 0x2002
      WriteMask(value);
      break;
    case 0x0003: // 0x2003
      WriteOAMAddress(value);
      break;
    case 0x0004: // 0x2004
      WriteOAMData(value);
      break;
    case 0x0005: // 0x2005
      WriteScroll(value);
      break;
    case 0x0006: // 0x2006
      WriteAddress(value);
      break;
    case 0x0007: // 0x2007
      WriteData(value);
      break;

    default:
      break;
  }
}

u8 PPU::Read(u16 address)
{
  address &= 0x3FFF;
  if (address >= 0x3F00)
  {
    // CGRAM/Palette.
    u8 palette_index = (address & 0x1F);

    // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C.
    if ((palette_index & 0x13) == 0x10)
      palette_index &= 0x0F;

    if (m_grayscale_flag)
      return m_palette_ram[palette_index] & 0x30;
    else
      return m_palette_ram[palette_index];
  }

  return m_bus->ReadPPUAddress(address);
}

void PPU::Write(u16 address, u8 value)
{
  address &= 0x3FFF;
  if (address >= 0x3F00)
  {
    u8 palette_index = (address & 0x1F);
    if ((palette_index & 0x13) == 0x10)
      palette_index &= 0x0F;

    m_palette_ram[palette_index] = value;
    return;
  }

  m_bus->WritePPUAddress(address, value);
}

u8 PPU::ReadPalette(u16 address)
{
  if (address >= 16 && (address % 4) == 0)
    address -= 16;

  return m_palette_ram[address];
}

void PPU::WritePalette(u16 address, u8 value)
{
  if (address >= 16 && (address % 4) == 0)
    address -= 16;

  m_palette_ram[address] = value;
}

void PPU::WriteControl(u8 value)
{
  m_regs.t.nametable = (value >> 0) & 3;
  m_vram_increment = ((value >> 2) & 1) ? 32 : 1;
  m_sprite_table_address = ((value >> 3) & 1) * 0x1000;
  m_background_table_address = ((value >> 4) & 1) * 0x1000;
  m_sprite_height = ((value >> 5) & 1) ? 16 : 8;
  m_flagMasterSlave = (value >> 6) & 1;
  m_nmi_enable = ((value >> 7) & 1) == 1;
}

void PPU::WriteMask(u8 value)
{
  m_grayscale_flag = (value >> 0) & 1;
  m_flagShowLeftBackground = (value >> 1) & 1;
  m_flagShowLeftSprites = (value >> 2) & 1;
  m_flagShowBackground = (value >> 3) & 1;
  m_flagShowSprites = (value >> 4) & 1;
  m_flagRedTint = (value >> 5) & 1;
  m_flagGreenTint = (value >> 6) & 1;
  m_flagBlueTint = (value >> 7) & 1;
}

u8 PPU::ReadStatus()
{
  u8 result = m_register & 0x1F;
  result |= (m_flagSpriteOverflow << 5);
  result |= (m_flagSpriteZeroHit << 6);
  result |= (u8(m_nmi_flag) << 7);

  m_regs.address_latch = 0;
  m_nmi_flag = 0;
  UpdateNMILine();

  return result;
}

void PPU::WriteOAMAddress(u8 value)
{
  m_oamAddress = value;
}

u8 PPU::ReadOAMData()
{
  return m_oam_ram[m_oamAddress];
}

void PPU::WriteOAMData(u8 value)
{
  DebugAssert(m_oamAddress <= sizeof(m_oam_ram));
  m_oam_ram[m_oamAddress] = value;
  m_oamAddress++;
}

void PPU::WriteScroll(u8 value)
{
  if (m_regs.address_latch == 0)
  {
    m_regs.fine_x = value & 0x07;
    m_regs.t.tile_x = (value >> 3) & 0x1F;
    m_regs.address_latch = 1;
  }
  else
  {
    m_regs.t.fine_y = value & 0x07;
    m_regs.t.tile_y = (value >> 3) & 0x1F;
    m_regs.address_latch = 0;
  }
}

void PPU::WriteAddress(u8 value)
{
  if (m_regs.address_latch == 0)
  {
    m_regs.t.address_high = value & 0x3F;
    m_regs.address_latch = 1;
  }
  else
  {
    m_regs.t.address_low = value;
    m_regs.v.address = m_regs.t.address;
    m_regs.address_latch = 0;
  }
}

u8 PPU::ReadData()
{
  if (IsRenderingEnabled() && (m_current_scanline <= 240 || m_current_scanline == 261))
    return 0x00;

  u16 address = m_regs.v.ppudata_address;
  m_regs.v.address += m_vram_increment;

  u8 data = m_ppu_bus_value;
  m_ppu_bus_value = Read(address);

  // CGRAM reads go direct, but still update the bus.
  // Currently, this is wrong.
  if (address >= 0x3F00)
    data = m_ppu_bus_value;

  // Log_DevPrintf("PPU read %04X %02X %02X", address, data, m_ppu_bus_value);
  return data;
}

void PPU::WriteData(u8 value)
{
  if (IsRenderingEnabled() && (m_current_scanline <= 240 || m_current_scanline == 261))
    return;

  // Log_DevPrintf("PPU write %04X %02X", m_regs.v.ppudata_address.GetValue(), value);
  Write(m_regs.v.ppudata_address, value);
  m_regs.v.address += m_vram_increment;
}

void PPU::WriteDMA(u8 value)
{
  const uint32 start_address = uint32(value) << 8;

  // We can optimize this if it's in WRAM.
  if (start_address < 0x2000)
  {
    std::memcpy(m_oam_ram, m_bus->GetWRAM() + (start_address & 0x7FF), 256);
  }
  else
  {
    // Otherwise, we have to do byte reads (in case it's registers, or IO space, etc.)
    for (u32 i = 0; i < 256; i++)
      m_oam_ram[i] = m_bus->ReadCPUAddress(start_address + i);
  }

  m_bus->StallCPU(513);
}

void PPU::UpdateNMILine()
{
  m_bus->SetCPUNMILine(m_nmi_enable && m_nmi_flag);
}

void PPU::FetchNameTableByte()
{
  const u16 address = 0x2000 | m_regs.v.nametable_address;
  m_regs.nametable_byte = Read(address);
}

void PPU::FetchAttributeTableByte()
{
  const u16 address =
    u16(0x23C0) | (u16(m_regs.v.nametable) << 10) | (u16(m_regs.v.tile_y >> 2) << 3) | (u16(m_regs.v.tile_x) >> 2);
  m_regs.next_attribute_byte = Read(address);

  if (m_regs.v.tile_y & 2)
    m_regs.next_attribute_byte >>= 4;
  if (m_regs.v.tile_x & 2)
    m_regs.next_attribute_byte >>= 2;
}

void PPU::CalculateBackgroundTileAddress()
{
  m_regs.tile_address = m_background_table_address | (u16(m_regs.nametable_byte) << 4) | u16(m_regs.v.fine_y);
}

void PPU::IncrementTileX()
{
  m_regs.v.tile_x++;
  if (m_regs.v.tile_x == 0)
    m_regs.v.nametable_x++;
}

void PPU::IncrementTileY()
{
  m_regs.v.fine_y++;
  if (m_regs.v.fine_y == 0)
  {
    m_regs.v.tile_y++;
    if (m_regs.v.tile_y == 30)
    {
      m_regs.v.nametable_y++;
      m_regs.v.tile_y = 0;
    }
  }
}

void PPU::FetchLowTileByte()
{
  m_regs.next_tile_data_low = Read(m_regs.tile_address);
}

void PPU::FetchHighTileByte()
{
  m_regs.next_tile_data_high = Read(m_regs.tile_address + 8);
}

void PPU::StoreTileData()
{
  m_regs.attribute_byte = (m_regs.attribute_byte << 2) | (m_regs.next_attribute_byte & 3);
  m_regs.tile_data_low = (m_regs.tile_data_low << 8) | u16(m_regs.next_tile_data_low);
  m_regs.tile_data_high = (m_regs.tile_data_high << 8) | u16(m_regs.next_tile_data_high);
}

void PPU::RenderPixel()
{
  int32 x = m_current_cycle - 1;
  int32 y = m_current_scanline;
  DebugAssert(x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT);

  u8 color = 0;
  if (m_flagShowBackground && (m_flagShowLeftBackground || x >= 8))
  {
    const u16 tile_bit = u16(0x8000) >> (m_regs.fine_x + (x & 7));
    color = ((m_regs.tile_data_low & tile_bit) ? 1 : 0) | ((m_regs.tile_data_high & tile_bit) ? 2 : 0);
    if (color > 0)
    {
      if (tile_bit >= 0x100)
        color |= (((m_regs.attribute_byte >> 2) & 3) << 2);
      else
        color |= ((m_regs.attribute_byte & 3) << 2);
    }
  }

  u8 sprite_color = 0;
  bool sprite_priority = false;
  if (m_flagShowSprites && (m_flagShowLeftSprites || x >= 8))
  {
    // The lowest sprite index must take precedence.
    for (int8 sprite_index = 0; sprite_index < m_sprite_count; sprite_index++)
    {
      const auto& sprite = m_regs.sprites[sprite_index];
      int32 sprite_x = x - int32(uint32(sprite.x)); // TODO: This can be replaced with unsigned math
      if (sprite.tile == 64 || sprite_x < 0 || sprite_x >= 8)
        continue;

      // Horizontal flip.
      if (sprite.attribute & 0x40)
        sprite_x ^= 0x07;

      // Read two-bit sprite data, most significant bit is the left-most.
      const u8 tile_bit = u8(0x80) >> sprite_x;
      sprite_color = ((sprite.tile_data_low & tile_bit) ? 1 : 0) | ((sprite.tile_data_high & tile_bit) ? 2 : 0);
      if (sprite_color == 0)
      {
        // Transparent sprite?
        continue;
      }

      // Sprite 0 hit background?
      if (sprite.index == 0 && color != 0 && x != 255)
        m_flagSpriteZeroHit = true;

      // Priority determines whether the background or the sprite takes precedence.
      sprite_priority = ConvertToBoolUnchecked((sprite.attribute >> 5) & 0x01);

      // Index sprite palette.
      sprite_color |= (sprite.attribute & 0x03) << 2;
      sprite_color += 16;
      break;
    }
  }

  // Sprite color is used when the background is transparent, or the priority bit is zero.
  if (sprite_color != 0 && (color == 0 || sprite_priority == 0))
    color = sprite_color;

  u8 ci = ReadPalette(color);
  uint32 c = PALETTE[ci % 64];
  m_display->SetPixel(x, y, c);
}

void PPU::EvaluateSprite()
{
  // We can skip this if the overflow bit is already set.
  // Otherwise, we have to test for a potential 9th sprite.
  if (m_sprite_counter == MAX_SPRITES_PER_LINE && m_flagSpriteOverflow)
    return;

  // Note: Scanline is one less than the scanline the sprite will be rendered on.
  u8 sprite_index = m_sprite_current_index++;
  int32 sprite_start_y = m_current_scanline - int32(uint32(m_oam_ram[sprite_index * 4 + 0]));
  if (sprite_start_y < 0 || sprite_start_y >= m_sprite_height)
  {
    // This sprite is not on this line.
    return;
  }

  // On the 9th sprite, set the overflow bit.
  if (m_sprite_counter == MAX_SPRITES_PER_LINE)
  {
    m_flagSpriteOverflow = true;
    return;
  }

  // Copy to secondary OAM for the line rendering.
  m_regs.secondary_oam[m_sprite_counter].y = m_oam_ram[sprite_index * 4 + 0];
  m_regs.secondary_oam[m_sprite_counter].tile = m_oam_ram[sprite_index * 4 + 1];
  m_regs.secondary_oam[m_sprite_counter].attribute = m_oam_ram[sprite_index * 4 + 2];
  m_regs.secondary_oam[m_sprite_counter].x = m_oam_ram[sprite_index * 4 + 3];
  m_regs.secondary_oam[m_sprite_counter].index = sprite_index;
  m_sprite_counter++;
}

void PPU::CalculateSpriteTileAddress(const u8 sprite_index)
{
  // Copy from secondary->sprite output unit.
  auto& sprite = m_regs.sprites[sprite_index];
  sprite.y = m_regs.secondary_oam[sprite_index].y;
  sprite.attribute = m_regs.secondary_oam[sprite_index].attribute;
  sprite.tile = m_regs.secondary_oam[sprite_index].tile;
  sprite.x = m_regs.secondary_oam[sprite_index].x;
  sprite.index = m_regs.secondary_oam[sprite_index].index;

  // Calculate sprite tile address.
  // For 8x16 sprites, the control register bit is ignored.
  if (m_sprite_height == 8)
    m_regs.tile_address = m_sprite_table_address + (u16(sprite.tile) * 16);
  else
    m_regs.tile_address = (u16(sprite.tile & 0xFE) * 16) + (u16(sprite.tile & 0x01) * 0x1000);

  // Line offset. This won't go negative because of the check in EvaluateSprite().
  int32 y_offset = (m_current_scanline - sprite.y) & (m_sprite_height - 1);

  // Vertical flip.
  if (sprite.attribute & 0x80)
    y_offset ^= (m_sprite_height - 1);

  m_regs.tile_address += y_offset;
}

void PPU::FetchLowSpriteTileByte(const u8 sprite_index)
{
  m_regs.sprites[sprite_index].tile_data_low = Read(m_regs.tile_address);
}

void PPU::FetchHighSpriteTileByte(const u8 sprite_index)
{
  m_regs.sprites[sprite_index].tile_data_high = Read(m_regs.tile_address + 8);
}

void PPU::Execute(CycleCount cycles)
{
  while ((cycles--) > 0)
  {
    if ((m_current_scanline == 241 || m_current_scanline == 261) && m_current_cycle == 0)
      m_nmi_flag = m_nmi_hold;
    else if ((m_current_scanline == 241 || m_current_scanline == 261) && m_current_cycle == 2)
      UpdateNMILine();

    // 240-260 - Vertical Blank
    if (m_current_scanline >= 240 && m_current_scanline <= 260)
    {
      if (m_current_cycle == 341)
      {
        m_current_cycle = 0;
        m_current_scanline++;
        // std::fprintf(stderr, "Scanline %u\n", m_current_scanline);
        continue;
      }

      if (m_current_cycle == 340)
      {
        if (m_current_scanline == 240)
        {
          m_nmi_hold = true;
          m_display->DisplayFramebuffer();
          m_system->EndFrame();
        }
        else if (m_current_scanline == 260)
        {
          m_nmi_hold = false;
          m_flagSpriteZeroHit = 0;
          m_flagSpriteOverflow = 0;
        }
      }

      m_current_cycle++;
      continue;
    }

    const bool pre_render_line = (m_current_scanline == 261);
    const bool visible_cycle = (m_current_cycle >= 1 && m_current_cycle <= 256);
    const bool sprite_fetch_cycle = (m_current_cycle >= 257 && m_current_cycle <= 320);
    const bool prefetch_cycle = (m_current_cycle >= 321 && m_current_cycle <= 336);

    if (visible_cycle || prefetch_cycle)
    {
      if (m_current_scanline < 240 && visible_cycle)
        RenderPixel();

      switch (m_current_cycle % 8)
      {
        case 0:
        {
          if (!prefetch_cycle && !pre_render_line && IsRenderingEnabled())
            EvaluateSprite();
          StoreTileData();
        }
        break;
        case 1:
          FetchNameTableByte();
          break;
        case 3:
          FetchAttributeTableByte();
          CalculateBackgroundTileAddress();
          break;
        case 4:
        {
          if (IsRenderingEnabled())
          {
            IncrementTileX();
            if (m_current_cycle == 252)
              IncrementTileY();

            if (!prefetch_cycle && !pre_render_line)
              EvaluateSprite();
          }
        }
        break;
        case 5:
          FetchLowTileByte();
          break;
        case 7:
          FetchHighTileByte();
          break;
      }
    }
    else if (sprite_fetch_cycle)
    {
      // The nametable and attribute fetches are not used.
      // Notes suggest this is because the same fetch circuitry from backgrounds is re-used for sprites.
      const u8 sprite_index = u8((m_current_cycle - 257) / 8);
      switch ((m_current_cycle - 257) % 8)
      {
        case 0:
          // FetchNameTableByte();
          break;

        case 2:
          // FetchAttributeTableByte();
          CalculateSpriteTileAddress(sprite_index);
          break;

        case 4:
          FetchLowSpriteTileByte(sprite_index);
          break;

        case 6:
          FetchHighSpriteTileByte(sprite_index);
          break;
      }
    }

    if (m_current_cycle == 1)
    {
      m_sprite_count = m_sprite_counter;
      std::memset(&m_regs.secondary_oam, 0, sizeof(m_regs.secondary_oam));
      m_sprite_current_index = 0;
      m_sprite_counter = 0;
    }
    if (m_current_cycle == 258)
    {
      if (IsRenderingEnabled())
      {
        m_regs.v.nametable_x = m_regs.t.nametable_x;
        m_regs.v.tile_x = m_regs.t.tile_x;
      }
    }
    else if (m_current_cycle == 305)
    {
      if (m_current_scanline == 261 && IsRenderingEnabled())
        m_regs.v.address = m_regs.t.address;
    }
    else if (m_current_cycle == 341)
    {
      m_current_cycle = 0;
      if (m_current_scanline == 261)
      {
        m_current_scanline = 0;
        m_f ^= 1;
      }
      else
      {
        m_current_scanline++;
      }

      // std::fprintf(stderr, "Scanline %u\n", m_current_scanline);
      continue;
    }

    m_current_cycle++;
  }
}

CycleCount PPU::GetMaxExecutionDelay() const
{
  // The NMI line is altered twice on line 241, so sync tighter here.
  if (m_current_scanline >= 240 || m_current_scanline <= 241)
    return 1;

  return std::max(CYCLES_PER_LINE - m_current_cycle, 1);
}
