/****************************************************************************
** Meta object code from reading C++ file 'RemoteBackupWindows.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../RemoteBackupWindows.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RemoteBackupWindows.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RemoteBackupWindows[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   21,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      63,   50,   20,   20, 0x0a,
     160,   97,   93,   20, 0x0a,
     226,   20,   93,   20, 0x0a,
     245,   20,  239,   20, 0x0a,
     259,   20,   20,   20, 0x0a,
     271,   21,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_RemoteBackupWindows[] = {
    "RemoteBackupWindows\0\0item\0"
    "sendStatus(QVariantMap)\0sEvent,sProc\0"
    "AddEventProc(QString,QString)\0int\0"
    "sAddr,uiPort,sEseeId,nChannel,nTypes,startTime,endTime,sbkpath\0"
    "startBackup(QString,uint,QString,int,int,QString,QString,QString)\0"
    "stopBackup()\0float\0getProgress()\0"
    "ChooseDir()\0sendToHtml(QVariantMap)\0"
};

void RemoteBackupWindows::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RemoteBackupWindows *_t = static_cast<RemoteBackupWindows *>(_o);
        switch (_id) {
        case 0: _t->sendStatus((*reinterpret_cast< QVariantMap(*)>(_a[1]))); break;
        case 1: _t->AddEventProc((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: { int _r = _t->startBackup((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])),(*reinterpret_cast< const QString(*)>(_a[6])),(*reinterpret_cast< const QString(*)>(_a[7])),(*reinterpret_cast< const QString(*)>(_a[8])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 3: { int _r = _t->stopBackup();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 4: { float _r = _t->getProgress();
            if (_a[0]) *reinterpret_cast< float*>(_a[0]) = _r; }  break;
        case 5: _t->ChooseDir(); break;
        case 6: _t->sendToHtml((*reinterpret_cast< QVariantMap(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData RemoteBackupWindows::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RemoteBackupWindows::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_RemoteBackupWindows,
      qt_meta_data_RemoteBackupWindows, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RemoteBackupWindows::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RemoteBackupWindows::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RemoteBackupWindows::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RemoteBackupWindows))
        return static_cast<void*>(const_cast< RemoteBackupWindows*>(this));
    if (!strcmp(_clname, "QWebPluginFWBase"))
        return static_cast< QWebPluginFWBase*>(const_cast< RemoteBackupWindows*>(this));
    return QWidget::qt_metacast(_clname);
}

int RemoteBackupWindows::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void RemoteBackupWindows::sendStatus(QVariantMap _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
