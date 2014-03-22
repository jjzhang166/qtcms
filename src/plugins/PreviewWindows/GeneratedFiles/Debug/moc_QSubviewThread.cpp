/****************************************************************************
** Meta object code from reading C++ file 'QSubviewThread.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../QSubviewThread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QSubviewThread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QSubviewThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0a,
      34,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QSubviewThread[] = {
    "QSubviewThread\0\0OpenCameraInWnd()\0"
    "CloseAll()\0"
};

void QSubviewThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QSubviewThread *_t = static_cast<QSubviewThread *>(_o);
        switch (_id) {
        case 0: _t->OpenCameraInWnd(); break;
        case 1: _t->CloseAll(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData QSubviewThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QSubviewThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_QSubviewThread,
      qt_meta_data_QSubviewThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QSubviewThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QSubviewThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QSubviewThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QSubviewThread))
        return static_cast<void*>(const_cast< QSubviewThread*>(this));
    return QThread::qt_metacast(_clname);
}

int QSubviewThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
