#ifndef _GLOBALSETTINGS_HEAD_FILE_
#define _GLOBALSETTINGS_HEAD_FILE_
#include <QMutex>
#include <QWaitCondition>

extern QMutex g_mutex;
extern QWaitCondition g_verify;

#endif