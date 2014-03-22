/****************************************************************************
** Meta object code from reading C++ file 'area_test.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../area_test.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'area_test.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_area_test[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x08,
      26,   10,   10,   10, 0x08,
      48,   10,   10,   10, 0x08,
      72,   10,   10,   10, 0x08,
      91,   10,   10,   10, 0x08,
     114,   10,   10,   10, 0x08,
     135,   10,   10,   10, 0x08,
     155,   10,   10,   10, 0x08,
     174,   10,   10,   10, 0x08,
     192,   10,   10,   10, 0x08,
     210,   10,   10,   10, 0x08,
     229,   10,   10,   10, 0x08,
     252,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_area_test[] = {
    "area_test\0\0AddArea_test()\0"
    "RemoveAreaById_test()\0RemoveAreaByName_test()\0"
    "SetAreaName_test()\0IsAreaNameExist_test()\0"
    "IsAreaIdExist_test()\0GetAreaCount_test()\0"
    "GetAreaList_test()\0GetSubArea_test()\0"
    "GetAreaPid_test()\0GetAreaName_test()\0"
    "GetAreaInfo_int_test()\0"
    "GetAreaInfo_qvariantmap_test()\0"
};

void area_test::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        area_test *_t = static_cast<area_test *>(_o);
        switch (_id) {
        case 0: _t->AddArea_test(); break;
        case 1: _t->RemoveAreaById_test(); break;
        case 2: _t->RemoveAreaByName_test(); break;
        case 3: _t->SetAreaName_test(); break;
        case 4: _t->IsAreaNameExist_test(); break;
        case 5: _t->IsAreaIdExist_test(); break;
        case 6: _t->GetAreaCount_test(); break;
        case 7: _t->GetAreaList_test(); break;
        case 8: _t->GetSubArea_test(); break;
        case 9: _t->GetAreaPid_test(); break;
        case 10: _t->GetAreaName_test(); break;
        case 11: _t->GetAreaInfo_int_test(); break;
        case 12: _t->GetAreaInfo_qvariantmap_test(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData area_test::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject area_test::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_area_test,
      qt_meta_data_area_test, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &area_test::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *area_test::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *area_test::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_area_test))
        return static_cast<void*>(const_cast< area_test*>(this));
    return QObject::qt_metacast(_clname);
}

int area_test::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
