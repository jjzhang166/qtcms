#include "HiH264Dll.h"
#include <QDir>
#include <QtGui/QApplication>

// #include "hi_config.h"
// #include "hi_h264api.h"

HiH264Dll HiH264Dec_dll;

HiH264Dll::HiH264Dll(void)
{
	QString sAppPath = QApplication::applicationDirPath();
	QString dllPath = sAppPath + QString("/") + QString("hi_h264dec_w.dll");
	qDebug("dllpath:%s",dllPath.toLatin1().data());
	mylib.setFileName(dllPath);
	if (mylib.load())
	{
		DecCreate = (lpHi264DecCreate)mylib.resolve("Hi264DecCreate");
		DecDestroy = (lpHi264DecDestroy)mylib.resolve("Hi264DecDestroy");
		DecFrame = (lpHi264DecFrame)mylib.resolve("Hi264DecFrame");
		qDebug("load success!!!");
	}else
	{
		qDebug("dll load failed!!!");
	}
}

HiH264Dll::~HiH264Dll(void)
{
	
}

