#pragma once
#include "common/audio.h"
#include <cstdint>

namespace SDLFrontend {
class Audio final : public ::Audio
{
public:
  Audio();
  ~Audio();

protected:
  bool OpenDevice() override;
  void PauseDevice(bool paused) override;
  void CloseDevice() override;

  static void AudioCallback(void* userdata, uint8_t* stream, int len);

  bool m_is_open = false;
};
} // namespace SDLFrontend