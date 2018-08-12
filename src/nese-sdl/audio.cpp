#include "audio.h"
#include "YBaseLib/Assert.h"
#include "YBaseLib/Log.h"
#include <SDL.h>
Log_SetChannel(Audio);

namespace SDLFrontend {
Audio::Audio() = default;

Audio::~Audio()
{
  if (m_is_open)
    Audio::CloseDevice();
}

bool Audio::OpenDevice()
{
  DebugAssert(!m_is_open);

  SDL_AudioSpec spec = {};
  spec.freq = m_output_sample_rate;
  spec.channels = m_channels;
  spec.format = AUDIO_S16;
  spec.samples = m_buffer_size;
  spec.callback = AudioCallback;
  spec.userdata = static_cast<void*>(this);

  SDL_AudioSpec obtained = {};
  if (SDL_OpenAudio(&spec, &obtained) < 0)
  {
    Log_ErrorPrintf("SDL_OpenAudio failed");
    return false;
  }

  m_is_open = true;
  return true;
}

void Audio::PauseDevice(bool paused)
{
  SDL_PauseAudio(paused ? 1 : 0);
}

void Audio::CloseDevice()
{
  DebugAssert(m_is_open);
  SDL_CloseAudio();
  m_is_open = false;
}

void Audio::AudioCallback(void* userdata, uint8_t* stream, int len)
{
  Audio* const this_ptr = static_cast<Audio*>(userdata);
  const u32 num_samples = len / sizeof(SampleType) / this_ptr->m_channels;
  const u32 read_samples = this_ptr->ReadSamples(reinterpret_cast<SampleType*>(stream), num_samples);
  const u32 silence_samples = num_samples - read_samples;
  if (silence_samples > 0)
  {
    std::memset(reinterpret_cast<SampleType*>(stream) + (read_samples * this_ptr->m_channels), 0,
                silence_samples * this_ptr->m_channels * sizeof(SampleType));
  }
}

} // namespace SDLFrontend