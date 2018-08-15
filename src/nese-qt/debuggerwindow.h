#pragma once
#include <QtWidgets/QMainWindow>
#include <memory>

class CPUDebugInterface;

namespace Ui {
class DebuggerWindow;
}

namespace QtFrontend {

class DebuggerCodeModel;
class DebuggerRegistersModel;
class DebuggerMemoryModel;
class DebuggerStackModel;

class DebuggerWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit DebuggerWindow(CPUDebugInterface* debugger_interface, QWidget* parent = nullptr);
  ~DebuggerWindow();

  void onExecutionContinued();
  void onExecutionStopped();

public Q_SLOTS:
  void refreshAll();
  void onCloseActionTriggered();
  void onRunActionTriggered(bool checked);
  void onSingleStepActionTriggered();

private:
  void connectSignals();
  void createModels();
  void setMonitorUIState(bool enabled);

  CPUDebugInterface* m_debugger_interface;

  std::unique_ptr<Ui::DebuggerWindow> m_ui;

  std::unique_ptr<DebuggerCodeModel> m_code_model;
  std::unique_ptr<DebuggerRegistersModel> m_registers_model;
  std::unique_ptr<DebuggerMemoryModel> m_memory_model;
  std::unique_ptr<DebuggerStackModel> m_stack_model;
};

} // namespace QtFrontend