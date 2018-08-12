#pragma once
#include "types.h"
#include <memory>

class Audio;
class Bus;
class CPU;
class PPU;
class APU;
class Controller;
class Cartridge;
class Display;

class System
{
public:
  static const uint32 NUM_CONTROLLERS = 2;

  System();
  ~System();

  Bus* GetBus() { return m_bus.get(); }
  CPU* GetCPU() { return m_cpu.get(); }
  PPU* GetPPU() { return m_ppu.get(); }
  APU* GetAPU() { return m_apu.get(); }

  Cartridge* GetCartridge() { return m_cartridge; }
  void SetCartridge(Cartridge* cartridge);

  Controller* GetController(uint32 index) const { return m_controllers[index]; }
  void SetController(uint32 index, Controller* controller);

  bool Initialize(Display* display, Audio* audio, Cartridge* cartridge);
  void Reset();

  void SingleStep();
  void FrameStep();

  u32 GetFrameNumber() const { return m_frame_number; }
  void EndFrame();

private:
  Display* m_display = nullptr;
  Audio* m_audio = nullptr;

  std::unique_ptr<Bus> m_bus;
  std::unique_ptr<CPU> m_cpu;
  std::unique_ptr<PPU> m_ppu;
  std::unique_ptr<APU> m_apu;

  Cartridge* m_cartridge = nullptr;

  Controller* m_controllers[NUM_CONTROLLERS] = {};

  u32 m_frame_number = 1;
};
