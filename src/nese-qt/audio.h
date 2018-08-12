#pragma once
#include "common/audio.h"

namespace QtFrontend {
class Audio final : public ::Audio
{
public:
  Audio();
  ~Audio();

protected:
  bool OpenDevice() override;
  void PauseDevice(bool paused) override;
  void CloseDevice() override;

  bool m_is_open = false;
};
} // namespace QtFrontend