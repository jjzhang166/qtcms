/****************************************************************************
** Meta object code from reading C++ file 'autoSearchDeviceWindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../autoSearchDeviceWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'autoSearchDeviceWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_autoSearchDeviceWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
      35,   23,   23,   23, 0x0a,
      48,   44,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_autoSearchDeviceWindow[] = {
    "autoSearchDeviceWindow\0\0sgCancel()\0"
    "cancel()\0bOk\0OnLoad(bool)\0"
};

void autoSearchDeviceWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        autoSearchDeviceWindow *_t = static_cast<autoSearchDeviceWindow *>(_o);
        switch (_id) {
        case 0: _t->sgCancel(); break;
        case 1: _t->cancel(); break;
        case 2: _t->OnLoad((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData autoSearchDeviceWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject autoSearchDeviceWindow::staticMetaObject = {
    { &QWebView::staticMetaObject, qt_meta_stringdata_autoSearchDeviceWindow,
      qt_meta_data_autoSearchDeviceWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &autoSearchDeviceWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *autoSearchDeviceWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *autoSearchDeviceWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_autoSearchDeviceWindow))
        return static_cast<void*>(const_cast< autoSearchDeviceWindow*>(this));
    return QWebView::qt_metacast(_clname);
}

int autoSearchDeviceWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWebView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void autoSearchDeviceWindow::sgCancel()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
