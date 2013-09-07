/****************************************************************************
** Meta object code from reading C++ file 'qcommonplugin.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qcommonplugin.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qcommonplugin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QCommonPlugin[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      28,   15,   14,   14, 0x0a,
     121,   62,   58,   14, 0x0a,
     168,  158,   58,   14, 0x0a,
     211,  188,   58,   14, 0x0a,
     264,  247,   58,   14, 0x0a,
     335,  293,   58,   14, 0x0a,
     381,  158,  376,   14, 0x0a,
     423,  403,  376,   14, 0x0a,
     450,  158,   58,   14, 0x0a,
     484,  158,  472,   14, 0x0a,
     514,   14,   58,   14, 0x0a,
     529,   14,  472,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QCommonPlugin[] = {
    "QCommonPlugin\0\0sEvent,sProc\0"
    "AddEventProc(QString,QString)\0int\0"
    "sUsername,sPassword,nLevel,nAuthorityMask1,nAuthorityMask2\0"
    "AddUser(QString,QString,int,int,int)\0"
    "sUsername\0RemoveUser(QString)\0"
    "sUsername,sNewPassword\0"
    "ModifyUserPassword(QString,QString)\0"
    "sUsername,nLevel\0ModifyUserLevel(QString,int)\0"
    "sUsername,nAuthorityMask1,nAuthorityMask2\0"
    "ModifyUserAuthorityMask(QString,int,int)\0"
    "bool\0IsUserExists(QString)\0"
    "sUsername,sPassword\0CheckUser(QString,QString)\0"
    "GetUserLevel(QString)\0QStringList\0"
    "GetUserAuthorityMask(QString)\0"
    "GetUserCount()\0GetUserList()\0"
};

void QCommonPlugin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QCommonPlugin *_t = static_cast<QCommonPlugin *>(_o);
        switch (_id) {
        case 0: _t->AddEventProc((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: { int _r = _t->AddUser((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 2: { int _r = _t->RemoveUser((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 3: { int _r = _t->ModifyUserPassword((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 4: { int _r = _t->ModifyUserLevel((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 5: { int _r = _t->ModifyUserAuthorityMask((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 6: { bool _r = _t->IsUserExists((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 7: { bool _r = _t->CheckUser((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 8: { int _r = _t->GetUserLevel((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 9: { QStringList _r = _t->GetUserAuthorityMask((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = _r; }  break;
        case 10: { int _r = _t->GetUserCount();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 11: { QStringList _r = _t->GetUserList();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QCommonPlugin::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QCommonPlugin::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QCommonPlugin,
      qt_meta_data_QCommonPlugin, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QCommonPlugin::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QCommonPlugin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QCommonPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QCommonPlugin))
        return static_cast<void*>(const_cast< QCommonPlugin*>(this));
    if (!strcmp(_clname, "QWebPluginFWBase"))
        return static_cast< QWebPluginFWBase*>(const_cast< QCommonPlugin*>(this));
    return QWidget::qt_metacast(_clname);
}

int QCommonPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
