/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QtFrontend__MainWindow_t
{
  QByteArrayData data[28];
  char stringdata0[505];
};
#define QT_MOC_LITERAL(idx, ofs, len)                                                                                  \
  Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(                                                             \
    len,                                                                                                               \
    qptrdiff(offsetof(qt_meta_stringdata_QtFrontend__MainWindow_t, stringdata0) + ofs - idx * sizeof(QByteArrayData)))
static const qt_meta_stringdata_QtFrontend__MainWindow_t qt_meta_stringdata_QtFrontend__MainWindow = {
  {
    QT_MOC_LITERAL(0, 0, 22),    // "QtFrontend::MainWindow"
    QT_MOC_LITERAL(1, 23, 19),   // "queueStartEmulation"
    QT_MOC_LITERAL(2, 43, 0),    // ""
    QT_MOC_LITERAL(3, 44, 18),   // "cartridge_filename"
    QT_MOC_LITERAL(4, 63, 29),   // "queueSetControllerButtonState"
    QT_MOC_LITERAL(5, 93, 10),   // "controller"
    QT_MOC_LITERAL(6, 104, 12),  // "button_index"
    QT_MOC_LITERAL(7, 117, 5),   // "state"
    QT_MOC_LITERAL(8, 123, 26),  // "queuePauseEmulationRequest"
    QT_MOC_LITERAL(9, 150, 6),   // "paused"
    QT_MOC_LITERAL(10, 157, 25), // "queueStopEmulationRequest"
    QT_MOC_LITERAL(11, 183, 15), // "queueSingleStep"
    QT_MOC_LITERAL(12, 199, 14), // "queueFrameStep"
    QT_MOC_LITERAL(13, 214, 30), // "onLoadCartridgeActionTriggered"
    QT_MOC_LITERAL(14, 245, 20), // "onPowerActionToggled"
    QT_MOC_LITERAL(15, 266, 8),  // "selected"
    QT_MOC_LITERAL(16, 275, 22), // "onResetActionTriggered"
    QT_MOC_LITERAL(17, 298, 29), // "onEnableDebuggerActionToggled"
    QT_MOC_LITERAL(18, 328, 22), // "onAboutActionTriggered"
    QT_MOC_LITERAL(19, 351, 25), // "onDisplayWindowKeyPressed"
    QT_MOC_LITERAL(20, 377, 10), // "QKeyEvent*"
    QT_MOC_LITERAL(21, 388, 5),  // "event"
    QT_MOC_LITERAL(22, 394, 26), // "onDisplayWindowKeyReleased"
    QT_MOC_LITERAL(23, 421, 16), // "onEmulationError"
    QT_MOC_LITERAL(24, 438, 10), // "error_text"
    QT_MOC_LITERAL(25, 449, 18), // "onEmulationStarted"
    QT_MOC_LITERAL(26, 468, 17), // "onEmulationPaused"
    QT_MOC_LITERAL(27, 486, 18)  // "onEmulationStopped"

  },
  "QtFrontend::MainWindow\0queueStartEmulation\0"
  "\0cartridge_filename\0queueSetControllerButtonState\0"
  "controller\0button_index\0state\0"
  "queuePauseEmulationRequest\0paused\0"
  "queueStopEmulationRequest\0queueSingleStep\0"
  "queueFrameStep\0onLoadCartridgeActionTriggered\0"
  "onPowerActionToggled\0selected\0"
  "onResetActionTriggered\0"
  "onEnableDebuggerActionToggled\0"
  "onAboutActionTriggered\0onDisplayWindowKeyPressed\0"
  "QKeyEvent*\0event\0onDisplayWindowKeyReleased\0"
  "onEmulationError\0error_text\0"
  "onEmulationStarted\0onEmulationPaused\0"
  "onEmulationStopped"};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtFrontend__MainWindow[] = {

  // content:
  7,      // revision
  0,      // classname
  0, 0,   // classinfo
  17, 14, // methods
  0, 0,   // properties
  0, 0,   // enums/sets
  0, 0,   // constructors
  0,      // flags
  6,      // signalCount

  // signals: name, argc, parameters, tag, flags
  1, 1, 99, 2, 0x06 /* Public */, 4, 3, 102, 2, 0x06 /* Public */, 8, 1, 109, 2, 0x06 /* Public */, 10, 0, 112, 2,
  0x06 /* Public */, 11, 0, 113, 2, 0x06 /* Public */, 12, 0, 114, 2, 0x06 /* Public */,

  // slots: name, argc, parameters, tag, flags
  13, 0, 115, 2, 0x0a /* Public */, 14, 1, 116, 2, 0x0a /* Public */, 16, 0, 119, 2, 0x0a /* Public */, 17, 1, 120, 2,
  0x0a /* Public */, 18, 0, 123, 2, 0x0a /* Public */, 19, 1, 124, 2, 0x0a /* Public */, 22, 1, 127, 2,
  0x0a /* Public */, 23, 1, 130, 2, 0x0a /* Public */, 25, 0, 133, 2, 0x0a /* Public */, 26, 1, 134, 2,
  0x0a /* Public */, 27, 0, 137, 2, 0x0a /* Public */,

  // signals: parameters
  QMetaType::Void, QMetaType::QString, 3, QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Bool, 5, 6, 7,
  QMetaType::Void, QMetaType::Bool, 9, QMetaType::Void, QMetaType::Void, QMetaType::Void,

  // slots: parameters
  QMetaType::Void, QMetaType::Void, QMetaType::Bool, 15, QMetaType::Void, QMetaType::Void, QMetaType::Bool, 15,
  QMetaType::Void, QMetaType::Void, 0x80000000 | 20, 21, QMetaType::Void, 0x80000000 | 20, 21, QMetaType::Void,
  QMetaType::QString, 24, QMetaType::Void, QMetaType::Void, QMetaType::Bool, 9, QMetaType::Void,

  0 // eod
};

