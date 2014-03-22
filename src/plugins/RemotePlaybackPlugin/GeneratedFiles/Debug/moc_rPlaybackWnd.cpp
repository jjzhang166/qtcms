/****************************************************************************
** Meta object code from reading C++ file 'rPlaybackWnd.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../rPlaybackWnd.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rPlaybackWnd.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RPlaybackWnd[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   14,   13,   13, 0x0a,
      84,   61,   57,   13, 0x0a,
     131,  124,   57,   13, 0x0a,
     174,  156,   57,   13, 0x0a,
     229,  209,   13,   13, 0x0a,
     298,  264,   57,   13, 0x0a,
     350,   13,  342,   13, 0x0a,
     386,  369,   57,   13, 0x0a,
     417,   13,   57,   13, 0x0a,
     430,   13,   57,   13, 0x0a,
     446,   13,   57,   13, 0x0a,
     468,  458,   57,   13, 0x0a,
     489,   13,   57,   13, 0x0a,
     506,   13,   57,   13, 0x0a,
     523,   13,   57,   13, 0x0a,
     544,  542,   13,   13, 0x08,
     587,   13,   13,   13, 0x08,
     612,  542,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_RPlaybackWnd[] = {
    "RPlaybackWnd\0\0sEvent,sProc\0"
    "AddEventProc(QString,QString)\0int\0"
    "sAddress,uiPort,eseeID\0"
    "setDeviceHostInfo(QString,uint,QString)\0"
    "vendor\0setDeviceVendor(QString)\0"
    "uiWndId,uiChannel\0AddChannelIntoPlayGroup(uint,uint)\0"
    "sUsername,sPassword\0"
    "setUserVerifyInfo(QString,QString)\0"
    "nChannel,nTypes,startTime,endTime\0"
    "startSearchRecFile(int,int,QString,QString)\0"
    "QString\0GetNowPlayedTime()\0nTypes,start,end\0"
    "GroupPlay(int,QString,QString)\0"
    "GroupPause()\0GroupContinue()\0GroupStop()\0"
    "uiPersent\0GroupSetVolume(uint)\0"
    "GroupSpeedFast()\0GroupSpeedSlow()\0"
    "GroupSpeedNormal()\0,\0"
    "OnSubWindowDblClick(QWidget*,QMouseEvent*)\0"
    "SetCurrentWind(QWidget*)\0"
    "ChangeAudioHint(QString,RSubView*)\0"
};

void RPlaybackWnd::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RPlaybackWnd *_t = static_cast<RPlaybackWnd *>(_o);
        switch (_id) {
        case 0: _t->AddEventProc((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: { int _r = _t->setDeviceHostInfo((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 2: { int _r = _t->setDeviceVendor((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 3: { int _r = _t->AddChannelIntoPlayGroup((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 4: _t->setUserVerifyInfo((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 5: { int _r = _t->startSearchRecFile((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 6: { QString _r = _t->GetNowPlayedTime();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 7: { int _r = _t->GroupPlay((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 8: { int _r = _t->GroupPause();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 9: { int _r = _t->GroupContinue();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 10: { int _r = _t->GroupStop();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 11: { int _r = _t->GroupSetVolume((*reinterpret_cast< const uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 12: { int _r = _t->GroupSpeedFast();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 13: { int _r = _t->GroupSpeedSlow();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 14: { int _r = _t->GroupSpeedNormal();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 15: _t->OnSubWindowDblClick((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QMouseEvent*(*)>(_a[2]))); break;
        case 16: _t->SetCurrentWind((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 17: _t->ChangeAudioHint((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< RSubView*(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData RPlaybackWnd::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RPlaybackWnd::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_RPlaybackWnd,
      qt_meta_data_RPlaybackWnd, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RPlaybackWnd::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RPlaybackWnd::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RPlaybackWnd::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RPlaybackWnd))
        return static_cast<void*>(const_cast< RPlaybackWnd*>(this));
    if (!strcmp(_clname, "QWebPluginFWBase"))
        return static_cast< QWebPluginFWBase*>(const_cast< RPlaybackWnd*>(this));
    return QWidget::qt_metacast(_clname);
}

int RPlaybackWnd::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
