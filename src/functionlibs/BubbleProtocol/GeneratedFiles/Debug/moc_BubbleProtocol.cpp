/****************************************************************************
** Meta object code from reading C++ file 'BubbleProtocol.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../BubbleProtocol.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BubbleProtocol.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BubbleProtocol[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,
      32,   15,   15,   15, 0x05,
      53,   47,   15,   15, 0x05,
      93,   80,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
     131,   15,   15,   15, 0x08,
     145,   15,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_BubbleProtocol[] = {
    "BubbleProtocol\0\0sigQuitThread()\0"
    "sigEndStream()\0block\0sigWriteSocket(QByteArray)\0"
    "address,port\0sigChildThreadToConn(QString,quint16)\0"
    "finishReply()\0sendHeartBeat()\0"
};

void BubbleProtocol::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        BubbleProtocol *_t = static_cast<BubbleProtocol *>(_o);
        switch (_id) {
        case 0: _t->sigQuitThread(); break;
        case 1: _t->sigEndStream(); break;
        case 2: _t->sigWriteSocket((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 3: _t->sigChildThreadToConn((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 4: _t->finishReply(); break;
        case 5: _t->sendHeartBeat(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData BubbleProtocol::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject BubbleProtocol::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_BubbleProtocol,
      qt_meta_data_BubbleProtocol, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BubbleProtocol::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BubbleProtocol::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BubbleProtocol::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BubbleProtocol))
        return static_cast<void*>(const_cast< BubbleProtocol*>(this));
    if (!strcmp(_clname, "IEventRegister"))
        return static_cast< IEventRegister*>(const_cast< BubbleProtocol*>(this));
    if (!strcmp(_clname, "IRemotePreview"))
        return static_cast< IRemotePreview*>(const_cast< BubbleProtocol*>(this));
    if (!strcmp(_clname, "IRemotePlayback"))
        return static_cast< IRemotePlayback*>(const_cast< BubbleProtocol*>(this));
    if (!strcmp(_clname, "IDeviceConnection"))
        return static_cast< IDeviceConnection*>(const_cast< BubbleProtocol*>(this));
    return QObject::qt_metacast(_clname);
}

int BubbleProtocol::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void BubbleProtocol::sigQuitThread()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void BubbleProtocol::sigEndStream()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void BubbleProtocol::sigWriteSocket(QByteArray _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void BubbleProtocol::sigChildThreadToConn(QString _t1, quint16 _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
