#include <QApplication>
#include <QtCore/QCoreApplication>
#include "qjawebview.h"
#include "libpcom.h"
#include <QEventLoop>
#include <QtDebug>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
void customMessageHandler(QtMsgType type, const char *msg){
	QString txt;  
	switch (type) {  
	case QtDebugMsg:  
		txt = QString("Debug: %1").arg(msg);
		txt.append("-----");
		txt.append(QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz"));
		break;  

	case QtWarningMsg:  
		txt = QString("Warning: %1").arg(msg);  
		break;  
	case QtCriticalMsg:  
		txt = QString("Critical: %1").arg(msg);  
		break;  
	case QtFatalMsg:  
		txt = QString("Fatal: %1").arg(msg);  
		abort();  
	}  

	QFile outFile("debug.log");  
	outFile.open(QIODevice::WriteOnly | QIODevice::Append);  
	QTextStream ts(&outFile);  
	ts << txt << endl; 
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	//debug log
	qInstallMsgHandler(customMessageHandler);
	QFile outFile("debug.log");  
	outFile.open(QIODevice::WriteOnly | QIODevice::Append);  
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
	QTimer::singleShot(1000,&eventloop,SLOT(quit()));
	eventloop.exec();
    view.showMaximized(); 
    return a.exec();
}
