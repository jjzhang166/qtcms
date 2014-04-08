/****************************************************************************
** Meta object code from reading C++ file 'HiChipTest.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../HiChipTest.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HiChipTest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HiChipUnitTest[] = {

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
      16,   15,   15,   15, 0x08,
      34,   15,   15,   15, 0x08,
      52,   15,   15,   15, 0x08,
      70,   15,   15,   15, 0x08,
      88,   15,   15,   15, 0x08,
     106,   15,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_HiChipUnitTest[] = {
    "HiChipUnitTest\0\0DeviceTestCase1()\0"
    "DeviceTestCase2()\0DeviceTestCase3()\0"
    "DeviceTestCase4()\0DeviceTestCase5()\0"
    "DeviceTestCase6()\0"
};

void HiChipUnitTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        HiChipUnitTest *_t = static_cast<HiChipUnitTest *>(_o);
        switch (_id) {
        case 0: _t->DeviceTestCase1(); break;
        case 1: _t->DeviceTestCase2(); break;
        case 2: _t->DeviceTestCase3(); break;
        case 3: _t->DeviceTestCase4(); break;
        case 4: _t->DeviceTestCase5(); break;
        case 5: _t->DeviceTestCase6(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData HiChipUnitTest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject HiChipUnitTest::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_HiChipUnitTest,
      qt_meta_data_HiChipUnitTest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HiChipUnitTest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HiChipUnitTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HiChipUnitTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HiChipUnitTest))
        return static_cast<void*>(const_cast< HiChipUnitTest*>(this));
    return QObject::qt_metacast(_clname);
}

int HiChipUnitTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
