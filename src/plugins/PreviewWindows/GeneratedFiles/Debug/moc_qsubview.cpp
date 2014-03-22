/****************************************************************************
** Meta object code from reading C++ file 'qsubview.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qsubview.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qsubview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QSubView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      25,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      14,       // signalCount

 // signals: signature, parameters, type, tag, flags
      12,   10,    9,    9, 0x05,
      52,   10,    9,    9, 0x05,
      91,   10,    9,    9, 0x05,
     129,    9,    9,    9, 0x05,
     166,  159,    9,    9, 0x05,
     212,    9,    9,    9, 0x05,
     226,    9,    9,    9, 0x05,
     242,    9,    9,    9, 0x05,
     257,    9,    9,    9, 0x05,
     275,    9,    9,    9, 0x05,
     294,    9,    9,    9, 0x05,
     315,    9,    9,    9, 0x05,
     346,    9,    9,    9, 0x05,
     371,   10,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
     406,    9,    9,    9, 0x0a,
     431,    9,    9,    9, 0x0a,
     451,    9,    9,    9, 0x0a,
     472,    9,    9,    9, 0x0a,
     488,    9,    9,    9, 0x0a,
     506,    9,    9,    9, 0x0a,
     523,    9,    9,    9, 0x0a,
     537,    9,    9,    9, 0x0a,
     558,    9,    9,    9, 0x0a,
     572,    9,    9,    9, 0x0a,
     598,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QSubView[] = {
    "QSubView\0\0,\0mouseDoubleClick(QWidget*,QMouseEvent*)\0"
    "mousePressEvent(QWidget*,QMouseEvent*)\0"
    "mouseLeftClick(QWidget*,QMouseEvent*)\0"
    "SetCurrentWindSignl(QWidget*)\0evMap,\0"
    "CurrentStateChangeSignl(QVariantMap,QWidget*)\0"
    "Connectting()\0DisConnecting()\0"
    "DisConnected()\0RMousePressMenu()\0"
    "RenderHistoryPix()\0AutoConnectSignals()\0"
    "CreateAutoConnectTimeSignals()\0"
    "RecordStateSignals(bool)\0"
    "ChangeAudioHint(QString,QSubView*)\0"
    "timerEvent(QTimerEvent*)\0OnRMousePressMenu()\0"
    "OnCloseFromMouseEv()\0OnConnectting()\0"
    "OnDisConnecting()\0OnDisConnected()\0"
    "OnOpenAudio()\0OnRenderHistoryPix()\0"
    "OnCheckTime()\0OnCreateAutoConnectTime()\0"
    "In_OpenAutoConnect()\0"
};

void QSubView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QSubView *_t = static_cast<QSubView *>(_o);
        switch (_id) {
        case 0: _t->mouseDoubleClick((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QMouseEvent*(*)>(_a[2]))); break;
        case 1: _t->mousePressEvent((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QMouseEvent*(*)>(_a[2]))); break;
        case 2: _t->mouseLeftClick((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QMouseEvent*(*)>(_a[2]))); break;
        case 3: _t->SetCurrentWindSignl((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 4: _t->CurrentStateChangeSignl((*reinterpret_cast< QVariantMap(*)>(_a[1])),(*reinterpret_cast< QWidget*(*)>(_a[2]))); break;
        case 5: _t->Connectting(); break;
        case 6: _t->DisConnecting(); break;
        case 7: _t->DisConnected(); break;
        case 8: _t->RMousePressMenu(); break;
        case 9: _t->RenderHistoryPix(); break;
        case 10: _t->AutoConnectSignals(); break;
        case 11: _t->CreateAutoConnectTimeSignals(); break;
        case 12: _t->RecordStateSignals((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->ChangeAudioHint((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QSubView*(*)>(_a[2]))); break;
        case 14: _t->timerEvent((*reinterpret_cast< QTimerEvent*(*)>(_a[1]))); break;
        case 15: _t->OnRMousePressMenu(); break;
        case 16: _t->OnCloseFromMouseEv(); break;
        case 17: _t->OnConnectting(); break;
        case 18: _t->OnDisConnecting(); break;
        case 19: _t->OnDisConnected(); break;
        case 20: _t->OnOpenAudio(); break;
        case 21: _t->OnRenderHistoryPix(); break;
        case 22: _t->OnCheckTime(); break;
        case 23: _t->OnCreateAutoConnectTime(); break;
        case 24: _t->In_OpenAutoConnect(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QSubView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QSubView::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QSubView,
      qt_meta_data_QSubView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QSubView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QSubView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QSubView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QSubView))
        return static_cast<void*>(const_cast< QSubView*>(this));
    return QWidget::qt_metacast(_clname);
}

int QSubView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 25)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 25;
    }
    return _id;
}

// SIGNAL 0
void QSubView::mouseDoubleClick(QWidget * _t1, QMouseEvent * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QSubView::mousePressEvent(QWidget * _t1, QMouseEvent * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QSubView::mouseLeftClick(QWidget * _t1, QMouseEvent * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QSubView::SetCurrentWindSignl(QWidget * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void QSubView::CurrentStateChangeSignl(QVariantMap _t1, QWidget * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void QSubView::Connectting()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}

// SIGNAL 6
void QSubView::DisConnecting()
{
    QMetaObject::activate(this, &staticMetaObject, 6, 0);
}

// SIGNAL 7
void QSubView::DisConnected()
{
    QMetaObject::activate(this, &staticMetaObject, 7, 0);
}

// SIGNAL 8
void QSubView::RMousePressMenu()
{
    QMetaObject::activate(this, &staticMetaObject, 8, 0);
}

// SIGNAL 9
void QSubView::RenderHistoryPix()
{
    QMetaObject::activate(this, &staticMetaObject, 9, 0);
}

// SIGNAL 10
void QSubView::AutoConnectSignals()
{
    QMetaObject::activate(this, &staticMetaObject, 10, 0);
}

// SIGNAL 11
void QSubView::CreateAutoConnectTimeSignals()
{
    QMetaObject::activate(this, &staticMetaObject, 11, 0);
}

// SIGNAL 12
void QSubView::RecordStateSignals(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void QSubView::ChangeAudioHint(QString _t1, QSubView * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}
QT_END_MOC_NAMESPACE
