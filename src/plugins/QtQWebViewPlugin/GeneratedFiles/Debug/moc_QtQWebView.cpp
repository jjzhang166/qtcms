/****************************************************************************
** Meta object code from reading C++ file 'QtQWebView.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../QtQWebView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtQWebView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtQWebView[] = {

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
      25,   12,   11,   11, 0x0a,
      59,   55,   11,   11, 0x0a,
      85,   80,   11,   11, 0x0a,
     120,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtQWebView[] = {
    "QtQWebView\0\0sEvent,sProc\0"
    "AddEventProc(QString,QString)\0url\0"
    "LoadNewPage(QString)\0text\0"
    "LoadNewPageFromViewSignal(QString)\0"
    "CloseAllPage()\0"
};

void QtQWebView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtQWebView *_t = static_cast<QtQWebView *>(_o);
        switch (_id) {
        case 0: _t->AddEventProc((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->LoadNewPage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->LoadNewPageFromViewSignal((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->CloseAllPage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtQWebView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtQWebView::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QtQWebView,
      qt_meta_data_QtQWebView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtQWebView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtQWebView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtQWebView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtQWebView))
        return static_cast<void*>(const_cast< QtQWebView*>(this));
    if (!strcmp(_clname, "QWebPluginFWBase"))
        return static_cast< QWebPluginFWBase*>(const_cast< QtQWebView*>(this));
    return QWidget::qt_metacast(_clname);
}

int QtQWebView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
