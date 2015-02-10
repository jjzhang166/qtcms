#include "manipulateUserInfo.h"
#include <QtCore/QCoreApplication>
#include "guid.h"
#include <QDebug>
#include <QFile>
#include <QDesktopServices>
#include <QDir>
manipulateUserInfo::manipulateUserInfo(void):m_pIConfigManager(NULL)
{
	pcomCreateInstance(CLSID_ConfigMgr,NULL,IID_IConfigManager,(void**)&m_pIConfigManager);
}


manipulateUserInfo::~manipulateUserInfo(void)
{
}

void manipulateUserInfo::inputUserInfo()
{
	QString sFilePath;
	if (NULL!=m_pIConfigManager)
	{
		if (getInputUserInfoFilePath(sFilePath))
		{
			m_pIConfigManager->Import(sFilePath);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"inputUserInfo fail as getInputUserInfoFilePath fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"inputUserInfo as m_pIConfigManager is null";
	}
}

void manipulateUserInfo::outputUserInfo()
{
	QString sFilePath;
	if (NULL!=m_pIConfigManager)
	{
		if (getOutputUserInfoFilePath(sFilePath))
		{
			m_pIConfigManager->Export(sFilePath);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"outputUserInfo fail as getOutputUserInfoFilePath fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"outputUserInfo as m_pIConfigManager is null";
	}
}

bool manipulateUserInfo::getOutputUserInfoFilePath(QString &sFilePath)
{
	QString sCurrentDir=QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	sCurrentDir=sCurrentDir+"/userInfo";
	QString sUserInfoFilePath=sCurrentDir+"/userInfo.xml";
	QFile tFile;
	tFile.setFileName(sUserInfoFilePath);
	if (tFile.exists())
	{
		//do nothing
		sFilePath=sUserInfoFilePath;
	}else{
		QDir tDir;
		bool bDir=false;
		if (!tDir.exists(sCurrentDir))
		{
			bDir=tDir.mkdir(sCurrentDir);
		}else{
			//do nothing
			bDir=true;
		}
		if (bDir)
		{
			if (tFile.open(QIODevice::WriteOnly))
			{
				sFilePath=sUserInfoFilePath;
				tFile.close();
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"open file fail"<<sUserInfoFilePath;
				return false;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getOutputUserInfoFilePath fail as dir do not exists"<<sCurrentDir;
			return false;
		}
	}
	return true;
}

bool manipulateUserInfo::getInputUserInfoFilePath(QString &sFilePath)
{
	QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QString sCurrentDir=QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	sCurrentDir=sCurrentDir+"/userInfo";
	QString sUserInfoFilePath=sCurrentDir+"/userInfo.xml";
	QFile tFile;
	tFile.setFileName(sUserInfoFilePath);
	if (tFile.exists())
	{
		sFilePath=sUserInfoFilePath;
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"file do not exists"<<sUserInfoFilePath;
		return false;
	}
}
