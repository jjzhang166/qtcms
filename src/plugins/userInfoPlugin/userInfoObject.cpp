#include "userInfoObject.h"
#include "guid.h"
#include <QFileDialog>
#include <QDebug>
#include <QDesktopServices>
userInfoObject::userInfoObject(QWidget *parent)	: QWidget(parent),
	QWebPluginFWBase(this),
	m_pConfigManager(NULL)
{
	pcomCreateInstance(CLSID_ConfigMgr,NULL,IID_IConfigManager,(void**)&m_pConfigManager);
}


userInfoObject::~userInfoObject()
{
	if (NULL!=m_pConfigManager)
	{
		m_pConfigManager->Release();
		m_pConfigManager=NULL;
	}
}

void userInfoObject::importUserInfo()
{
	QString sFilePath;
	sFilePath=QFileDialog::getOpenFileName( this,"",
		"",
		"file (*.XML)"); 
	if (!sFilePath.isEmpty())
	{
		if (NULL!=m_pConfigManager)
		{
			QVariantMap tMap;
			if (0==m_pConfigManager->Import(sFilePath))
			{
				//执行成功
				tMap.insert("flags",true);
				tMap.insert("file",sFilePath);
				EventProcCall("importUserInfoCallback",tMap);
			}else{
				//执行失败
				tMap.insert("flags",false);
				tMap.insert("file",sFilePath);
				EventProcCall("importUserInfoCallback",tMap);
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"importUserInfo fail as m_pConfigManager is null";
		}
	}
}

void userInfoObject::outportUserInfo()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		"",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		QString sFilePath=dir+"/userInfo.xml";
		if (NULL!=m_pConfigManager)
		{
			QVariantMap tMap;
			if (0==m_pConfigManager->Export(sFilePath))
			{
				//执行成功
				tMap.insert("flags",true);
				tMap.insert("file",sFilePath);
				EventProcCall("outportUserInfoCallback",tMap);
			}else{
				//执行失败
				tMap.insert("flags",false);
				tMap.insert("file",sFilePath);
				EventProcCall("outportUserInfoCallback",tMap);
			}

			//同时保存到默认的路径
			saveToDefaultPath();
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"outportUserInfo fail as m_pConfigManager is null";
		}
	}

}

void userInfoObject::saveToDefaultPath()
{
	QString sFilePath;
	if (getOutputUserInfoFilePath(sFilePath))
	{
		if (NULL!=m_pConfigManager)
		{
			m_pConfigManager->Export(sFilePath);
		}else{
			//do nothing
		}
	}else{
		//do nothing
	}
}

bool userInfoObject::getOutputUserInfoFilePath( QString &sFilePath )
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
