/****************************************************************************
** Meta object code from reading C++ file 'previewactivity.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../previewactivity.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'previewactivity.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_previewactivity[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x0a,
      35,   16,   16,   16, 0x0a,
      51,   16,   16,   16, 0x0a,
      69,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_previewactivity[] = {
    "previewactivity\0\0OnTopActDbClick()\0"
    "OnCloseWindow()\0OnMaxsizeWindow()\0"
    "OnMinsizeWindow()\0"
};

void previewactivity::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        previewactivity *_t = static_cast<previewactivity *>(_o);
        switch (_id) {
        case 0: _t->OnTopActDbClick(); break;
        case 1: _t->OnCloseWindow(); break;
        case 2: _t->OnMaxsizeWindow(); break;
        case 3: _t->OnMinsizeWindow(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData previewactivity::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject previewactivity::staticMetaObject = {
    { &QWebUiFWBase::staticMetaObject, qt_meta_stringdata_previewactivity,
      qt_meta_data_previewactivity, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &previewactivity::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *previewactivity::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *previewactivity::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_previewactivity))
        return static_cast<void*>(const_cast< previewactivity*>(this));
    if (!strcmp(_clname, "IActivities"))
        return static_cast< IActivities*>(const_cast< previewactivity*>(this));
    return QWebUiFWBase::qt_metacast(_clname);
}

int previewactivity::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWebUiFWBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
