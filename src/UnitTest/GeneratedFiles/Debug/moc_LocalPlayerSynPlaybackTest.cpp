/****************************************************************************
** Meta object code from reading C++ file 'LocalPlayerSynPlaybackTest.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../LocalPlayerSynPlaybackTest.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LocalPlayerSynPlaybackTest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LocalPlayerSynPlaybackTest[] = {

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
      28,   27,   27,   27, 0x08,
      40,   27,   27,   27, 0x08,
      52,   27,   27,   27, 0x08,
      64,   27,   27,   27, 0x08,
      76,   27,   27,   27, 0x08,
      88,   27,   27,   27, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_LocalPlayerSynPlaybackTest[] = {
    "LocalPlayerSynPlaybackTest\0\0TestCase1()\0"
    "TestCase2()\0TestCase3()\0TestCase4()\0"
    "TestCase5()\0TestCase6()\0"
};

void LocalPlayerSynPlaybackTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LocalPlayerSynPlaybackTest *_t = static_cast<LocalPlayerSynPlaybackTest *>(_o);
        switch (_id) {
        case 0: _t->TestCase1(); break;
        case 1: _t->TestCase2(); break;
        case 2: _t->TestCase3(); break;
        case 3: _t->TestCase4(); break;
        case 4: _t->TestCase5(); break;
        case 5: _t->TestCase6(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData LocalPlayerSynPlaybackTest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LocalPlayerSynPlaybackTest::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_LocalPlayerSynPlaybackTest,
      qt_meta_data_LocalPlayerSynPlaybackTest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LocalPlayerSynPlaybackTest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LocalPlayerSynPlaybackTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LocalPlayerSynPlaybackTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LocalPlayerSynPlaybackTest))
        return static_cast<void*>(const_cast< LocalPlayerSynPlaybackTest*>(this));
    return QWidget::qt_metacast(_clname);
}

int LocalPlayerSynPlaybackTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
