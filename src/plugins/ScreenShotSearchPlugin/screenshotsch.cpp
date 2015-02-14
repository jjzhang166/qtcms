#include "screenshotsch.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>

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

int ScreenShotSch::searchScreenShot( const QString &sWndId, const QString &sStartTime, const QString &sEndTime, const int &nType, const QString &sUser )
{
	qDebug()<<sWndId<<sStartTime<<sEndTime<<nType<<sUser;

	qint64 nWnd = sWndId.toLongLong(NULL, 2);
	//check input parameter
	if (nWnd <= 0 || nWnd > ((qint64)1<<MAX_WINDOWS_NUM) - 1 || nType <= 0 || nType > 7 || sStartTime.isEmpty() || sEndTime.isEmpty() || sUser.isEmpty()){
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
	if (((qint64)1<<MAX_WINDOWS_NUM) - 1 != nWnd && 0 != nWnd){
		sql += createSql(nWnd, QString("chl")) + " and ";
	}
	QDateTime start = QDateTime::fromString(sStartTime, "yyyy-MM-dd");
	QDateTime end = QDateTime::fromString(sEndTime, "yyyy-MM-dd");
	end = end.addSecs(24*3600 - 1);

	sql += QString("(time>=%1 and time<=%2) and ").arg(QString::number(start.toMSecsSinceEpoch())).arg(QString::number(end.toMSecsSinceEpoch()));

	if (3 != nType){
		sql += createSql(nType, QString("type")) + " and ";
	}
	sql += QString("userName='%1'").arg(sUser);
	//clear old data
	m_infoMap.clear();

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
		QDateTime datetime = QDateTime::fromMSecsSinceEpoch(info["time"].toULongLong()); 
		info["time"] = datetime.toString("yyyy-MM-dd hh:mm:ss");
		QString path = info["fileDir"].toString() + "/" + info["fileName"].toString();
		if (QFile::exists(path)){
// 			EventProcCall(QString("ScreenShotInfo"), info);
			combineData(info);
		}

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

void ScreenShotSch::combineData( QVariantMap item )
{
	QString info;
	QStringList attrList;
	QVariantMap::iterator iter = item.begin();
	while (iter != item.end()){
		attrList<<"\\\"" + iter.key() + "\\\":\\\"" + iter.value().toString() + "\\\"";
		iter++;
	}
	info = "{" + attrList.join(",") + "}";
	int len = m_infoMap.size();
	m_infoMap.insert(len, info);
// 	if (99 == len){
// 		EventProcCall(QString("ScreenShotInfo"), m_infoMap);
// 		m_infoMap.clear();
// 	}
}

int ScreenShotSch::getImageNum()
{
	return m_infoMap.size();
}

QString ScreenShotSch::getImageInfo( const int &nStartIndex, const int &nEndIndex )
{
	QString res;
	if (nStartIndex > nEndIndex || nStartIndex > m_infoMap.size() || nEndIndex > m_infoMap.size()){
		qDebug()<<"input index error";
		return res;
	}
	res += "{";
	int index = nStartIndex;
	do 
	{
		QString val = m_infoMap.value(index);
		if (val.isEmpty()){
			continue;
		}
		res += QString("index_%1").arg(index) + ":'" + val + "'";
		if (index != nEndIndex - 1){
			res += ",";
		}
		index++;
	} while (index < nEndIndex);
	res += "}";

	return res;
}
