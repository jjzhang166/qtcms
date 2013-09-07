#include "AvLibDll.h"
#include <QLibrary>
#include <QDir>
#include <QtGui/QApplication>

CAvLibDll::CAvLibDll(void)
{
	QString sAppPath = QApplication::applicationDirPath();
	QString dllPath = sAppPath + QString("/") + QString("avlib.dll");
	qDebug("dllpath:%s",dllPath.toLatin1().data());
	QLibrary mylib(dllPath);
	if (mylib.load())
	{
		avsdk_init = (lpavsdk_init)mylib.resolve("avsdk_init");
		CreateVideoDecoder = (lpCreateVideoDecoder)mylib.resolve("CreateVideoDecoder");
		VideoDecoderInit = (lpVideoDecoderInit)mylib.resolve("VideoDecoderInit");
		VideoDecoderDecode = (lpVideoDecoderDecode)mylib.resolve("VideoDecoderDecode");
		VideoDeocderRelease = (lpVideoDeocderRelease)mylib.resolve("VideoDeocderRelease");
		qDebug("load success!!!");
	}else
	{
		qDebug("dll load failed!!!");
	}
	avsdk_init();
}

CAvLibDll::~CAvLibDll(void)
{
	
}

