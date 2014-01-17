/****************************************************************************
** Meta object code from reading C++ file 'SubWebView.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SubWebView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SubWebView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SubWebView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      17,   12,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      46,   42,   11,   11, 0x0a,
      59,   12,   11,   11, 0x0a,
      91,   87,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SubWebView[] = {
    "SubWebView\0\0text\0LoadOrChangeUrl(QString)\0"
    "bOk\0OnLoad(bool)\0OnstatusBarMessage(QString)\0"
    "url\0OnurlChanged(QUrl)\0"
};

void SubWebView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SubWebView *_t = static_cast<SubWebView *>(_o);
        switch (_id) {
        case 0: _t->LoadOrChangeUrl((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->OnLoad((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->OnstatusBarMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->OnurlChanged((*reinterpret_cast< const QUrl(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SubWebView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SubWebView::staticMetaObject = {
    { &QWebView::staticMetaObject, qt_meta_stringdata_SubWebView,
      qt_meta_data_SubWebView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SubWebView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SubWebView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SubWebView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SubWebView))
        return static_cast<void*>(const_cast< SubWebView*>(this));
    return QWebView::qt_metacast(_clname);
}

int SubWebView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWebView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void SubWebView::LoadOrChangeUrl(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
