/****************************************************************************
** Meta object code from reading C++ file 'RecorderTest.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../RecorderTest.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RecorderTest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RecorderTest[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x08,
      30,   13,   13,   13, 0x08,
      46,   13,   13,   13, 0x08,
      62,   13,   13,   13, 0x08,
      78,   13,   13,   13, 0x08,
      94,   13,   13,   13, 0x08,
     110,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_RecorderTest[] = {
    "RecorderTest\0\0RecorderTest1()\0"
    "RecorderTest2()\0RecorderTest3()\0"
    "RecorderTest4()\0RecorderTest5()\0"
    "RecorderTest6()\0RecorderTest7()\0"
};

void RecorderTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RecorderTest *_t = static_cast<RecorderTest *>(_o);
        switch (_id) {
        case 0: _t->RecorderTest1(); break;
        case 1: _t->RecorderTest2(); break;
        case 2: _t->RecorderTest3(); break;
        case 3: _t->RecorderTest4(); break;
        case 4: _t->RecorderTest5(); break;
        case 5: _t->RecorderTest6(); break;
        case 6: _t->RecorderTest7(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData RecorderTest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RecorderTest::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_RecorderTest,
      qt_meta_data_RecorderTest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RecorderTest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RecorderTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RecorderTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RecorderTest))
        return static_cast<void*>(const_cast< RecorderTest*>(this));
    return QObject::qt_metacast(_clname);
}

int RecorderTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
