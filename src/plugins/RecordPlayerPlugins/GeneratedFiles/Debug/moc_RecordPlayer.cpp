/****************************************************************************
** Meta object code from reading C++ file 'RecordPlayer.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../RecordPlayer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RecordPlayer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RecordPlayer[] = {

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
      27,   14,   13,   13, 0x0a,
      70,   61,   57,   13, 0x0a,
     150,  102,   57,   13, 0x0a,
     213,  207,   13,   13, 0x0a,
     243,  207,   13,   13, 0x0a,
     274,  207,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_RecordPlayer[] = {
    "RecordPlayer\0\0sEvent,sProc\0"
    "AddEventProc(QString,QString)\0int\0"
    "sdevname\0searchDateByDeviceName(QString)\0"
    "sdevname,sdate,sbegintime,sendtime,schannellist\0"
    "searchVideoFile(QString,QString,QString,QString,QString)\0"
    "evMap\0transRecordDate(QVariantMap&)\0"
    "transRecordFiles(QVariantMap&)\0"
    "transSearchStop(QVariantMap&)\0"
};

void RecordPlayer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RecordPlayer *_t = static_cast<RecordPlayer *>(_o);
        switch (_id) {
        case 0: _t->AddEventProc((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: { int _r = _t->searchDateByDeviceName((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 2: { int _r = _t->searchVideoFile((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const QString(*)>(_a[5])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 3: _t->transRecordDate((*reinterpret_cast< QVariantMap(*)>(_a[1]))); break;
        case 4: _t->transRecordFiles((*reinterpret_cast< QVariantMap(*)>(_a[1]))); break;
        case 5: _t->transSearchStop((*reinterpret_cast< QVariantMap(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData RecordPlayer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RecordPlayer::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_RecordPlayer,
      qt_meta_data_RecordPlayer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RecordPlayer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RecordPlayer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RecordPlayer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RecordPlayer))
        return static_cast<void*>(const_cast< RecordPlayer*>(this));
    if (!strcmp(_clname, "QWebPluginFWBase"))
        return static_cast< QWebPluginFWBase*>(const_cast< RecordPlayer*>(this));
    return QWidget::qt_metacast(_clname);
}

int RecordPlayer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
