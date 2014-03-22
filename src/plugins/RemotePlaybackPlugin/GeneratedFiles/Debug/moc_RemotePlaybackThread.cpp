/****************************************************************************
** Meta object code from reading C++ file 'RemotePlaybackThread.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../RemotePlaybackThread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RemotePlaybackThread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RemotePlaybackThread[] = {

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
      22,   21,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      81,   47,   21,   21, 0x0a,
     155,  130,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_RemotePlaybackThread[] = {
    "RemotePlaybackThread\0\0finishSearchRecFileSig()\0"
    "nChannel,nTypes,startTime,endTime\0"
    "startSearchRecFileSlots(int,int,QString,QString)\0"
    "nTypes,startTime,endTime\0"
    "GroupPlaySlots(int,QString,QString)\0"
};

void RemotePlaybackThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RemotePlaybackThread *_t = static_cast<RemotePlaybackThread *>(_o);
        switch (_id) {
        case 0: _t->finishSearchRecFileSig(); break;
        case 1: _t->startSearchRecFileSlots((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4]))); break;
        case 2: _t->GroupPlaySlots((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData RemotePlaybackThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RemotePlaybackThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_RemotePlaybackThread,
      qt_meta_data_RemotePlaybackThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RemotePlaybackThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RemotePlaybackThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RemotePlaybackThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RemotePlaybackThread))
        return static_cast<void*>(const_cast< RemotePlaybackThread*>(this));
    return QThread::qt_metacast(_clname);
}

int RemotePlaybackThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void RemotePlaybackThread::finishSearchRecFileSig()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
