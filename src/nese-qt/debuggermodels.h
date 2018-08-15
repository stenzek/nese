#pragma once
#include "nese/cpu_debug_interface.h"
#include <QtCore/QAbstractListModel>
#include <QtCore/QAbstractTableModel>
#include <map>

namespace QtFrontend {

class DebuggerCodeModel : public QAbstractTableModel
{
public:
  DebuggerCodeModel(CPUDebugInterface* intf, QObject* parent = nullptr);
  virtual ~DebuggerCodeModel();

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Returns the row for this instruction pointer
  void resetCodeView(CPUDebugInterface::ProgramCounterType start_address);
  int getRowForAddress(CPUDebugInterface::ProgramCounterType address);
  bool getAddressForRow(CPUDebugInterface::ProgramCounterType* address, int row) const;

  int updateInstructionPointer();

private:
  CPUDebugInterface* m_interface;

  CPUDebugInterface::ProgramCounterType m_start_instruction_pointer = 0;
  CPUDebugInterface::ProgramCounterType m_last_instruction_pointer = 0;
  mutable std::map<int, CPUDebugInterface::ProgramCounterType> m_row_ips;
};

class DebuggerRegistersModel : public QAbstractListModel
{
public:
  DebuggerRegistersModel(CPUDebugInterface* intf, QObject* parent = nullptr);
  virtual ~DebuggerRegistersModel();

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  void invalidateView();

private:
  CPUDebugInterface* m_interface;
};

class DebuggerMemoryModel : public QAbstractTableModel
{
public:
  DebuggerMemoryModel(CPUDebugInterface* intf, QObject* parent = nullptr);
  virtual ~DebuggerMemoryModel();

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  void invalidateView();

private:
  CPUDebugInterface* m_interface;
};

class DebuggerStackModel : public QAbstractListModel
{
public:
  DebuggerStackModel(CPUDebugInterface* intf, QObject* parent = nullptr);
  virtual ~DebuggerStackModel();

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  void invalidateView();

private:
  CPUDebugInterface* m_interface;
};

} // namespace QtFrontend