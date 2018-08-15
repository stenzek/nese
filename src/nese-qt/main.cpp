#include "YBaseLib/Log.h"
#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QOpenGLWidget>
#include <memory>

static void InitLogging()
{
  // set log flags
  // g_pLog->SetConsoleOutputParams(true);
  g_pLog->SetConsoleOutputParams(true, nullptr, LOGLEVEL_PROFILE);
  g_pLog->SetFilterLevel(LOGLEVEL_PROFILE);
  // g_pLog->SetDebugOutputParams(true);
}

int main(int argc, char* argv[])
{
  InitLogging();

  QSurfaceFormat format;
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setVersion(3, 3);
  format.setRedBufferSize(8);
  format.setGreenBufferSize(8);
  format.setBlueBufferSize(8);
  // format.setSwapInterval(0);
  format.setOption(QSurfaceFormat::DebugContext);
  QSurfaceFormat::setDefaultFormat(format);

  QApplication app(argc, argv);

  auto window = std::make_unique<QtFrontend::MainWindow>();
  window->show();

  // We can't create the GL context before the window.
  if (!window->createOpenGLObjects())
  {
    Panic("Failed to create OpenGL objects.");
    return -1;
  }

  return app.exec();
}