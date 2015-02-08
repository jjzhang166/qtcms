#include "userInfoObject.h"
#include "guid.h"
#include <QFileDialog>
#include <QDebug>
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
	sFilePath=QFileDialog::getOpenFileName( this,"test xxx",
		"",
		"file (*.XML)"); 
	if (!sFilePath.isEmpty())
	{
		if (NULL!=m_pConfigManager)
		{
			m_pConfigManager->Import(sFilePath);
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
			m_pConfigManager->Export(sFilePath);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"outportUserInfo fail as m_pConfigManager is null";
		}
	}

}
