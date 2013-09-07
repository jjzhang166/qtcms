/****************************************************************************
** Meta object code from reading C++ file 'qffmpeg.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qffmpeg.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qffmpeg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qffmpeg[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x0a,
      16,    8,    8,    8, 0x0a,
      24,    8,    8,    8, 0x0a,
      31,    8,    8,    8, 0x0a,
      51,   38,    8,    8, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_qffmpeg[] = {
    "qffmpeg\0\0Play()\0Pause()\0Stop()\0Open()\0"
    "sEvent,sProc\0AddEventProc(QString,QString)\0"
};

void qffmpeg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        qffmpeg *_t = static_cast<qffmpeg *>(_o);
        switch (_id) {
        case 0: _t->Play(); break;
        case 1: _t->Pause(); break;
        case 2: _t->Stop(); break;
        case 3: _t->Open(); break;
        case 4: _t->AddEventProc((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData qffmpeg::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject qffmpeg::staticMetaObject = {
    { &QGroupBox::staticMetaObject, qt_meta_stringdata_qffmpeg,
      qt_meta_data_qffmpeg, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qffmpeg::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qffmpeg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qffmpeg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qffmpeg))
        return static_cast<void*>(const_cast< qffmpeg*>(this));
    if (!strcmp(_clname, "QWebPluginFWBase"))
        return static_cast< QWebPluginFWBase*>(const_cast< qffmpeg*>(this));
    return QGroupBox::qt_metacast(_clname);
}

int qffmpeg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGroupBox::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
