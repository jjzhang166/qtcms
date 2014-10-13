#pragma once
#include <QThread>
#include <QSqlQuery>
class sqliteThread:public QThread
{
	Q_OBJECT
public:
	sqliteThread(void);
	~sqliteThread(void);
private:
	void run();
public:
	void startThread();
	void setTestMode(int nMode);
private:
	bool execCommand(QSqlQuery & tQuery,QString sCommand);
private:
	int m_nMode;
};

