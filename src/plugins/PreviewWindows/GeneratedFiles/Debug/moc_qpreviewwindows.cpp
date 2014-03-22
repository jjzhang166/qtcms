/****************************************************************************
** Meta object code from reading C++ file 'qpreviewwindows.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qpreviewwindows.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qpreviewwindows.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QPreviewWindows[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      30,   17,   16,   16, 0x0a,
      60,   16,   16,   16, 0x0a,
      71,   16,   16,   16, 0x0a,
      85,   16,   81,   16, 0x0a,
     102,   16,   81,   16, 0x0a,
     125,  113,   81,   16, 0x0a,
     153,   16,  145,   16, 0x0a,
     175,  173,   16,   16, 0x0a,
     218,  173,   16,   16, 0x0a,
     264,   16,   16,   16, 0x0a,
     289,   16,   81,   16, 0x0a,
     403,  305,   81,   16, 0x0a,
     509,  488,   81,   16, 0x0a,
     548,  537,   81,   16, 0x0a,
     569,  537,   81,   16, 0x0a,
     611,  601,   16,   16, 0x0a,
     658,  173,   16,   16, 0x0a,
     700,  693,   81,   16, 0x0a,
     717,  693,   81,   16, 0x0a,
     760,  733,   81,   16, 0x0a,
     798,  788,   81,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QPreviewWindows[] = {
    "QPreviewWindows\0\0sEvent,sProc\0"
    "AddEventProc(QString,QString)\0nextPage()\0"
    "prePage()\0int\0getCurrentPage()\0"
    "getPages()\0divModeName\0setDivMode(QString)\0"
    "QString\0getCureentDivMode()\0,\0"
    "OnSubWindowDblClick(QWidget*,QMouseEvent*)\0"
    "OnSubWindowRmousePress(QWidget*,QMouseEvent*)\0"
    "SetCurrentWind(QWidget*)\0GetCurrentWnd()\0"
    "uiWndIndex,sAddress,uiPort,sEseeId,uiChannelId,uiStreamId,sUsername,sP"
    "assword,sCameraname,sVendor\0"
    "OpenCameraInWnd(uint,QString,uint,QString,uint,uint,QString,QString,QS"
    "tring,QString)\0"
    "uiWndIndex,ChannelId\0SetDevChannelInfo(uint,int)\0"
    "uiWndIndex\0CloseWndCamera(uint)\0"
    "GetWindowConnectionStatus(uint)\0"
    "evMap,WID\0CurrentStateChangePlugin(QVariantMap,QWidget*)\0"
    "ChangeAudioHint(QString,QSubView*)\0"
    "nWndID\0StartRecord(int)\0StopRecord(int)\0"
    "devname,nChannelNum,nWndID\0"
    "SetDevInfo(QString,int,int)\0uiPersent\0"
    "SetVolume(uint)\0"
};

void QPreviewWindows::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QPreviewWindows *_t = static_cast<QPreviewWindows *>(_o);
        switch (_id) {
        case 0: _t->AddEventProc((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->nextPage(); break;
        case 2: _t->prePage(); break;
        case 3: { int _r = _t->getCurrentPage();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 4: { int _r = _t->getPages();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 5: { int _r = _t->setDivMode((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 6: { QString _r = _t->getCureentDivMode();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 7: _t->OnSubWindowDblClick((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QMouseEvent*(*)>(_a[2]))); break;
        case 8: _t->OnSubWindowRmousePress((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QMouseEvent*(*)>(_a[2]))); break;
        case 9: _t->SetCurrentWind((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 10: { int _r = _t->GetCurrentWnd();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 11: { int _r = _t->OpenCameraInWnd((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< uint(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< uint(*)>(_a[5])),(*reinterpret_cast< uint(*)>(_a[6])),(*reinterpret_cast< const QString(*)>(_a[7])),(*reinterpret_cast< const QString(*)>(_a[8])),(*reinterpret_cast< const QString(*)>(_a[9])),(*reinterpret_cast< const QString(*)>(_a[10])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 12: { int _r = _t->SetDevChannelInfo((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 13: { int _r = _t->CloseWndCamera((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 14: { int _r = _t->GetWindowConnectionStatus((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 15: _t->CurrentStateChangePlugin((*reinterpret_cast< QVariantMap(*)>(_a[1])),(*reinterpret_cast< QWidget*(*)>(_a[2]))); break;
        case 16: _t->ChangeAudioHint((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QSubView*(*)>(_a[2]))); break;
        case 17: { int _r = _t->StartRecord((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 18: { int _r = _t->StopRecord((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 19: { int _r = _t->SetDevInfo((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 20: { int _r = _t->SetVolume((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QPreviewWindows::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QPreviewWindows::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QPreviewWindows,
      qt_meta_data_QPreviewWindows, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QPreviewWindows::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QPreviewWindows::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QPreviewWindows::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QPreviewWindows))
        return static_cast<void*>(const_cast< QPreviewWindows*>(this));
    if (!strcmp(_clname, "QWebPluginFWBase"))
        return static_cast< QWebPluginFWBase*>(const_cast< QPreviewWindows*>(this));
    return QWidget::qt_metacast(_clname);
}

int QPreviewWindows::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
