#include "mainwindow.h"
#include "YBaseLib/ByteStream.h"
#include "audio.h"
#include "displaywindow.h"
#include "emuthread.h"
#include "nese/controller.h"
#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

namespace QtFrontend {

MainWindow::MainWindow(QWidget* parent /*= nullptr*/) : QMainWindow(parent)
{
  m_ui = std::make_unique<Ui::MainWindow>();
  m_ui->setupUi(this);

  m_display_window = new DisplayWindow();
  m_display_window->setBaseSize(QSize(512, 512));
  m_display_widget = QWidget::createWindowContainer(m_display_window, this);
  m_display_widget->setFocusPolicy(Qt::StrongFocus);
  setCentralWidget(m_display_widget);

  m_ui->centralwidget->deleteLater();
  m_ui->centralwidget = nullptr;

  m_status_message = new QLabel(this);
  m_ui->statusbar->addWidget(m_status_message, 1);
  m_status_speed = new QLabel(this);
  m_ui->statusbar->addWidget(m_status_speed, 0);
  m_status_fps = new QLabel(this);
  m_ui->statusbar->addWidget(m_status_fps, 0);

  m_audio = std::make_unique<Audio>();

  connectSignals();
  adjustSize();
}

MainWindow::~MainWindow() {}

bool MainWindow::createOpenGLObjects()
{
  return m_display_window->createOpenGLObjects();
}

void MainWindow::onLoadCartridgeActionTriggered()
{
  if (m_emu_thread)
    return;

#if 1
  QString filename = QFileDialog::getOpenFileName(
    this, tr("Load Cartridge File"), QString(), tr("iNES File (*.nes);;All Files (*)"), nullptr, QFileDialog::ReadOnly);
  if (filename.isEmpty())
    return;
#else
  QString filename = "testroms/RGB.NES";
#endif

  // Create, connect thread signals, and then kick it off.
  createEmuThread();

  // Queue the load.
  queueStartEmulation(filename);
}

void MainWindow::onPowerActionToggled(bool selected) {}

void MainWindow::onResetActionTriggered() {}

void MainWindow::onAboutActionTriggered()
{
  QMessageBox::about(this, tr("NES Emulator"), tr("Blah!"));
}

void MainWindow::onDisplayWindowKeyPressed(QKeyEvent* event)
{
  if (!m_emu_thread)
    return;

  // TODO: Proper key bindings.
  switch (event->key())
  {
    case Qt::Key_W:
    case Qt::Key_Up:
      emit queueSetControllerButtonState(0, StandardController::Button_Up, true);
      break;

    case Qt::Key_S:
    case Qt::Key_Down:
      emit queueSetControllerButtonState(0, StandardController::Button_Down, true);
      break;

    case Qt::Key_A:
    case Qt::Key_Left:
      emit queueSetControllerButtonState(0, StandardController::Button_Left, true);
      break;

    case Qt::Key_D:
    case Qt::Key_Right:
      emit queueSetControllerButtonState(0, StandardController::Button_Right, true);
      break;

    case Qt::Key_Return:
      emit queueSetControllerButtonState(0, StandardController::Button_Start, true);
      break;

    case Qt::Key_Control:
      emit queueSetControllerButtonState(0, StandardController::Button_Select, true);
      break;

    case Qt::Key_Z:
      emit queueSetControllerButtonState(0, StandardController::Button_B, true);
      break;

    case Qt::Key_X:
      emit queueSetControllerButtonState(0, StandardController::Button_A, true);
      break;

    default:
      break;
  }
}

void MainWindow::onDisplayWindowKeyReleased(QKeyEvent* event)
{
  if (!m_emu_thread)
    return;

  // TODO: Proper key bindings.
  switch (event->key())
  {
    case Qt::Key_W:
    case Qt::Key_Up:
      emit queueSetControllerButtonState(0, StandardController::Button_Up, false);
      break;

    case Qt::Key_S:
    case Qt::Key_Down:
      emit queueSetControllerButtonState(0, StandardController::Button_Down, false);
      break;

    case Qt::Key_A:
    case Qt::Key_Left:
      emit queueSetControllerButtonState(0, StandardController::Button_Left, false);
      break;

    case Qt::Key_D:
    case Qt::Key_Right:
      emit queueSetControllerButtonState(0, StandardController::Button_Right, false);
      break;

    case Qt::Key_Return:
      emit queueSetControllerButtonState(0, StandardController::Button_Start, false);
      break;

    case Qt::Key_Control:
      emit queueSetControllerButtonState(0, StandardController::Button_Select, false);
      break;

    case Qt::Key_Z:
      emit queueSetControllerButtonState(0, StandardController::Button_B, false);
      break;

    case Qt::Key_X:
      emit queueSetControllerButtonState(0, StandardController::Button_A, false);
      break;

    default:
      break;
  }
}

void MainWindow::onEmulationError(QString error_text)
{
  QMessageBox::critical(this, tr("Emulation Error"), error_text);
}

void MainWindow::onEmulationStarted() {}

void MainWindow::onEmulationPaused(bool paused) {}

void MainWindow::onEmulationStopped()
{
  // Move the GL context back to the main thread.
  m_display_window->makeOpenGLContextCurrent();
  m_emu_thread = nullptr;
}

void MainWindow::connectSignals()
{
  connect(m_ui->actionLoadCartridge, SIGNAL(triggered()), this, SLOT(onLoadCartridgeActionTriggered()));
  connect(m_ui->actionPower, SIGNAL(triggered(bool)), this, SLOT(onPowerActionToggled(bool)));
  connect(m_ui->action_About, SIGNAL(triggered()), this, SLOT(onAboutActionTriggered()));

  connect(m_display_window, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(onDisplayWindowKeyPressed(QKeyEvent*)));
  connect(m_display_window, SIGNAL(keyReleased(QKeyEvent*)), this, SLOT(onDisplayWindowKeyReleased(QKeyEvent*)));
}

void MainWindow::createEmuThread()
{
  m_emu_thread = new EmuThread(m_display_window, m_audio.get(), QThread::currentThread());
  m_display_window->moveOpenGLContextToThread(m_emu_thread);
  m_emu_thread->moveToThread(m_emu_thread);
  m_emu_thread->start();

  connect(this, SIGNAL(queueStartEmulation(QString)), m_emu_thread, SLOT(onStartEmulation(QString)));
  connect(this, SIGNAL(queuePauseEmulationRequest(bool)), m_emu_thread, SLOT(onEmulationPauseRequest(bool)));
  connect(this, SIGNAL(queueStopEmulationRequest()), m_emu_thread, SLOT(onEmulationStopRequest()));
  connect(this, SIGNAL(queueSingleStep()), m_emu_thread, SLOT(onSingleStep()));
  connect(this, SIGNAL(queueSetControllerButtonState(int, int, bool)), m_emu_thread,
          SLOT(onSetControllerButtonState(int, int, bool)));

  connect(m_emu_thread, SIGNAL(emulationErrorEvent(QString)), this, SLOT(onEmulationError(QString)),
          Qt::BlockingQueuedConnection);
  connect(m_emu_thread, SIGNAL(emulationStartedEvent()), this, SLOT(onEmulationStarted()),
          Qt::BlockingQueuedConnection);
  connect(m_emu_thread, SIGNAL(emulationPausedEvent(bool)), this, SLOT(onEmulationPaused(bool)),
          Qt::BlockingQueuedConnection);
  connect(m_emu_thread, SIGNAL(emulationStoppedEvent()), this, SLOT(onEmulationStopped()),
          Qt::BlockingQueuedConnection);
  connect(m_emu_thread, SIGNAL(finished()), m_emu_thread, SLOT(deleteLater()));

  // Switch focus to the display widget, so it catches input.
  m_display_widget->setFocus();
}
} // namespace QtFrontend