void QtFrontend::MainWindow::qt_static_metacall(QObject* _o, QMetaObject::Call _c, int _id, void** _a)
{
  if (_c == QMetaObject::InvokeMetaMethod)
  {
    Q_ASSERT(staticMetaObject.cast(_o));
    MainWindow* _t = static_cast<MainWindow*>(_o);
    Q_UNUSED(_t)
    switch (_id)
    {
      case 0:
        _t->queueStartEmulation((*reinterpret_cast<QString(*)>(_a[1])));
        break;
      case 1:
        _t->queueSetControllerButtonState((*reinterpret_cast<int(*)>(_a[1])), (*reinterpret_cast<int(*)>(_a[2])),
                                          (*reinterpret_cast<bool(*)>(_a[3])));
        break;
      case 2:
        _t->queuePauseEmulationRequest((*reinterpret_cast<bool(*)>(_a[1])));
        break;
      case 3:
        _t->queueStopEmulationRequest();
        break;
      case 4:
        _t->queueSingleStep();
        break;
      case 5:
        _t->queueFrameStep();
        break;
      case 6:
        _t->onLoadCartridgeActionTriggered();
        break;
      case 7:
        _t->onPowerActionToggled((*reinterpret_cast<bool(*)>(_a[1])));
        break;
      case 8:
        _t->onResetActionTriggered();
        break;
      case 9:
        _t->onEnableDebuggerActionToggled((*reinterpret_cast<bool(*)>(_a[1])));
        break;
      case 10:
        _t->onAboutActionTriggered();
        break;
      case 11:
        _t->onDisplayWindowKeyPressed((*reinterpret_cast<QKeyEvent*(*)>(_a[1])));
        break;
      case 12:
        _t->onDisplayWindowKeyReleased((*reinterpret_cast<QKeyEvent*(*)>(_a[1])));
        break;
      case 13:
        _t->onEmulationError((*reinterpret_cast<QString(*)>(_a[1])));
        break;
      case 14:
        _t->onEmulationStarted();
        break;
      case 15:
        _t->onEmulationPaused((*reinterpret_cast<bool(*)>(_a[1])));
        break;
      case 16:
        _t->onEmulationStopped();
        break;
      default:;
    }
  }
  else if (_c == QMetaObject::IndexOfMethod)
  {
    int* result = reinterpret_cast<int*>(_a[0]);
    void** func = reinterpret_cast<void**>(_a[1]);
    {
      typedef void (MainWindow::*_t)(QString);
      if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&MainWindow::queueStartEmulation))
      {
        *result = 0;
        return;
      }
    }
    {
      typedef void (MainWindow::*_t)(int, int, bool);
      if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&MainWindow::queueSetControllerButtonState))
      {
        *result = 1;
        return;
      }
    }
    {
      typedef void (MainWindow::*_t)(bool);
      if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&MainWindow::queuePauseEmulationRequest))
      {
        *result = 2;
        return;
      }
    }
    {
      typedef void (MainWindow::*_t)();
      if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&MainWindow::queueStopEmulationRequest))
      {
        *result = 3;
        return;
      }
    }
    {
      typedef void (MainWindow::*_t)();
      if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&MainWindow::queueSingleStep))
      {
        *result = 4;
        return;
      }
    }
    {
      typedef void (MainWindow::*_t)();
      if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&MainWindow::queueFrameStep))
      {
        *result = 5;
        return;
      }
    }
  }
}

