#include <QApplication>
#include <QtCore/QCoreApplication>
#include "qjawebview.h"
#include "libpcom.h"
#include <QEventLoop>
#include <QtDebug>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <MyEventSender.h>
#include <QMutex>

//#include "vld.h"

QMutex g_tMessage;
QString g_sLocalPath;
QFile outFile; 
#define  FILESIZE 52428800//50M
void customMessageHandler(QtMsgType type, const char *msg){
	g_tMessage.lock();
	QString txt; 
	QString sIgnore="QNativeSocketEngine::hasPendingDatagrams() was called in QAbstractSocket::UnconnectedState";
	switch (type) {  
	case QtDebugMsg:  
		txt = QString("Debug: %1").arg(msg);
		txt.append("-----");
		txt.append(QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz"));
		break;  

	case QtWarningMsg:  
		txt = QString("Warning: %1").arg(msg);
		if (txt.contains(sIgnore))
		{
			g_tMessage.unlock();
			return;
		}else{
			//keep going
		}
		txt.append("-----");
		txt.append(QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz"));
		

		break;  
	case QtCriticalMsg:  
		txt = QString("Critical: %1").arg(msg);  
		break;  
	case QtFatalMsg:  
		txt = QString("Fatal: %1").arg(msg); 
		QTextStream stds(stdout);
		stds << txt << endl;
		g_tMessage.unlock();
		abort();  
	
	} 
	if (outFile.size()>FILESIZE)
	{
		QString sFilePath;
		sFilePath=g_sLocalPath+"/"+QDate::currentDate().toString("dd.MM.yyyy")+"-"+QTime::currentTime().toString("hh-mm-ss")+".log";
		outFile.close();
		outFile.setFileName(sFilePath);
		if (outFile.isOpen())
		{
			//do nothing
		}else{
			outFile.open(QIODevice::WriteOnly | QIODevice::Append);  
		}
		QTextStream ts(&outFile);  
		QString txt="\r\n\r\n=====start=====\r\n\r\n";
		ts << txt << endl; 
	}else{
		//keep going 
	}
	if (outFile.isOpen())
	{
		//do nothing
	}else{
		outFile.open(QIODevice::WriteOnly | QIODevice::Append);  
	}
	QTextStream ts(&outFile);  
	ts << txt << endl; 

	QTextStream stds(stdout);
	stds << txt << endl;
	g_tMessage.unlock();
}
int main(int argc, char *argv[])
{
	MyEventSender a(argc,argv);
	//debug log
	qInstallMsgHandler(customMessageHandler);
	QString sAppPath=QCoreApplication::applicationDirPath();
	sAppPath+="/log";
	QDir tLogDir;
	if (tLogDir.exists(sAppPath))
	{
		//do nothing
	}else{
		tLogDir.mkpath(sAppPath);
	}
	g_sLocalPath=sAppPath;
	QString sLogName=sAppPath+"/"+QDate::currentDate().toString("dd.MM.yyyy")+".log";
	outFile.setFileName(sLogName);
	if (outFile.isOpen())
	{
		//do nothing
	}else{
		outFile.open(QIODevice::WriteOnly | QIODevice::Append);  
	}
	QTextStream ts(&outFile);  
	QString txt="\r\n\r\n=====start=====\r\n\r\n";
	ts << txt << endl; 
	// Get Application Path
	QString sTemp = QCoreApplication::applicationDirPath();
	// extern libraries path
	QString sExternLib(sTemp + "/exlibs");
	QApplication::addLibraryPath(sExternLib);
	QJaWebView view;
	QEventLoop eventloop;
	QTimer::singleShot(500,&eventloop,SLOT(quit()));
	eventloop.exec();
    view.showMaximized(); 
    return a.exec();
}
