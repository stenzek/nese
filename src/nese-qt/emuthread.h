#pragma once
#include "nese/types.h"
#include <QtCore/QThread>
#include <memory>

class QKeyEvent;

class Cartridge;
class StandardController;
class System;

namespace QtFrontend {

class Audio;
class DisplayWindow;

class EmuThread final : public QThread
{
  Q_OBJECT

public:
  EmuThread(DisplayWindow* display_widget, Audio* audio, QThread* owner_thread);
  ~EmuThread();

Q_SIGNALS:
  void emulationErrorEvent(QString error_text);
  void emulationStartedEvent();
  void emulationPausedEvent(bool paused);
  void emulationStoppedEvent();

public Q_SLOTS:
  void onStartEmulation(QString cartridge_filename);

  void onEmulationPauseRequest(bool paused);
  void onEmulationStopRequest();

  void onSingleStep();
  void onFrameStep();

  void onSetControllerButtonState(int controller, int button, bool state);

protected:
  void run() override;

private:
  void Stop();

  DisplayWindow* m_display_window;
  Audio* m_audio;
  QThread* m_owner_thread;

  std::unique_ptr<StandardController> m_controller_1;
  std::unique_ptr<Cartridge> m_cartridge;
  std::unique_ptr<System> m_system;

  bool m_paused = true;
  bool m_stopped = false;
};

} // namespace QtFrontend