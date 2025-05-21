#pragma once

#include "types.h"

#include "retro2/retro2_fwd.h"

#include <array>
#include <memory>

class Bus;
class CPU;
class PPU;
class APU;
class Controller;
class Cartridge;

class System
{
public:
  static const u32 NUM_CONTROLLERS = 2;

  System();
  ~System();

  Bus* GetBus() { return m_bus.get(); }
  CPU* GetCPU() { return m_cpu.get(); }
  PPU* GetPPU() { return m_ppu.get(); }
  APU* GetAPU() { return m_apu.get(); }

  Cartridge* GetCartridge() { return m_cartridge.get(); }
  void SetCartridge(std::unique_ptr<Cartridge> cartridge);

  Controller* GetController(u32 index) const { return m_controllers[index].get(); }
  void SetController(u32 index, std::unique_ptr<Controller> controller);

  void Initialize(std::unique_ptr<Cartridge> cartridge);
  void Reset();

  void SingleStep();
  void FrameStep();

  u32 GetFrameNumber() const { return m_frame_number; }
  void EndFrame();

private:
  std::unique_ptr<Bus> m_bus;
  std::unique_ptr<CPU> m_cpu;
  std::unique_ptr<PPU> m_ppu;
  std::unique_ptr<APU> m_apu;

  std::unique_ptr<Cartridge> m_cartridge;

  std::array<std::unique_ptr<Controller>, NUM_CONTROLLERS> m_controllers = {};

  u32 m_frame_number = 1;
};

// Retro2 Interfaces
extern IRetro2Error R2Error;
extern IRetro2Settings R2Settings;
extern IRetro2FileSystem R2FileSystem;
extern IRetro2Session R2Session;
extern IRetro2FramebufferVideo R2Video;
