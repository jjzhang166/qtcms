/****************************************************************************
** Meta object code from reading C++ file 'LocalRecordSearchTest.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../LocalRecordSearchTest.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LocalRecordSearchTest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LocalRecordSearchTest[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      23,   22,   22,   22, 0x08,
      35,   22,   22,   22, 0x08,
      47,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_LocalRecordSearchTest[] = {
    "LocalRecordSearchTest\0\0TestCase1()\0"
    "TestCase2()\0TestCase3()\0"
};

void LocalRecordSearchTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LocalRecordSearchTest *_t = static_cast<LocalRecordSearchTest *>(_o);
        switch (_id) {
        case 0: _t->TestCase1(); break;
        case 1: _t->TestCase2(); break;
        case 2: _t->TestCase3(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData LocalRecordSearchTest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LocalRecordSearchTest::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_LocalRecordSearchTest,
      qt_meta_data_LocalRecordSearchTest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LocalRecordSearchTest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LocalRecordSearchTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LocalRecordSearchTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LocalRecordSearchTest))
        return static_cast<void*>(const_cast< LocalRecordSearchTest*>(this));
    return QObject::qt_metacast(_clname);
}

int LocalRecordSearchTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
