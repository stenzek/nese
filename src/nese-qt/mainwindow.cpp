#include "mainwindow.h"
#include "YBaseLib/ByteStream.h"
#include "audio.h"
#include "displaywidget.h"
#include "emuthread.h"
#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

namespace QtFrontend {

MainWindow::MainWindow(QWidget* parent /*= nullptr*/) : QMainWindow(parent)
{
  m_ui = std::make_unique<Ui::MainWindow>();
  m_ui->setupUi(this);

  m_display_widget = new DisplayWidget(this);
  setCentralWidget(m_display_widget);

  m_ui->centralwidget->deleteLater();
  m_ui->centralwidget = nullptr;

  m_status_message = new QLabel(this);
  m_ui->statusbar->addWidget(m_status_message, 1);
  m_status_speed = new QLabel(this);
  m_ui->statusbar->addWidget(m_status_speed, 0);
  m_status_fps = new QLabel(this);
  m_ui->statusbar->addWidget(m_status_fps, 0);

  connectSignals();
  adjustSize();
}

MainWindow::~MainWindow() = default;

void MainWindow::onPowerActionToggled(bool selected) {}

void MainWindow::onResetActionTriggered() {}

void MainWindow::onAboutActionTriggered()
{
  QMessageBox::about(this, tr("NES Emulator"), tr("Blah!"));
}

void MainWindow::onDisplayWidgetKeyPressed(QKeyEvent* event) {}

void MainWindow::onDisplayWidgetKeyReleased(QKeyEvent* event) {}

void MainWindow::connectSignals()
{
  connect(m_ui->actionEnableDebugger, SIGNAL(toggled(bool)), this, SLOT(onEnableDebuggerActionToggled(bool)));
  connect(m_ui->action_About, SIGNAL(triggered()), this, SLOT(onAboutActionTriggered()));
  connect(m_ui->actionChange_Floppy_A, SIGNAL(triggered()), this, SLOT(onChangeFloppyATriggered()));

  connect(m_display_widget, SIGNAL(onKeyPressed(QKeyEvent*)), this, SLOT(onDisplayWidgetKeyPressed(QKeyEvent*)));
  connect(m_display_widget, SIGNAL(onKeyReleased(QKeyEvent*)), this, SLOT(onDisplayWidgetKeyReleased(QKeyEvent*)));
}

void MainWindow::connectEmuThreadSignals() {}

} // namespace QtFrontend