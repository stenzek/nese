#include "emuthread.h"
#include "YBaseLib/ByteStream.h"
#include "YBaseLib/Error.h"
#include "audio.h"
#include "displaywindow.h"
#include "nese/cartridge.h"
#include "nese/controller.h"
#include "nese/system.h"
#include <QtCore/QEventLoop>
#include <QtGui/QKeyEvent>

namespace QtFrontend {

EmuThread::EmuThread(DisplayWindow* display_widget, Audio* audio, QThread* owner_thread)
  : m_display_window(display_widget), m_audio(audio), m_owner_thread(owner_thread)
{
}

EmuThread::~EmuThread() = default;

void EmuThread::onStartEmulation(const QString cartridge_filename)
{
  ByteStream* stream;
  if (!ByteStream_OpenFileStream(cartridge_filename.toStdString().c_str(),
                                 BYTESTREAM_OPEN_READ | BYTESTREAM_OPEN_STREAMED, &stream))
  {
    emit emulationErrorEvent(QString::asprintf("Failed to open file '%s'", cartridge_filename.toStdString().c_str()));
    Stop();
    return;
  }

  // Parse cartridge.
  Error error;
  m_cartridge = Cartridge::Load(stream, &error);
  if (!m_cartridge)
  {
    stream->Release();
    emit emulationErrorEvent(error.GetErrorCodeAndDescription().GetCharArray());
    Stop();
    return;
  }

  // Create system.
  stream->Release();
  m_system = std::make_unique<System>();

  // Attach controllers.
  m_controller_1 = std::make_unique<StandardController>();
  m_system->SetController(0, m_controller_1.get());

  // Initialize the system.
  if (!m_system->Initialize(m_display_window, m_audio, m_cartridge.get()))
  {
    emit emulationErrorEvent("Failed to initialize system.");
    Stop();
    return;
  }

  m_system->Reset();

  emit emulationStartedEvent();

  m_paused = false;
}

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

void EmuThread::onSetControllerButtonState(int controller, int button, bool state)
{
  if (controller == 0 && m_controller_1)
    m_controller_1->SetButtonState(static_cast<u8>(button), state);
  // else if (controller == 1 && m_controller_2)
  // m_controller_2->SetButtonState(static_cast<u8>(button), state);
}

void EmuThread::run()
{
  m_display_window->makeOpenGLContextCurrent();

  QEventLoop eventloop;
  while (!m_stopped)
  {
    // Process all events.
    eventloop.processEvents(QEventLoop::AllEvents, 1000);

    // If we're not paused, execute.
    while (!m_paused)
    {
      m_system->FrameStep();
      eventloop.processEvents(QEventLoop::AllEvents);
    }
  }

  // Destroy all resources we created here.
  m_system.reset();
  m_cartridge.reset();
  m_controller_1.reset();

  // Move the GL context back to the main thread.
  m_display_window->moveOpenGLContextToThread(m_owner_thread);

  emit emulationStoppedEvent();

  // Move back to the main thread, it cleans us up.
  moveToThread(m_owner_thread);
  exit();
}

void EmuThread::Stop()
{
  m_paused = true;
  m_stopped = true;
  emit emulationStoppedEvent();
}

} // namespace QtFrontend