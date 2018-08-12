#pragma once
#include "nese/types.h"
#include <QtCore/QThread>
#include <memory>

class Error;

class Cartridge;
class StandardController;
class System;

namespace QtFrontend {

class DisplayWidget;

class EmuThread : public QThread
{
  Q_OBJECT

public:
  EmuThread(DisplayWidget* display_widget);
  ~EmuThread();

Q_SIGNALS:
  void emulationErrorEvent(Error* error);
  void emulationStartedEvent();
  void emulationPausedEvent(bool paused);
  void emulationStoppedEvent();

public Q_SLOTS:
  void onStartEmulation(const QString cartridge_filename);

  void onControllerButtonPressed(u32 controller, u32 button_index);

  void onEmulationPauseRequest(bool paused);
  void onEmulationStopRequest();

  void onSingleStep();
  void onFrameStep();

private:
  DisplayWidget* m_display_widget;

  std::unique_ptr<StandardController> m_controller_1;
  std::unique_ptr<Cartridge> m_cartridge;
  std::unique_ptr<System> m_system;

  bool m_paused = true;
  bool m_stopped = false;
};

} // namespace QtFrontend