#ifndef _DEVICEGLOBALSETTINGS_HEAD_FILE_
#define _DEVICEGLOBALSETTINGS_HEAD_FILE_
#include <QDebug>
#ifdef _DEBUG
#define Device_Debug(param) qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<param
#else
#define Device_Debug(param)//DeviceDebug(QString param)
#endif
#endif