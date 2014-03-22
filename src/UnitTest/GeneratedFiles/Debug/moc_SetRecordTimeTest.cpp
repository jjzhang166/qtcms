/****************************************************************************
** Meta object code from reading C++ file 'SetRecordTimeTest.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SetRecordTimeTest.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SetRecordTimeTest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SetRecordTimeTest[] = {

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
      19,   18,   18,   18, 0x08,
      40,   18,   18,   18, 0x08,
      61,   18,   18,   18, 0x08,
      82,   18,   18,   18, 0x08,
     103,   18,   18,   18, 0x08,
     124,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SetRecordTimeTest[] = {
    "SetRecordTimeTest\0\0SetRecordTimeTest1()\0"
    "SetRecordTimeTest2()\0SetRecordTimeTest3()\0"
    "SetRecordTimeTest4()\0SetRecordTimeTest5()\0"
    "SetRecordTimeTest6()\0"
};

void SetRecordTimeTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SetRecordTimeTest *_t = static_cast<SetRecordTimeTest *>(_o);
        switch (_id) {
        case 0: _t->SetRecordTimeTest1(); break;
        case 1: _t->SetRecordTimeTest2(); break;
        case 2: _t->SetRecordTimeTest3(); break;
        case 3: _t->SetRecordTimeTest4(); break;
        case 4: _t->SetRecordTimeTest5(); break;
        case 5: _t->SetRecordTimeTest6(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData SetRecordTimeTest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SetRecordTimeTest::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SetRecordTimeTest,
      qt_meta_data_SetRecordTimeTest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SetRecordTimeTest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SetRecordTimeTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SetRecordTimeTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SetRecordTimeTest))
        return static_cast<void*>(const_cast< SetRecordTimeTest*>(this));
    return QObject::qt_metacast(_clname);
}

int SetRecordTimeTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
