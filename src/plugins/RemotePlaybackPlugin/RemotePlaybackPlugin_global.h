#ifndef REMOTEPLAYBACKPLUGIN_GLOBAL_H
#define REMOTEPLAYBACKPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QString>
#include <QVariantMap>
#include <QList>
#include <QDateTime>

typedef struct __tagRecordInfo
{
    uint uiChannel;
    uint uiTypes;
    QDateTime sStartTime;
    QDateTime sEndTime;
    QString sFileName;
}RecordInfo;
typedef struct _tagSession 
{
	uint uiType;
	QList<QDateTime> timeSession;
}Session;
typedef struct _tagTimeInfo
{
	uint uiChannel;
	QList<Session> lstTypeTime;
}TimeInfo;
QList<RecordInfo> g_RecList;

void analyze(QList<RecordInfo>& lstInfo, QList<TimeInfo>&lstTimeInfo);

int cbFoundFile(QString evName,QVariantMap evMap,void*pUser);
int cbRecFileSearchFinished(QString evName,QVariantMap evMap,void*pUser);
int cbSocketError(QString evName,QVariantMap evMap,void*pUser);
int cbStateChange(QString evName,QVariantMap evMap,void*pUser);

int  childThreadSearch(uint uiRecNum, QString& start,QString& end, QList<QVariantMap> &selectedList);

#endif // REMOTEPLAYBACKPLUGIN_GLOBAL_H
