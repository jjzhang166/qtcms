#include "checkdatfile.h"
#include <QtGui/QApplication>
#include <QTextEdit>
#include <QDir>
#include <QMessageBox>
QTextEdit *g_pTest=NULL;
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
		if (NULL!=g_pTest)
		{
			g_pTest->append(txt);
			QTime dieTime=QTime::currentTime().addMSecs(10);
			while(QTime::currentTime()<dieTime){
				QCoreApplication::processEvents(QEventLoop::AllEvents,100);
			}
		}
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
	QApplication a(argc, argv);
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
	checkDatFile w;
	w.setText(&g_pTest);
	w.show();
	return a.exec();
}