const QMetaObject QtFrontend::MainWindow::staticMetaObject = {
  {&QMainWindow::staticMetaObject, qt_meta_stringdata_QtFrontend__MainWindow.data, qt_meta_data_QtFrontend__MainWindow,
   qt_static_metacall, Q_NULLPTR, Q_NULLPTR}};

const QMetaObject* QtFrontend::MainWindow::metaObject() const
{
  return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void* QtFrontend::MainWindow::qt_metacast(const char* _clname)
{
  if (!_clname)
    return Q_NULLPTR;
  if (!strcmp(_clname, qt_meta_stringdata_QtFrontend__MainWindow.stringdata0))
    return static_cast<void*>(const_cast<MainWindow*>(this));
  return QMainWindow::qt_metacast(_clname);
}

int QtFrontend::MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void** _a)
{
  _id = QMainWindow::qt_metacall(_c, _id, _a);
  if (_id < 0)
    return _id;
  if (_c == QMetaObject::InvokeMetaMethod)
  {
    if (_id < 17)
      qt_static_metacall(this, _c, _id, _a);
    _id -= 17;
  }
  else if (_c == QMetaObject::RegisterMethodArgumentMetaType)
  {
    if (_id < 17)
      *reinterpret_cast<int*>(_a[0]) = -1;
    _id -= 17;
  }
  return _id;
}

// SIGNAL 0
void QtFrontend::MainWindow::queueStartEmulation(QString _t1)
{
  void* _a[] = {Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1))};
  QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtFrontend::MainWindow::queueSetControllerButtonState(int _t1, int _t2, bool _t3)
{
  void* _a[] = {Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)),
                const_cast<void*>(reinterpret_cast<const void*>(&_t2)),
                const_cast<void*>(reinterpret_cast<const void*>(&_t3))};
  QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtFrontend::MainWindow::queuePauseEmulationRequest(bool _t1)
{
  void* _a[] = {Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1))};
  QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtFrontend::MainWindow::queueStopEmulationRequest()
{
  QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}

// SIGNAL 4
void QtFrontend::MainWindow::queueSingleStep()
{
  QMetaObject::activate(this, &staticMetaObject, 4, Q_NULLPTR);
}

// SIGNAL 5
void QtFrontend::MainWindow::queueFrameStep()
{
  QMetaObject::activate(this, &staticMetaObject, 5, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
