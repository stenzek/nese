#pragma once
#include "types.h"
#include <memory>

class Audio;
class Bus;
class Nes_Apu;
class Blip_Buffer;

class APU
{
public:
  APU();
  ~APU();

  void Initialize(Bus* bus, Audio* audio);
  void Reset();

  u8 ReadRegister(u8 address);
  void WriteRegister(u8 address, u8 value);

  CycleCount GetMaxExecutionDelay() const;

  void Execute(CycleCount cycles);

private:
  void UpdateIRQDelay();

  static int DMCReadCallback(void* userdata, unsigned address);
  static void IRQNotifierCallback(void* userdata);

  Bus* m_bus = nullptr;
  Audio* m_audio = nullptr;

  std::unique_ptr<Nes_Apu> m_apu;
  std::unique_ptr<Blip_Buffer> m_buffer;

  CycleCount m_time_since_last_mix = 0;
  CycleCount m_mix_interval = 1;
  CycleCount m_cycles_until_irq = -1;
};
