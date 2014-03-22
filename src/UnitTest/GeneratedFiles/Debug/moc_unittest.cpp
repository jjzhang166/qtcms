/****************************************************************************
** Meta object code from reading C++ file 'unittest.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../unittest.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'unittest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_UnitTest[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x08,
      22,    9,    9,    9, 0x08,
      34,    9,    9,    9, 0x08,
      46,    9,    9,    9, 0x08,
      58,    9,    9,    9, 0x08,
      70,    9,    9,    9, 0x08,
      82,    9,    9,    9, 0x08,
      94,    9,    9,    9, 0x08,
     106,    9,    9,    9, 0x08,
     118,    9,    9,    9, 0x08,
     131,    9,    9,    9, 0x08,
     144,    9,    9,    9, 0x08,
     157,    9,    9,    9, 0x08,
     170,    9,    9,    9, 0x08,
     183,    9,    9,    9, 0x08,
     196,    9,    9,    9, 0x08,
     209,    9,    9,    9, 0x08,
     222,    9,    9,    9, 0x08,
     235,    9,    9,    9, 0x08,
     248,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_UnitTest[] = {
    "UnitTest\0\0UserCase1()\0UserCase2()\0"
    "UserCase3()\0UserCase4()\0UserCase5()\0"
    "UserCase6()\0UserCase7()\0UserCase8()\0"
    "UserCase9()\0UserCase10()\0UserCase11()\0"
    "UserCase12()\0UserCase13()\0UserCase14()\0"
    "UserCase15()\0UserCase16()\0UserCase17()\0"
    "UserCase18()\0UserCase19()\0UserCase20()\0"
};

void UnitTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        UnitTest *_t = static_cast<UnitTest *>(_o);
        switch (_id) {
        case 0: _t->UserCase1(); break;
        case 1: _t->UserCase2(); break;
        case 2: _t->UserCase3(); break;
        case 3: _t->UserCase4(); break;
        case 4: _t->UserCase5(); break;
        case 5: _t->UserCase6(); break;
        case 6: _t->UserCase7(); break;
        case 7: _t->UserCase8(); break;
        case 8: _t->UserCase9(); break;
        case 9: _t->UserCase10(); break;
        case 10: _t->UserCase11(); break;
        case 11: _t->UserCase12(); break;
        case 12: _t->UserCase13(); break;
        case 13: _t->UserCase14(); break;
        case 14: _t->UserCase15(); break;
        case 15: _t->UserCase16(); break;
        case 16: _t->UserCase17(); break;
        case 17: _t->UserCase18(); break;
        case 18: _t->UserCase19(); break;
        case 19: _t->UserCase20(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData UnitTest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject UnitTest::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_UnitTest,
      qt_meta_data_UnitTest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &UnitTest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *UnitTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *UnitTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_UnitTest))
        return static_cast<void*>(const_cast< UnitTest*>(this));
    return QObject::qt_metacast(_clname);
}

int UnitTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
