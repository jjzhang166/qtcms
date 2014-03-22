/****************************************************************************
** Meta object code from reading C++ file 'Channel_Test.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Channel_Test.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Channel_Test.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Channel_Test[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x08,
      25,   13,   13,   13, 0x08,
      36,   13,   13,   13, 0x08,
      47,   13,   13,   13, 0x08,
      58,   13,   13,   13, 0x08,
      69,   13,   13,   13, 0x08,
      80,   13,   13,   13, 0x08,
      91,   13,   13,   13, 0x08,
     102,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Channel_Test[] = {
    "Channel_Test\0\0ChlCase1()\0ChlCase2()\0"
    "ChlCase3()\0ChlCase4()\0ChlCase5()\0"
    "ChlCase6()\0ChlCase7()\0ChlCase8()\0"
    "ChlCase9()\0"
};

void Channel_Test::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Channel_Test *_t = static_cast<Channel_Test *>(_o);
        switch (_id) {
        case 0: _t->ChlCase1(); break;
        case 1: _t->ChlCase2(); break;
        case 2: _t->ChlCase3(); break;
        case 3: _t->ChlCase4(); break;
        case 4: _t->ChlCase5(); break;
        case 5: _t->ChlCase6(); break;
        case 6: _t->ChlCase7(); break;
        case 7: _t->ChlCase8(); break;
        case 8: _t->ChlCase9(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Channel_Test::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Channel_Test::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Channel_Test,
      qt_meta_data_Channel_Test, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Channel_Test::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Channel_Test::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Channel_Test::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Channel_Test))
        return static_cast<void*>(const_cast< Channel_Test*>(this));
    return QObject::qt_metacast(_clname);
}

int Channel_Test::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
