#ifndef LOCALPLAYER_GLOBAL_H
#define LOCALPLAYER_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QStringList>
#include <QDateTime>

#include "PlayMgr.h"

typedef struct _tagPrePlay{
	PlayMgr *pPlayMgr;
	QStringList fileList;
	QDateTime startTime;
	QDateTime endTime;
	int startPos;
}PrePlay;


#endif // LOCALPLAYER_GLOBAL_H
