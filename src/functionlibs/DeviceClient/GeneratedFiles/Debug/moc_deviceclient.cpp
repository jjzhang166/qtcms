/****************************************************************************
** Meta object code from reading C++ file 'deviceclient.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../deviceclient.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'deviceclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DeviceClient[] = {

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
      14,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      48,   39,   13,   13, 0x08,
      93,   79,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DeviceClient[] = {
    "DeviceClient\0\0TerminateConnectSignal()\0"
    "options,\0action(QString,BufferManager*)\0"
    "persent,pBuff\0bufferStatus(int,BufferManager*)\0"
};

void DeviceClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DeviceClient *_t = static_cast<DeviceClient *>(_o);
        switch (_id) {
        case 0: _t->TerminateConnectSignal(); break;
        case 1: _t->action((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< BufferManager*(*)>(_a[2]))); break;
        case 2: _t->bufferStatus((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< BufferManager*(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DeviceClient::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DeviceClient::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_DeviceClient,
      qt_meta_data_DeviceClient, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DeviceClient::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DeviceClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DeviceClient::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DeviceClient))
        return static_cast<void*>(const_cast< DeviceClient*>(this));
    if (!strcmp(_clname, "IDeviceClient"))
        return static_cast< IDeviceClient*>(const_cast< DeviceClient*>(this));
    if (!strcmp(_clname, "IEventRegister"))
        return static_cast< IEventRegister*>(const_cast< DeviceClient*>(this));
    if (!strcmp(_clname, "IDeviceSearchRecord"))
        return static_cast< IDeviceSearchRecord*>(const_cast< DeviceClient*>(this));
    if (!strcmp(_clname, "IDeviceGroupRemotePlayback"))
        return static_cast< IDeviceGroupRemotePlayback*>(const_cast< DeviceClient*>(this));
    if (!strcmp(_clname, "IRemoteBackup"))
        return static_cast< IRemoteBackup*>(const_cast< DeviceClient*>(this));
    return QThread::qt_metacast(_clname);
}

int DeviceClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void DeviceClient::TerminateConnectSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
