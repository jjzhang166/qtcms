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
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   14,   13,   13, 0x0a,
      70,   61,   57,   13, 0x0a,
     150,  102,   57,   13, 0x0a,
     241,  207,   57,   13, 0x0a,
     295,  291,   57,   13, 0x0a,
     315,   13,   57,   13, 0x0a,
     327,   13,   57,   13, 0x0a,
     340,   13,   57,   13, 0x0a,
     356,   13,   57,   13, 0x0a,
     374,  368,   57,   13, 0x0a,
     394,  368,   57,   13, 0x0a,
     414,   13,   57,   13, 0x0a,
     441,   13,  433,   13, 0x0a,
     470,  460,   57,   13, 0x0a,
     493,  491,   13,   13, 0x0a,
     536,  491,   13,   13, 0x08,
     579,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_RecordPlayer[] = {
    "RecordPlayer\0\0sEvent,sProc\0"
    "AddEventProc(QString,QString)\0int\0"
    "sdevname\0searchDateByDeviceName(QString)\0"
    "sdevname,sdate,sbegintime,sendtime,schannellist\0"
    "searchVideoFile(QString,QString,QString,QString,QString)\0"
    "filelist,nWndID,startTime,endTime\0"
    "AddFileIntoPlayGroup(QString,int,QString,QString)\0"
    "num\0SetSynGroupNum(int)\0GroupPlay()\0"
    "GroupPause()\0GroupContinue()\0GroupStop()\0"
    "speed\0GroupSpeedFast(int)\0GroupSpeedSlow(int)\0"
    "GroupSpeedNormal()\0QString\0"
    "GetNowPlayedTime()\0uiPersent\0"
    "GroupSetVolume(uint)\0,\0"
    "ChangeAudioHint(QString,RecordPlayerView*)\0"
    "OnSubWindowDblClick(QWidget*,QMouseEvent*)\0"
    "SetCurrentWind(QWidget*)\0"
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
        case 3: { int _r = _t->AddFileIntoPlayGroup((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 4: { int _r = _t->SetSynGroupNum((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 5: { int _r = _t->GroupPlay();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 6: { int _r = _t->GroupPause();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 7: { int _r = _t->GroupContinue();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 8: { int _r = _t->GroupStop();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 9: { int _r = _t->GroupSpeedFast((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 10: { int _r = _t->GroupSpeedSlow((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 11: { int _r = _t->GroupSpeedNormal();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 12: { QString _r = _t->GetNowPlayedTime();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 13: { int _r = _t->GroupSetVolume((*reinterpret_cast< const uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 14: _t->ChangeAudioHint((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< RecordPlayerView*(*)>(_a[2]))); break;
        case 15: _t->OnSubWindowDblClick((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QMouseEvent*(*)>(_a[2]))); break;
        case 16: _t->SetCurrentWind((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
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
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
