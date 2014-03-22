/****************************************************************************
** Meta object code from reading C++ file 'Device_Test.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Device_Test.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Device_Test.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Device_Test[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x08,
      31,   12,   12,   12, 0x08,
      49,   12,   12,   12, 0x08,
      67,   12,   12,   12, 0x08,
      85,   12,   12,   12, 0x08,
     103,   12,   12,   12, 0x08,
     121,   12,   12,   12, 0x08,
     139,   12,   12,   12, 0x08,
     157,   12,   12,   12, 0x08,
     175,   12,   12,   12, 0x08,
     194,   12,   12,   12, 0x08,
     213,   12,   12,   12, 0x08,
     232,   12,   12,   12, 0x08,
     251,   12,   12,   12, 0x08,
     270,   12,   12,   12, 0x08,
     289,   12,   12,   12, 0x08,
     308,   12,   12,   12, 0x08,
     327,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Device_Test[] = {
    "Device_Test\0\0DeviceTestCase1()\0"
    "DeviceTestCase2()\0DeviceTestCase3()\0"
    "DeviceTestCase4()\0DeviceTestCase5()\0"
    "DeviceTestCase6()\0DeviceTestCase7()\0"
    "DeviceTestCase8()\0DeviceTestCase9()\0"
    "DeviceTestCase10()\0DeviceTestCase11()\0"
    "DeviceTestCase12()\0DeviceTestCase13()\0"
    "DeviceTestCase14()\0DeviceTestCase15()\0"
    "DeviceTestCase16()\0DeviceTestCase17()\0"
    "DeviceTestCase18()\0"
};

void Device_Test::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Device_Test *_t = static_cast<Device_Test *>(_o);
        switch (_id) {
        case 0: _t->DeviceTestCase1(); break;
        case 1: _t->DeviceTestCase2(); break;
        case 2: _t->DeviceTestCase3(); break;
        case 3: _t->DeviceTestCase4(); break;
        case 4: _t->DeviceTestCase5(); break;
        case 5: _t->DeviceTestCase6(); break;
        case 6: _t->DeviceTestCase7(); break;
        case 7: _t->DeviceTestCase8(); break;
        case 8: _t->DeviceTestCase9(); break;
        case 9: _t->DeviceTestCase10(); break;
        case 10: _t->DeviceTestCase11(); break;
        case 11: _t->DeviceTestCase12(); break;
        case 12: _t->DeviceTestCase13(); break;
        case 13: _t->DeviceTestCase14(); break;
        case 14: _t->DeviceTestCase15(); break;
        case 15: _t->DeviceTestCase16(); break;
        case 16: _t->DeviceTestCase17(); break;
        case 17: _t->DeviceTestCase18(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Device_Test::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Device_Test::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Device_Test,
      qt_meta_data_Device_Test, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Device_Test::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Device_Test::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Device_Test::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Device_Test))
        return static_cast<void*>(const_cast< Device_Test*>(this));
    return QObject::qt_metacast(_clname);
}

int Device_Test::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
