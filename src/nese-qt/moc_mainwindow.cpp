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
  QByteArrayData data[10];
  char stringdata0[170];
};
#define QT_MOC_LITERAL(idx, ofs, len)                                                                                  \
  Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(                                                             \
    len,                                                                                                               \
    qptrdiff(offsetof(qt_meta_stringdata_QtFrontend__MainWindow_t, stringdata0) + ofs - idx * sizeof(QByteArrayData)))
static const qt_meta_stringdata_QtFrontend__MainWindow_t qt_meta_stringdata_QtFrontend__MainWindow = {
  {
    QT_MOC_LITERAL(0, 0, 22),   // "QtFrontend::MainWindow"
    QT_MOC_LITERAL(1, 23, 20),  // "onPowerActionToggled"
    QT_MOC_LITERAL(2, 44, 0),   // ""
    QT_MOC_LITERAL(3, 45, 8),   // "selected"
    QT_MOC_LITERAL(4, 54, 22),  // "onResetActionTriggered"
    QT_MOC_LITERAL(5, 77, 22),  // "onAboutActionTriggered"
    QT_MOC_LITERAL(6, 100, 25), // "onDisplayWidgetKeyPressed"
    QT_MOC_LITERAL(7, 126, 10), // "QKeyEvent*"
    QT_MOC_LITERAL(8, 137, 5),  // "event"
    QT_MOC_LITERAL(9, 143, 26)  // "onDisplayWidgetKeyReleased"

  },
  "QtFrontend::MainWindow\0onPowerActionToggled\0"
  "\0selected\0onResetActionTriggered\0"
  "onAboutActionTriggered\0onDisplayWidgetKeyPressed\0"
  "QKeyEvent*\0event\0onDisplayWidgetKeyReleased"};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtFrontend__MainWindow[] = {

  // content:
  7,     // revision
  0,     // classname
  0, 0,  // classinfo
  5, 14, // methods
  0, 0,  // properties
  0, 0,  // enums/sets
  0, 0,  // constructors
  0,     // flags
  0,     // signalCount

  // slots: name, argc, parameters, tag, flags
  1, 1, 39, 2, 0x0a /* Public */, 4, 0, 42, 2, 0x0a /* Public */, 5, 0, 43, 2, 0x0a /* Public */, 6, 1, 44, 2,
  0x0a /* Public */, 9, 1, 47, 2, 0x0a /* Public */,

  // slots: parameters
  QMetaType::Void, QMetaType::Bool, 3, QMetaType::Void, QMetaType::Void, QMetaType::Void, 0x80000000 | 7, 8,
  QMetaType::Void, 0x80000000 | 7, 8,

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
        _t->onPowerActionToggled((*reinterpret_cast<bool(*)>(_a[1])));
        break;
      case 1:
        _t->onResetActionTriggered();
        break;
      case 2:
        _t->onAboutActionTriggered();
        break;
      case 3:
        _t->onDisplayWidgetKeyPressed((*reinterpret_cast<QKeyEvent*(*)>(_a[1])));
        break;
      case 4:
        _t->onDisplayWidgetKeyReleased((*reinterpret_cast<QKeyEvent*(*)>(_a[1])));
        break;
      default:;
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
    if (_id < 5)
      qt_static_metacall(this, _c, _id, _a);
    _id -= 5;
  }
  else if (_c == QMetaObject::RegisterMethodArgumentMetaType)
  {
    if (_id < 5)
      *reinterpret_cast<int*>(_a[0]) = -1;
    _id -= 5;
  }
  return _id;
}
QT_END_MOC_NAMESPACE
