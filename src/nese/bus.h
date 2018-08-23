#pragma once
#include "types.h"

class System;
class CPU;
class PPU;
class APU;
class Cartridge;
class Controller;

class Bus
{
public:
  static const u32 WRAM_SIZE = 2048;
  static const u32 VRAM_SIZE = 2048; // Also known as "CIRAM".
  static const u32 NUM_CONTROLLERS = 2;

  Bus();
  ~Bus();

  void Initialize(CPU* cpu, PPU* ppu, APU* apu);
  void Reset();

  void SetController(uint32 index, Controller* controller) { m_controllers[index] = controller; }
  void SetCartridge(Cartridge* cartridge) { m_cartridge = cartridge; }

  CycleCount GetPendingCycles() const { return m_pending_cycles; }
  void AddPendingCycles(CycleCount cycles) { m_pending_cycles += cycles; }
  void ExecutePendingCycles();

  void EndScanline();

  const byte* GetWRAM() const { return m_wram; }
  const byte* GetVRAM() const { return m_vram; }

  u8 ReadWRAM(u32 offset) { return m_wram[offset]; }
  u8 ReadVRAM(u32 offset) { return m_vram[offset]; }

  void WriteWRAM(u32 offset, u8 value) { m_wram[offset] = value; }
  void WriteVRAM(u32 offset, u8 value) { m_vram[offset] = value; }

  u8 ReadCPUAddress(u16 address);
  void WriteCPUAddress(u16 address, u8 value);

  u8 ReadPPUAddress(u16 address);
  void WritePPUAddress(u16 address, u8 value);

  // Holds the bus for a specified number of cycles, preventing CPU access.
  void StallCPU(u32 num_cycles);

  // Sets/clears the NMI line on the CPU.
  void SetCPUNMILine(bool active);

  // Sets/clears the IRQ line on the CPU.
  void SetCPUIRQLine(bool active);

  // Notifies other components when the PPU finishes rendering a scanline.
  void PPUScanline(u32 line, bool rendering_enabled);

private:
  CPU* m_cpu = nullptr;
  PPU* m_ppu = nullptr;
  APU* m_apu = nullptr;

  Cartridge* m_cartridge = nullptr;
  Controller* m_controllers[2] = {};

  CycleCount m_pending_cycles = 0;

  byte m_wram[WRAM_SIZE];
  byte m_vram[WRAM_SIZE];
};
