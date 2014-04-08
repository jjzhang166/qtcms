/****************************************************************************
** Meta object code from reading C++ file 'RecordPlayerView.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../RecordPlayerView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RecordPlayerView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RecordPlayerView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   18,   17,   17, 0x05,
      60,   17,   17,   17, 0x05,
      90,   17,   17,   17, 0x05,
     108,   18,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
     151,   17,   17,   17, 0x0a,
     165,   17,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_RecordPlayerView[] = {
    "RecordPlayerView\0\0,\0"
    "mouseDoubleClick(QWidget*,QMouseEvent*)\0"
    "SetCurrentWindSignl(QWidget*)\0"
    "RMousePressMenu()\0"
    "ChangeAudioHint(QString,RecordPlayerView*)\0"
    "OnOpenAudio()\0OnRMousePressMenu()\0"
};

void RecordPlayerView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RecordPlayerView *_t = static_cast<RecordPlayerView *>(_o);
        switch (_id) {
        case 0: _t->mouseDoubleClick((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QMouseEvent*(*)>(_a[2]))); break;
        case 1: _t->SetCurrentWindSignl((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 2: _t->RMousePressMenu(); break;
        case 3: _t->ChangeAudioHint((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< RecordPlayerView*(*)>(_a[2]))); break;
        case 4: _t->OnOpenAudio(); break;
        case 5: _t->OnRMousePressMenu(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData RecordPlayerView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RecordPlayerView::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_RecordPlayerView,
      qt_meta_data_RecordPlayerView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RecordPlayerView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RecordPlayerView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RecordPlayerView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RecordPlayerView))
        return static_cast<void*>(const_cast< RecordPlayerView*>(this));
    return QWidget::qt_metacast(_clname);
}

int RecordPlayerView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void RecordPlayerView::mouseDoubleClick(QWidget * _t1, QMouseEvent * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void RecordPlayerView::SetCurrentWindSignl(QWidget * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void RecordPlayerView::RMousePressMenu()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void RecordPlayerView::ChangeAudioHint(QString _t1, RecordPlayerView * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
