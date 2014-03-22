/****************************************************************************
** Meta object code from reading C++ file 'BufferManager.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../BufferManager.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BufferManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BufferManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      30,   15,   14,   14, 0x05,
      77,   61,   14,   14, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_BufferManager[] = {
    "BufferManager\0\0option,pBuffer\0"
    "action(QString,BufferManager*)\0"
    "persent,pBuffer\0bufferStatus(int,BufferManager*)\0"
};

void BufferManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        BufferManager *_t = static_cast<BufferManager *>(_o);
        switch (_id) {
        case 0: _t->action((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< BufferManager*(*)>(_a[2]))); break;
        case 1: _t->bufferStatus((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< BufferManager*(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData BufferManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject BufferManager::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_BufferManager,
      qt_meta_data_BufferManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BufferManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BufferManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BufferManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BufferManager))
        return static_cast<void*>(const_cast< BufferManager*>(this));
    return QThread::qt_metacast(_clname);
}

int BufferManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void BufferManager::action(QString _t1, BufferManager * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void BufferManager::bufferStatus(int _t1, BufferManager * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
