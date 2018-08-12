#include "audio.h"
#include "YBaseLib/Assert.h"

namespace QtFrontend {
Audio::Audio() = default;

Audio::~Audio()
{
  if (m_is_open)
    Audio::CloseDevice();
}

bool Audio::OpenDevice()
{
  DebugAssert(!m_is_open);

  m_is_open = true;
  return true;
}

void Audio::PauseDevice(bool paused) {}

void Audio::CloseDevice()
{
  DebugAssert(m_is_open);
  m_is_open = false;
}

} // namespace QtFrontend