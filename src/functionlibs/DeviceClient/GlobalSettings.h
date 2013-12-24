#ifndef _GLOBALSETTINGS_HEAD_FILE_
#define _GLOBALSETTINGS_HEAD_FILE_
#include <QMutex>
#include <QWaitCondition>
#include "PlayManager.h"



 extern QMutex g_mutex;
 extern QWaitCondition g_pause;

 typedef struct _tagWndPlay{
 	BufferManager *bufferManager;
 	PlayManager *playManager;
 	QWidget *wnd;
 }WndPlay;
 

#endif