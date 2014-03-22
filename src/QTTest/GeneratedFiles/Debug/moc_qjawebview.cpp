/****************************************************************************
** Meta object code from reading C++ file 'qjawebview.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qjawebview.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qjawebview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QJaWebView[] = {

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
      16,   12,   11,   11, 0x0a,
      34,   29,   11,   11, 0x0a,
      66,   62,   11,   11, 0x0a,
     100,   95,   85,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QJaWebView[] = {
    "QJaWebView\0\0bOk\0OnLoad(bool)\0text\0"
    "OnstatusBarMessage(QString)\0url\0"
    "OnurlChanged(QUrl)\0QWebView*\0type\0"
    "createWindow(QWebPage::WebWindowType)\0"
};

void QJaWebView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QJaWebView *_t = static_cast<QJaWebView *>(_o);
        switch (_id) {
        case 0: _t->OnLoad((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->OnstatusBarMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->OnurlChanged((*reinterpret_cast< const QUrl(*)>(_a[1]))); break;
        case 3: { QWebView* _r = _t->createWindow((*reinterpret_cast< QWebPage::WebWindowType(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QWebView**>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QJaWebView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QJaWebView::staticMetaObject = {
    { &QWebView::staticMetaObject, qt_meta_stringdata_QJaWebView,
      qt_meta_data_QJaWebView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QJaWebView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QJaWebView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QJaWebView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QJaWebView))
        return static_cast<void*>(const_cast< QJaWebView*>(this));
    return QWebView::qt_metacast(_clname);
}

int QJaWebView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_END_MOC_NAMESPACE
