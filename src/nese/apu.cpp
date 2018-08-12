#include "apu.h"
#include "YBaseLib/Timer.h"
#include "bus.h"
#include "common/audio.h"
#include "nes_apu/Blip_Buffer.h"
#include "nes_apu/Nes_Apu.h"
#include "nes_apu/apu_snapshot.h"

APU::APU() : m_apu(std::make_unique<Nes_Apu>()), m_buffer(std::make_unique<Blip_Buffer>()) {}

APU::~APU()
{
  if (m_audio)
    m_audio->PauseOutput(true);
}

void APU::Initialize(Bus* bus, Audio* audio)
{
  m_bus = bus;
  m_audio = audio;

  m_buffer->clock_rate(1789773);
  m_buffer->sample_rate(m_audio->GetOutputSampleRate());
  m_apu->output(m_buffer.get());
  m_apu->dmc_reader(DMCReadCallback, this);
  m_apu->irq_notifier(IRQNotifierCallback, this);

  m_audio->PauseOutput(false);

  // m_mix_interval = (1789773 + (m_audio->GetBufferSize() - 1)) / m_audio->GetBufferSize();
  m_mix_interval = (1789773 + (60 - 1)) / 60;
  std::fprintf(stderr, "Audio output every %d cycles\n", m_mix_interval);
}

void APU::Reset()
{
  m_time_since_last_mix = 0;
  m_cycles_until_irq = -1;
  m_apu->reset(false, 0);
  UpdateIRQDelay();
}

u8 APU::ReadRegister(u8 address)
{
  if (address == 0x15) // SND_CHN
  {
    // m_bus->SetCPUIRQLine(false);
    return m_apu->read_status(m_time_since_last_mix + m_bus->GetPendingCycles());
  }
  else
  {
    return 0x00;
  }
}

void APU::WriteRegister(u8 address, u8 value)
{
  // if (address == 0x15)
  // m_bus->SetCPUIRQLine(false);

  m_apu->write_register(m_time_since_last_mix + m_bus->GetPendingCycles(), 0x4000 | cpu_addr_t(address),
                        int(unsigned(value)));
}

CycleCount APU::GetMaxExecutionDelay() const
{
  return (m_mix_interval - m_time_since_last_mix);
}

void APU::Execute(CycleCount cycles)
{
  m_time_since_last_mix += cycles;

  if (m_time_since_last_mix >= m_mix_interval)
  {
    m_apu->end_frame(m_time_since_last_mix);
    m_buffer->end_frame(m_time_since_last_mix);
    m_time_since_last_mix = 0;

    while (m_buffer->samples_avail() > 0)
    {
      Audio::SampleType* samples;
      u32 free_sample_count;
      m_audio->BeginWrite(&samples, &free_sample_count);

      u32 max_samples = std::min(u32(m_buffer->samples_avail()), free_sample_count);

      u32 num_samples_read = m_buffer->read_samples(samples, max_samples);
      m_audio->EndWrite(max_samples);

#if 0
      static Timer t;
      static u32 s;
      s += num_samples_read;
      if (t.GetTimeSeconds() > 1.0f)
      {
        std::fprintf(stderr, "%u samples in %f seconds (%f fps)\n", s, t.GetTimeSeconds(), s / t.GetTimeSeconds());
        t.Reset();
        s = 0;
      }
#endif
    }
  }

  if (m_cycles_until_irq >= 0)
  {
    if (cycles >= m_cycles_until_irq)
    {
      // m_bus->SetCPUIRQLine(true);
      // m_cycles_until_irq = -1;
    }
    else
    {
      // m_cycles_until_irq -= cycles;
    }
  }
}

void APU::UpdateIRQDelay()
{
  cpu_time_t earliest_irq = m_apu->earliest_irq();
  if (earliest_irq == Nes_Apu::no_irq)
  {
    m_bus->SetCPUIRQLine(false);
    m_cycles_until_irq = -1;
  }
  else if (earliest_irq == Nes_Apu::irq_waiting)
  {
    m_bus->SetCPUIRQLine(true);
    m_cycles_until_irq = -1;
  }
  else
  {
    m_bus->SetCPUIRQLine(false);
    m_cycles_until_irq = earliest_irq - m_time_since_last_mix;
  }
}

int APU::DMCReadCallback(void* userdata, unsigned address)
{
  APU* const apu = reinterpret_cast<APU*>(userdata);
  apu->m_bus->StallCPU(1);
  return int(unsigned(apu->m_bus->ReadCPUAddress(address)));
}

void APU::IRQNotifierCallback(void* userdata)
{
  APU* const apu = reinterpret_cast<APU*>(userdata);
  apu->UpdateIRQDelay();
}
