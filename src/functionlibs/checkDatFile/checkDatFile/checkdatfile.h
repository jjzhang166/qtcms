#ifndef CHECKDATFILE_H
#define CHECKDATFILE_H

#include <QtGui/QMainWindow>
#include "checkdatfile_global.h"
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QTextStream>
#include <QtCore/QCoreApplication>
#include <QMutex>
#include <QFile>
#include <QDateTime>
#include <QtSql>
#include <QList>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QMap>
#include <QDateTime>
#include <QTextEdit>
#include <sqliteThread.h>
#define DIRSIZE 256
class checkDatFile : public QWidget
{
	Q_OBJECT
public:
	checkDatFile(QWidget *parent = 0);
	~checkDatFile();

	void setText(QTextEdit **tText);
public slots:
	void slCheckFile();
	void slDateTime();
	void slSqlThread();
private:
	QString getRecordDisk();//D,E,F;
	void checkDiskItem(QString sDiskTtem);
	bool createRecordDatabase(QString sDatabasePath);
	bool createRecordFileStatusItem(quint64 uiFileNum,QString sFilePath);
	void analysisDatFile(QString sFilePath);
	bool setRecordFileStatus(QString sFilePath,QString sKey,int nFlags);
	bool isJUANRecordDatFile(QString sFilePath);
	bool testPerFrame(tagFileHead *pFileHead,QString sFilePath);
	bool testFrameHeadInfo(tagFileFrameHead* pFileFrameHead,QString sFilePath);
	bool testPerFrameIndex(tagFileFrameHead* pFileFrameHead ,QString sFilePath,quint64 uiCurrentIndex);
	void clearDatFileInfo();
	bool saveItemToDatabase(QString sFilePath);
	bool builtSearch_recordItem(QString sDiskTtem);
	void printfFileData(QString sFilePath);
	void changeDateTime(QString sDiskItem);
	bool createDateTime(QString sDiskItem);
	bool insertDateTime(QString sDatabasePath,QList<tagSearch_recordItem> tSearchRecordItem,QList<tagRecordItemInfo> tRecordItemInfo);
private:
	QVBoxLayout *m_pLayout;
	QPushButton *m_pPushButton;
	QPushButton *m_pDateTimeButton;
	QPushButton *m_pSqliteThread;
	QTextEdit *m_pText;
	QString m_sDatabasePath;
	QByteArray m_tFileData;
	QMap<int ,tagWndRecordItemInfo> m_tWndRecordItemInfo;
	tagFileHead m_tFileHeadInfo;
	QList<int> m_tRecordType;
	QList<tagSearch_recordItem> m_tSearchRecordItem;
	QList<tagRecordItemInfo> m_tRecordItemInfo;
	sqliteThread m_tSqlite1;
	sqliteThread m_tSqlite2;
	sqliteThread m_tSqlite3;
	sqliteThread m_tSqlite4;
	sqliteThread m_tSqlite5;
};

#endif // CHECKDATFILE_H
