/****************************************************************************
** Meta object code from reading C++ file 'RemotePlaybackObject.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../RemotePlaybackObject.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RemotePlaybackObject.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RemotePlaybackObject[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      56,   22,   21,   21, 0x05,
     132,  107,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
     170,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_RemotePlaybackObject[] = {
    "RemotePlaybackObject\0\0"
    "nChannel,nTypes,startTime,endTime\0"
    "startSearchRecFileSignals(int,int,QString,QString)\0"
    "nTypes,startTime,endTime\0"
    "GroupPlaySignals(int,QString,QString)\0"
    "finishSearchRecFileSlots()\0"
};

void RemotePlaybackObject::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RemotePlaybackObject *_t = static_cast<RemotePlaybackObject *>(_o);
        switch (_id) {
        case 0: _t->startSearchRecFileSignals((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4]))); break;
        case 1: _t->GroupPlaySignals((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 2: _t->finishSearchRecFileSlots(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData RemotePlaybackObject::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RemotePlaybackObject::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_RemotePlaybackObject,
      qt_meta_data_RemotePlaybackObject, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RemotePlaybackObject::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RemotePlaybackObject::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RemotePlaybackObject::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RemotePlaybackObject))
        return static_cast<void*>(const_cast< RemotePlaybackObject*>(this));
    return QThread::qt_metacast(_clname);
}

int RemotePlaybackObject::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
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
void RemotePlaybackObject::startSearchRecFileSignals(int _t1, int _t2, const QString & _t3, const QString & _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void RemotePlaybackObject::GroupPlaySignals(int _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
