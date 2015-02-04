#include "screenshotsch.h"
#include <QCoreApplication>
#include <QDateTime>

#include <QDebug>

#define qDebug() qDebug()<<__FUNCTION__<<__LINE__

static const char* gs_keyArr[] = {"fileName", "fileDir", "userName", "wndId", "type", "time"};

ScreenShotSch::ScreenShotSch(QWidget *parent)
	: QWidget(parent),
	QWebPluginFWBase(this)
{

}

ScreenShotSch::~ScreenShotSch()
{

}

int ScreenShotSch::searchScreenShot( const QString &sWndId,const QString &sTime,const int &nType,const QString &sUser )
{
	qint64 nWnd = sWndId.toLongLong(NULL, 2);
	//check input parameter
	if (nWnd < 0 || nWnd > ((qint64)1<<MAX_WINDOWS_NUM) - 1 || nType < 0 || nType > 3 || sUser.isEmpty()){
		qDebug()<<"input parameters error!";
		return 1;
	}
	//get database interface
	char *pErr = NULL;
	sqlite3_stmt *pstmt = NULL;
	sqlite3 *pdb = getSqlInterface();
	if (!pdb){
		qDebug()<<"get sql interface error";
		return 2;
	}
	//create sql
	QString sql = "select * from screenShot where ";
	if (((qint64)1<<MAX_WINDOWS_NUM) - 1 != nWnd){
		sql += createSql(nWnd, QString("chl"));
	}
	if (!sTime.isEmpty()){
		QDateTime date = QDateTime::fromString(sTime, "yyyy-MM-dd");
		sql += QString(" and (time-%1)<86399 ").arg(date.toTime_t());
	}
	if (3 != nType){
		sql += " and " + createSql(nType, QString("type"));
	}
	sql += QString(" and userName='%1'").arg(sUser);

	//start search
	int ret = sqlite3_prepare_v2(pdb, sql.toLatin1().data(), -1, &pstmt, (const char**)&pErr);
	if (SQLITE_OK != ret){
		qDebug()<<"search fail, error:"<<pErr;
		sqlite3_close(pdb);
		return 2;
	}
	int col = sqlite3_column_count(pstmt);
	ret = sqlite3_step(pstmt);
	while (SQLITE_ROW == ret){
		QVariantMap info;
		for (int index = 0; index < col - 1; index++){
			info.insert(gs_keyArr[index], QString((const char*)sqlite3_column_text(pstmt, index + 1)));
		}
		QDateTime datetime = QDateTime::fromTime_t(info["time"].toUInt()); 
		info["time"] = datetime.toString("yyyy-MM-dd hh:mm:ss");

		EventProcCall(QString("ScreenShotInfo"), info);
		ret = sqlite3_step(pstmt);
	}
	//release resource
	sqlite3_finalize(pstmt);
	sqlite3_close(pdb);

	return 0;
}

sqlite3* ScreenShotSch::getSqlInterface()
{
	QString path = QCoreApplication::applicationDirPath() + "/system.db";
	sqlite3 *pdb = NULL;
	int ret = sqlite3_open(path.toLatin1().data(), &pdb);
	if (SQLITE_OK != ret){
		qDebug()<<"open database "<<path<<" fail";
		return NULL;
	}
	return pdb;
}

QString ScreenShotSch::createSql( qint64 num, QString keyWord )
{
	QStringList list;
	int count = 0;
	while (num){
		if (num & 1){
			list.append(QString("%1=%2").arg(keyWord).arg(count));
		}
		count++;
		num = num>>1;
	}
	return "(" + list.join(" or ") + ")";
}
