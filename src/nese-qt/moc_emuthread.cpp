/****************************************************************************
** Meta object code from reading C++ file 'emuthread.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "emuthread.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'emuthread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QtFrontend__EmuThread_t
{
  QByteArrayData data[19];
  char stringdata0[290];
};
#define QT_MOC_LITERAL(idx, ofs, len)                                                                                  \
  Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(                                                             \
    len,                                                                                                               \
    qptrdiff(offsetof(qt_meta_stringdata_QtFrontend__EmuThread_t, stringdata0) + ofs - idx * sizeof(QByteArrayData)))
static const qt_meta_stringdata_QtFrontend__EmuThread_t qt_meta_stringdata_QtFrontend__EmuThread = {
  {
    QT_MOC_LITERAL(0, 0, 21),    // "QtFrontend::EmuThread"
    QT_MOC_LITERAL(1, 22, 19),   // "emulationErrorEvent"
    QT_MOC_LITERAL(2, 42, 0),    // ""
    QT_MOC_LITERAL(3, 43, 6),    // "Error*"
    QT_MOC_LITERAL(4, 50, 5),    // "error"
    QT_MOC_LITERAL(5, 56, 21),   // "emulationStartedEvent"
    QT_MOC_LITERAL(6, 78, 20),   // "emulationPausedEvent"
    QT_MOC_LITERAL(7, 99, 6),    // "paused"
    QT_MOC_LITERAL(8, 106, 21),  // "emulationStoppedEvent"
    QT_MOC_LITERAL(9, 128, 16),  // "onStartEmulation"
    QT_MOC_LITERAL(10, 145, 18), // "cartridge_filename"
    QT_MOC_LITERAL(11, 164, 25), // "onControllerButtonPressed"
    QT_MOC_LITERAL(12, 190, 3),  // "u32"
    QT_MOC_LITERAL(13, 194, 10), // "controller"
    QT_MOC_LITERAL(14, 205, 12), // "button_index"
    QT_MOC_LITERAL(15, 218, 23), // "onEmulationPauseRequest"
    QT_MOC_LITERAL(16, 242, 22), // "onEmulationStopRequest"
    QT_MOC_LITERAL(17, 265, 12), // "onSingleStep"
    QT_MOC_LITERAL(18, 278, 11)  // "onFrameStep"

  },
  "QtFrontend::EmuThread\0emulationErrorEvent\0"
  "\0Error*\0error\0emulationStartedEvent\0"
  "emulationPausedEvent\0paused\0"
  "emulationStoppedEvent\0onStartEmulation\0"
  "cartridge_filename\0onControllerButtonPressed\0"
  "u32\0controller\0button_index\0"
  "onEmulationPauseRequest\0onEmulationStopRequest\0"
  "onSingleStep\0onFrameStep"};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtFrontend__EmuThread[] = {

  // content:
  7,      // revision
  0,      // classname
  0, 0,   // classinfo
  10, 14, // methods
  0, 0,   // properties
  0, 0,   // enums/sets
  0, 0,   // constructors
  0,      // flags
  4,      // signalCount

  // signals: name, argc, parameters, tag, flags
  1, 1, 64, 2, 0x06 /* Public */, 5, 0, 67, 2, 0x06 /* Public */, 6, 1, 68, 2, 0x06 /* Public */, 8, 0, 71, 2,
  0x06 /* Public */,

  // slots: name, argc, parameters, tag, flags
  9, 1, 72, 2, 0x0a /* Public */, 11, 2, 75, 2, 0x0a /* Public */, 15, 1, 80, 2, 0x0a /* Public */, 16, 0, 83, 2,
  0x0a /* Public */, 17, 0, 84, 2, 0x0a /* Public */, 18, 0, 85, 2, 0x0a /* Public */,

  // signals: parameters
  QMetaType::Void, 0x80000000 | 3, 4, QMetaType::Void, QMetaType::Void, QMetaType::Bool, 7, QMetaType::Void,

  // slots: parameters
  QMetaType::Void, QMetaType::QString, 10, QMetaType::Void, 0x80000000 | 12, 0x80000000 | 12, 13, 14, QMetaType::Void,
  QMetaType::Bool, 7, QMetaType::Void, QMetaType::Void, QMetaType::Void,

  0 // eod
};

