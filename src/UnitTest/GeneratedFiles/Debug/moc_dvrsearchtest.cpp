/****************************************************************************
** Meta object code from reading C++ file 'dvrsearchtest.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../dvrsearchtest.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dvrsearchtest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DvrSearchTest[] = {

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
      15,   14,   14,   14, 0x08,
      32,   14,   14,   14, 0x08,
      49,   14,   14,   14, 0x08,
      66,   14,   14,   14, 0x08,
      83,   14,   14,   14, 0x08,
     100,   14,   14,   14, 0x08,
     117,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DvrSearchTest[] = {
    "DvrSearchTest\0\0DvrSearchCase1()\0"
    "DvrSearchCase2()\0DvrSearchCase3()\0"
    "DvrSearchCase4()\0DvrSearchCase5()\0"
    "DvrSearchCase6()\0DvrSearchCase7()\0"
};

void DvrSearchTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DvrSearchTest *_t = static_cast<DvrSearchTest *>(_o);
        switch (_id) {
        case 0: _t->DvrSearchCase1(); break;
        case 1: _t->DvrSearchCase2(); break;
        case 2: _t->DvrSearchCase3(); break;
        case 3: _t->DvrSearchCase4(); break;
        case 4: _t->DvrSearchCase5(); break;
        case 5: _t->DvrSearchCase6(); break;
        case 6: _t->DvrSearchCase7(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData DvrSearchTest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DvrSearchTest::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DvrSearchTest,
      qt_meta_data_DvrSearchTest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DvrSearchTest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DvrSearchTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DvrSearchTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DvrSearchTest))
        return static_cast<void*>(const_cast< DvrSearchTest*>(this));
    return QObject::qt_metacast(_clname);
}

int DvrSearchTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
