#pragma once

#include "ui_mainwindow.h"
#include <QtCore/QThread>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <memory>

namespace QtFrontend {

class Audio;
class DisplayWidget;
class EmuThread;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

public Q_SLOTS:
  void onPowerActionToggled(bool selected);
  void onResetActionTriggered();
  void onAboutActionTriggered();

  void onDisplayWidgetKeyPressed(QKeyEvent* event);
  void onDisplayWidgetKeyReleased(QKeyEvent* event);

private:
  void connectSignals();
  void connectEmuThreadSignals();

  std::unique_ptr<Ui::MainWindow> m_ui;

  DisplayWidget* m_display_widget = nullptr;
  QLabel* m_status_message = nullptr;
  QLabel* m_status_speed = nullptr;
  QLabel* m_status_fps = nullptr;

  std::unique_ptr<Audio> m_audio;

  std::unique_ptr<EmuThread> m_emu_thread;
};
} // namespace QtFrontend