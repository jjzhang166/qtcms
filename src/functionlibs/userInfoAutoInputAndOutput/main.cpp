
#include <QtCore/QCoreApplication>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include "manipulateUserInfo.h"
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QString sParam;
	for (int i=0;i<argc;i++)
	{
		if (i==1)
		{
			sParam=argv[i];
			break;
		}
	}
	manipulateUserInfo tManipulateUserInfo;
	if (sParam=="1")
	{
		tManipulateUserInfo.outputUserInfo();
	}else if (sParam=="2")
	{
		tManipulateUserInfo.inputUserInfo();
	}else{
		//do nothing
	}
	return 0;
}
