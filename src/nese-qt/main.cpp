#include "YBaseLib/Log.h"
#include "mainwindow.h"
#include <QtWidgets/QApplication>
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

  QApplication app(argc, argv);

  auto window = std::make_unique<QtFrontend::MainWindow>();
  window->show();

  return app.exec();
}