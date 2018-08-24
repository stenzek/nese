#pragma once
#include "common/bitfield.h"
#include "types.h"

class System;
class Bus;
class Display;

class PPU
{
public:
  static const u32 SCREEN_WIDTH = 256;
  static const u32 SCREEN_HEIGHT = 240;
  static const u32 MAX_SPRITES_PER_LINE = 8;
  static const CycleCount CYCLES_PER_LINE = 341;

public:
  PPU();
  ~PPU();

  void Initialize(System* system, Bus* bus, Display* display);
  void Reset();

  u8 ReadRegister(u8 address);
  void WriteRegister(u8 address, u8 value);
  void WriteDMA(u8 value);

  // Returns the number of cycles until the next execution of the PPU is required.
  CycleCount GetMaxExecutionDelay() const;

  void Execute(CycleCount cycles);

private:
  bool IsRenderingEnabled() const { return m_flagShowBackground || m_flagShowSprites; }

  System* m_system = nullptr;
  Bus* m_bus = nullptr;
  Display* m_display = nullptr;

  CycleCount m_current_cycle = 0;
  u32 m_current_scanline = 0;

  u8 m_palette_ram[32]; // CGRAM
  u8 m_oam_ram[256];

  struct
  {
    union
    {
      u16 bits;
      BitField<u16, u16, 0, 15> address;
      BitField<u16, u16, 0, 12> nametable_address;
      BitField<u16, u16, 0, 14> ppudata_address;
      BitField<u16, u8, 0, 8> address_low;
      BitField<u16, u8, 8, 7> address_high;
      BitField<u16, u8, 0, 5> tile_x;
      BitField<u16, u8, 5, 5> tile_y;
      BitField<u16, u8, 10, 2> nametable;
      BitField<u16, u8, 10, 1> nametable_x;
      BitField<u16, u8, 11, 1> nametable_y;
      BitField<u16, u8, 12, 3> fine_y;
    } v, t;

    union
    {
      u8 misc_bits;
      BitField<u8, u8, 0, 3> fine_x;
      BitField<u8, u8, 3, 1> address_latch;
    };

    u16 tile_data_low;
    u16 tile_data_high;

    u16 tile_address;
    u8 attribute_byte;
    u8 nametable_byte;
    u8 next_attribute_byte;
    u8 next_tile_data_low;
    u8 next_tile_data_high;

    // For next line
    struct
    {
      u8 y;
      u8 tile;
      u8 attribute;
      u8 x;
      u8 index; // not strictly part of secondary OAM..
    } secondary_oam[MAX_SPRITES_PER_LINE];

    // For current line
    struct
    {
      u8 y;
      u8 tile;
      u8 attribute;
      u8 x;
      u8 index;
      u8 tile_data_low;
      u8 tile_data_high;
    } sprites[MAX_SPRITES_PER_LINE];
  } m_regs;

  u8 m_f; // odd/even frame flag (1 bit)

  u8 m_register;

  bool m_nmi_enable;
  bool m_nmi_hold;
  bool m_nmi_flag;

  u8 m_sprite_current_index = 0; // 0..63, current index
  u8 m_sprite_counter = 0;       // Count for next line
  u8 m_sprite_count = 0;         // Count for current line

  u8 m_vram_increment;
  u16 m_sprite_table_address;
  u16 m_background_table_address;
  u8 m_sprite_height;
  u8 m_flagMasterSlave;

  u8 m_grayscale_flag;
  u8 m_flagShowLeftBackground;
  u8 m_flagShowLeftSprites;
  u8 m_flagShowBackground;
  u8 m_flagShowSprites;
  u8 m_flagRedTint;
  u8 m_flagGreenTint;
  u8 m_flagBlueTint;

  u8 m_flagSpriteZeroHit;
  u8 m_flagSpriteOverflow;

  u8 m_oamAddress;
  u8 m_ppu_bus_value;

  u8 ReadCHR(u16 address);
  u8 ReadCGRAM(u16 address);
  void WriteCGRAM(u16 address, u8 value);

  u8 ReadStatus();
  u8 ReadOAMData();
  u8 ReadData();

  void WriteControl(u8 value);
  void WriteMask(u8 value);
  void WriteOAMAddress(u8 value);
  void WriteOAMData(u8 value);
  void WriteScroll(u8 value);
  void WriteAddress(u8 value);
  void WriteData(u8 value);

  void UpdateNMILine();

  void IncrementTileX();
  void IncrementTileY();

  void FetchNameTableByte();
  void FetchAttributeTableByte();
  void CalculateBackgroundTileAddress();
  void FetchLowTileByte();
  void FetchHighTileByte();
  void StoreTileData();

  void RenderPixel();

  void EvaluateSprite();
  void CalculateSpriteTileAddress(const u8 sprite_index);
  void FetchLowSpriteTileByte(const u8 sprite_index);
  void FetchHighSpriteTileByte(const u8 sprite_index);
};
