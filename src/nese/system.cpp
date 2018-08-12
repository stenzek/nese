#include "system.h"
#include "YBaseLib/Timer.h"
#include "apu.h"
#include "audio.h"
#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"

System::System()
  : m_bus(std::make_unique<Bus>()), m_cpu(std::make_unique<CPU>()), m_ppu(std::make_unique<PPU>()),
    m_apu(std::make_unique<APU>())
{
}

System::~System() = default;

void System::SetCartridge(Cartridge* cartridge)
{
  m_cartridge = cartridge;
  m_bus->SetCartridge(cartridge);
}

void System::SetController(uint32 index, Controller* controller)
{
  m_controllers[index] = controller;
  m_bus->SetController(index, controller);
}

bool System::Initialize(Display* display, Audio* audio, Cartridge* cartridge)
{
  m_display = display;
  m_audio = audio;

  if (!m_audio->Reconfigure(Audio::DefaultOutputSampleRate, 1))
    return false;

  m_bus->Initialize(m_cpu.get(), m_ppu.get(), m_apu.get());
  m_cpu->Initialize(this, m_bus.get());
  m_ppu->Initialize(this, m_bus.get(), m_display);
  m_apu->Initialize(m_bus.get(), audio);
  SetCartridge(cartridge);
  return true;
}

void System::Reset()
{
  m_audio->EmptyBuffers();
  m_bus->Reset();
  m_cartridge->Reset();
  m_ppu->Reset();
  m_apu->Reset();
  m_cpu->Reset();
  m_frame_number = 1;
}

void System::SingleStep()
{
  m_cpu->Execute(1);
  m_bus->ExecutePendingCycles();
}

void System::FrameStep()
{
  const u32 prev_frame_number = m_frame_number;
  while (m_frame_number == prev_frame_number)
  {
    const u32 cpu_cycles = std::min(m_ppu->GetMaxExecutionDelay(), m_apu->GetMaxExecutionDelay());
    // m_cpu->Execute(cpu_cycles);
    m_cpu->Execute(1);
    m_bus->ExecutePendingCycles();
  }
}

void System::EndFrame()
{
  m_frame_number++;

#if 0
  static Timer tmr;
  static u32 lf;
  static u32 cc;
  if (tmr.GetTimeSeconds() > 1.0f)
  {
    u32 f = m_frame_number - lf;
    std::fprintf(stderr, "%u frames in %f seconds (%f fps), %u cycles\n", f, tmr.GetTimeSeconds(), f / tmr.GetTimeSeconds(), m_cpu->GetCyclesSinceReset() - cc);
    tmr.Reset();
    lf = m_frame_number;
    cc = m_cpu->GetCyclesSinceReset();
  }
#endif
}
