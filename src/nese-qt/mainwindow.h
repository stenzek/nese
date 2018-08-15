#pragma once

#include "common/types.h"
#include "ui_mainwindow.h"
#include <QtCore/QThread>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <memory>

namespace QtFrontend {

class Audio;
class DisplayWindow;
class EmuThread;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  bool createOpenGLObjects();

public Q_SLOTS:
  void onLoadCartridgeActionTriggered();
  void onPowerActionToggled(bool selected);
  void onResetActionTriggered();
  void onAboutActionTriggered();

  void onDisplayWindowKeyPressed(QKeyEvent* event);
  void onDisplayWindowKeyReleased(QKeyEvent* event);

  void onEmulationError(QString error_text);
  void onEmulationStarted();
  void onEmulationPaused(bool paused);
  void onEmulationStopped();

Q_SIGNALS:
  // Passing to emulation thread.
  void queueStartEmulation(QString cartridge_filename);
  void queueSetControllerButtonState(int controller, int button_index, bool state);
  void queuePauseEmulationRequest(bool paused);
  void queueStopEmulationRequest();
  void queueSingleStep();
  void queueFrameStep();

private:
  void connectSignals();
  void createEmuThread();

  std::unique_ptr<Ui::MainWindow> m_ui;

  DisplayWindow* m_display_window = nullptr;
  QWidget* m_display_widget = nullptr;
  QLabel* m_status_message = nullptr;
  QLabel* m_status_speed = nullptr;
  QLabel* m_status_fps = nullptr;

  std::unique_ptr<Audio> m_audio;

  EmuThread* m_emu_thread = nullptr;
};
} // namespace QtFrontend