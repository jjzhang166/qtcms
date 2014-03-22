/****************************************************************************
** Meta object code from reading C++ file 'StreamProcess.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../StreamProcess.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'StreamProcess.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_StreamProcess[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x0a,
      28,   14,   14,   14, 0x08,
      54,   44,   14,   14, 0x08,
     106,   94,   14,   14, 0x08,
     151,  149,   14,   14, 0x08,
     184,  178,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_StreamProcess[] = {
    "StreamProcess\0\0stopStream()\0receiveStream()\0"
    "sockerror\0showError(QAbstractSocket::SocketError)\0"
    "socketState\0stateChanged(QAbstractSocket::SocketState)\0"
    ",\0conToHost(QString,quint16)\0block\0"
    "socketWrites(QByteArray)\0"
};

void StreamProcess::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        StreamProcess *_t = static_cast<StreamProcess *>(_o);
        switch (_id) {
        case 0: _t->stopStream(); break;
        case 1: _t->receiveStream(); break;
        case 2: _t->showError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 3: _t->stateChanged((*reinterpret_cast< QAbstractSocket::SocketState(*)>(_a[1]))); break;
        case 4: _t->conToHost((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 5: _t->socketWrites((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData StreamProcess::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject StreamProcess::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_StreamProcess,
      qt_meta_data_StreamProcess, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &StreamProcess::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *StreamProcess::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *StreamProcess::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_StreamProcess))
        return static_cast<void*>(const_cast< StreamProcess*>(this));
    return QObject::qt_metacast(_clname);
}

int StreamProcess::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
