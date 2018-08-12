#include "emuthread.h"
#include "displaywidget.h"
#include "nese/cartridge.h"
#include "nese/controller.h"
#include "nese/system.h"

namespace QtFrontend {

EmuThread::EmuThread(DisplayWidget* display_widget) : m_display_widget(display_widget) {}

EmuThread::~EmuThread() = default;

void EmuThread::onStartEmulation(const QString cartridge_filename) {}

void EmuThread::onControllerButtonPressed(u32 controller, u32 button_index) {}

void EmuThread::onEmulationPauseRequest(bool paused)
{
  if (paused == m_paused)
    return;

  m_paused = paused;
  emit emulationPausedEvent(paused);
}

void EmuThread::onEmulationStopRequest()
{
  m_stopped = true;
}

void EmuThread::onSingleStep() {}

void EmuThread::onFrameStep() {}
} // namespace QtFrontend