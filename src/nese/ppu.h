#pragma once
#include "bitfield.h"
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

  uint8 ReadRegister(u8 address);
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

  uint8 m_paletteData[32]; // CGRAM
  uint8 m_oamData[256];

  struct
  {
    union
    {
      u16 bits;
      BitField<u16, u16, 0, 15> address;
      BitField<u16, u16, 0, 12> nametable_address;
      BitField<u16, u16, 0, 14> ppudata_address;
      BitField<u16, uint8, 0, 8> address_low;
      BitField<u16, uint8, 8, 7> address_high;
      BitField<u16, uint8, 0, 5> tile_x;
      BitField<u16, uint8, 5, 5> tile_y;
      BitField<u16, uint8, 10, 2> nametable;
      BitField<u16, uint8, 10, 1> nametable_x;
      BitField<u16, uint8, 11, 1> nametable_y;
      BitField<u16, uint8, 12, 3> fine_y;
    } v, t;

    union
    {
      uint8 misc_bits;
      BitField<uint8, uint8, 0, 3> fine_x;
      BitField<uint8, uint8, 3, 1> address_latch;
    };

    u16 tile_data_low;
    u16 tile_data_high;

    u16 tile_address;
    uint8 attribute_byte;
    uint8 nametable_byte;
    uint8 next_attribute_byte;
    uint8 next_tile_data_low;
    uint8 next_tile_data_high;

    // For next line
    struct
    {
      uint8 y;
      uint8 tile;
      uint8 attribute;
      uint8 x;
      uint8 index; // not strictly part of secondary OAM..
    } secondary_oam[MAX_SPRITES_PER_LINE];

    // For current line
    struct
    {
      uint8 y;
      uint8 tile;
      uint8 attribute;
      uint8 x;
      uint8 index;
      uint8 tile_data_low;
      uint8 tile_data_high;
    } sprites[MAX_SPRITES_PER_LINE];
  } m_regs;

  uint8 m_f; // odd/even frame flag (1 bit)

  uint8 m_register;

  bool m_nmi_enable;
  bool m_nmi_hold;
  bool m_nmi_flag;

  uint8 m_sprite_current_index = 0; // 0..63, current index
  uint8 m_sprite_counter = 0;       // Count for next line
  uint8 m_sprite_count = 0;         // Count for current line

  uint8 m_vram_increment;
  u16 m_sprite_table_address;
  u16 m_background_table_address;
  uint8 m_sprite_height;
  uint8 m_flagMasterSlave;

  uint8 m_flagGrayscale;
  uint8 m_flagShowLeftBackground;
  uint8 m_flagShowLeftSprites;
  uint8 m_flagShowBackground;
  uint8 m_flagShowSprites;
  uint8 m_flagRedTint;
  uint8 m_flagGreenTint;
  uint8 m_flagBlueTint;

  uint8 m_flagSpriteZeroHit;
  uint8 m_flagSpriteOverflow;

  uint8 m_oamAddress;
  uint8 m_ppu_bus_value;

  uint8 Read(u16 address);
  void Write(u16 address, uint8 value);

  uint8 ReadPalette(u16 address);
  void WritePalette(u16 address, uint8 value);

  uint8 ReadStatus();
  uint8 ReadOAMData();
  uint8 ReadData();

  void WriteControl(uint8 value);
  void WriteMask(uint8 value);
  void WriteOAMAddress(uint8 value);
  void WriteOAMData(uint8 value);
  void WriteScroll(uint8 value);
  void WriteAddress(uint8 value);
  void WriteData(uint8 value);

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
  void CalculateSpriteTileAddress(const uint8 sprite_index);
  void FetchLowSpriteTileByte(const uint8 sprite_index);
  void FetchHighSpriteTileByte(const uint8 sprite_index);
};
