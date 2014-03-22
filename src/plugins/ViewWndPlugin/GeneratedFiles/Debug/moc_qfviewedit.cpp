/****************************************************************************
** Meta object code from reading C++ file 'qfviewedit.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qfviewedit.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qfviewedit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qfviewedit[] = {

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
      18,   12,   11,   11, 0x0a,
      50,   37,   11,   11, 0x0a,
      98,   92,   80,   11, 0x0a,
     122,   92,   11,   11, 0x0a,
     136,   11,   80,   11, 0x0a,
     166,   11,  154,   11, 0x0a,
     180,   11,  176,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_qfviewedit[] = {
    "qfviewedit\0\0sText\0PrintText(QString)\0"
    "sEvent,sProc\0AddEventProc(QString,QString)\0"
    "QStringList\0nTest\0TestStringList(QString)\0"
    "RefTest(int&)\0PointReturnTest()\0"
    "QVariantMap\0RetTest()\0int\0my_test()\0"
};

void qfviewedit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        qfviewedit *_t = static_cast<qfviewedit *>(_o);
        switch (_id) {
        case 0: _t->PrintText((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->AddEventProc((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: { QStringList _r = _t->TestStringList((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = _r; }  break;
        case 3: _t->RefTest((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: { QStringList _r = _t->PointReturnTest();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = _r; }  break;
        case 5: { QVariantMap _r = _t->RetTest();
            if (_a[0]) *reinterpret_cast< QVariantMap*>(_a[0]) = _r; }  break;
        case 6: { int _r = _t->my_test();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData qfviewedit::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject qfviewedit::staticMetaObject = {
    { &QTextEdit::staticMetaObject, qt_meta_stringdata_qfviewedit,
      qt_meta_data_qfviewedit, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qfviewedit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qfviewedit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qfviewedit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qfviewedit))
        return static_cast<void*>(const_cast< qfviewedit*>(this));
    if (!strcmp(_clname, "QWebPluginFWBase"))
        return static_cast< QWebPluginFWBase*>(const_cast< qfviewedit*>(this));
    return QTextEdit::qt_metacast(_clname);
}

int qfviewedit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTextEdit::qt_metacall(_c, _id, _a);
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
