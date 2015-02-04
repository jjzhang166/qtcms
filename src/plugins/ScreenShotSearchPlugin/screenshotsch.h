#ifndef SCREENSHOTSCH_H
#define SCREENSHOTSCH_H

#include <QWidget>
#include "qwfw.h"
#include "sqlite3.h"


#define MAX_WINDOWS_NUM 49

class ScreenShotSch : public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	ScreenShotSch(QWidget *parent = 0);
	~ScreenShotSch();
public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}

	int searchScreenShot(const QString &sWndId,const QString &sTime,const int &nType,const QString &sUser);
private:
	sqlite3* getSqlInterface();
	QString createSql(qint64 num, QString keyWord);
};

#endif // SCREENSHOTSCH_H