void QtFrontend::EmuThread::qt_static_metacall(QObject* _o, QMetaObject::Call _c, int _id, void** _a)
{
  if (_c == QMetaObject::InvokeMetaMethod)
  {
    Q_ASSERT(staticMetaObject.cast(_o));
    EmuThread* _t = static_cast<EmuThread*>(_o);
    Q_UNUSED(_t)
    switch (_id)
    {
      case 0:
        _t->emulationErrorEvent((*reinterpret_cast<Error*(*)>(_a[1])));
        break;
      case 1:
        _t->emulationStartedEvent();
        break;
      case 2:
        _t->emulationPausedEvent((*reinterpret_cast<bool(*)>(_a[1])));
        break;
      case 3:
        _t->emulationStoppedEvent();
        break;
      case 4:
        _t->onStartEmulation((*reinterpret_cast<const QString(*)>(_a[1])));
        break;
      case 5:
        _t->onControllerButtonPressed((*reinterpret_cast<u32(*)>(_a[1])), (*reinterpret_cast<u32(*)>(_a[2])));
        break;
      case 6:
        _t->onEmulationPauseRequest((*reinterpret_cast<bool(*)>(_a[1])));
        break;
      case 7:
        _t->onEmulationStopRequest();
        break;
      case 8:
        _t->onSingleStep();
        break;
      case 9:
        _t->onFrameStep();
        break;
      default:;
    }
  }
  else if (_c == QMetaObject::IndexOfMethod)
  {
    int* result = reinterpret_cast<int*>(_a[0]);
    void** func = reinterpret_cast<void**>(_a[1]);
    {
      typedef void (EmuThread::*_t)(Error*);
      if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&EmuThread::emulationErrorEvent))
      {
        *result = 0;
        return;
      }
    }
    {
      typedef void (EmuThread::*_t)();
      if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&EmuThread::emulationStartedEvent))
      {
        *result = 1;
        return;
      }
    }
    {
      typedef void (EmuThread::*_t)(bool);
      if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&EmuThread::emulationPausedEvent))
      {
        *result = 2;
        return;
      }
    }
    {
      typedef void (EmuThread::*_t)();
      if (*reinterpret_cast<_t*>(func) == static_cast<_t>(&EmuThread::emulationStoppedEvent))
      {
        *result = 3;
        return;
      }
    }
  }
}

const QMetaObject QtFrontend::EmuThread::staticMetaObject = {
  {&QThread::staticMetaObject, qt_meta_stringdata_QtFrontend__EmuThread.data, qt_meta_data_QtFrontend__EmuThread,
   qt_static_metacall, Q_NULLPTR, Q_NULLPTR}};

const QMetaObject* QtFrontend::EmuThread::metaObject() const
{
  return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void* QtFrontend::EmuThread::qt_metacast(const char* _clname)
{
  if (!_clname)
    return Q_NULLPTR;
  if (!strcmp(_clname, qt_meta_stringdata_QtFrontend__EmuThread.stringdata0))
    return static_cast<void*>(const_cast<EmuThread*>(this));
  return QThread::qt_metacast(_clname);
}

int QtFrontend::EmuThread::qt_metacall(QMetaObject::Call _c, int _id, void** _a)
{
  _id = QThread::qt_metacall(_c, _id, _a);
  if (_id < 0)
    return _id;
  if (_c == QMetaObject::InvokeMetaMethod)
  {
    if (_id < 10)
      qt_static_metacall(this, _c, _id, _a);
    _id -= 10;
  }
  else if (_c == QMetaObject::RegisterMethodArgumentMetaType)
  {
    if (_id < 10)
      *reinterpret_cast<int*>(_a[0]) = -1;
    _id -= 10;
  }
  return _id;
}

// SIGNAL 0
void QtFrontend::EmuThread::emulationErrorEvent(Error* _t1)
{
  void* _a[] = {Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1))};
  QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtFrontend::EmuThread::emulationStartedEvent()
{
  QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void QtFrontend::EmuThread::emulationPausedEvent(bool _t1)
{
  void* _a[] = {Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1))};
  QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtFrontend::EmuThread::emulationStoppedEvent()
{
  QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
