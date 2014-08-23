#include "rPlaybackWndEx.h"
#include <QDebug>
#include <qwfw_tools.h>
#include <guid.h>
rPlaybackWndEx::rPlaybackWndEx(QWidget *parent):QWidget(parent),
	QWebPluginFWBase(this)
{
}


rPlaybackWndEx::~rPlaybackWndEx(void)
{
}

void rPlaybackWndEx::resizeEvent( QResizeEvent * )
{

}

int rPlaybackWndEx::setDeviceHostInfo( const QString & sAddress,unsigned int uiPort,const QString &sEseeID )
{
	if (!(m_tDeviceInfo.tHostAddress.setAddress(sAddress)||uiPort>65535))
	{
		m_tDeviceInfo.sAddress=sAddress;
		m_tDeviceInfo.uiPort=uiPort;
		m_tDeviceInfo.sEseeId=sEseeID;
		if (m_tPlaybackRunEx.setDeviceHostInfo(sAddress,uiPort,sEseeID)==0)
		{
			return 0;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHostInfo fail as m_tPlaybackRunEx.setDeviceHostInfo fail";
			return 1;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHostInfo fail as the input param is unCorrect";
		return 1;
	}
}

int rPlaybackWndEx::setDeviceVendor( const QString & sVendor )
{
	if (!sVendor.isEmpty())
	{
		if (m_tPlaybackRunEx.setDevcieVendor(sVendor)==0)
		{
			m_tDeviceInfo.sVendor=sVendor;
			return 0;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceVendor fail as m_tPlaybackRunEx.setDevcieVendor fail";
			return 1;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceVendor as sVendor isEmpty";
		return 1;
	}
}

int rPlaybackWndEx::AddChannelIntoPlayGroup( uint uiWndId,int uiChannelId )
{
	//fix me
	if (!(uiWndId>ARRAY_SIZE(m_tPlaybackWnd)||m_tDeviceInfo.uiChannelId>32))
	{
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"";
	}
	return 1;
}

int rPlaybackWndEx::GetWndInfo( int uiWndId )
{
	return 1;
}

void rPlaybackWndEx::setUserVerifyInfo( const QString & sUsername,const QString & sPassword )
{

}

int rPlaybackWndEx::startSearchRecFile( int nChannel,int nTypes,const QString & sStartTime,const QString & sEndTime )
{
	return 1;
}

QString rPlaybackWndEx::GetNowPlayedTime()
{
	return "";
}

int rPlaybackWndEx::GroupPlay( int nTypes,const QString & sStart,const QString & sEnd )
{
	return 1;
}

int rPlaybackWndEx::GroupPause()
{
	return 1;
}

int rPlaybackWndEx::GroupContinue()
{
	return 1;
}

int rPlaybackWndEx::GroupStop()
{
	return 1;
}

int rPlaybackWndEx::AudioEnabled( bool bEnable )
{
	return 1;
}

int rPlaybackWndEx::SetVolume( const unsigned int &uiPersent )
{
	return 1;
}

int rPlaybackWndEx::GroupSpeedFast()
{
	return 1;
}

int rPlaybackWndEx::GroupSpeedSlow()
{
	return 1;
}

int rPlaybackWndEx::GroupSpeedNormal()
{
	return 1;
}

int rPlaybackWndEx::GetCurrentState()
{
	return 1;
}

bool rPlaybackWndEx::getChannelInfo( int nChlId )
{
	IChannelManager *pChannelManager=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		QVariantMap tChannelInfo=pChannelManager->GetChannelInfo(nChlId);
		m_tDeviceInfo.uiStream=tChannelInfo.value("stream").toInt();
		m_tDeviceInfo.uiChannelId=tChannelInfo.value("number").toInt();
		m_tDeviceInfo.sDeviceName=tChannelInfo.value("name").toString();
		int nDeviceId=tChannelInfo.value("dev_id").toInt();
		pChannelManager->Release();
		pChannelManager=NULL;
		IDeviceManager *pDeviceManager=NULL;
		pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void**)&pDeviceManager);
		if (NULL!=pDeviceManager)
		{
			QVariantMap tDeviceInfo=pDeviceManager->GetDeviceInfo(nDeviceId);
			m_tDeviceInfo.sVendor=tDeviceInfo.value("vendor").toString();
			m_tDeviceInfo.sPassword=tDeviceInfo.value("password").toString();
			m_tDeviceInfo.sUserName=tDeviceInfo.value("username").toString();
			m_tDeviceInfo.sEseeId=tDeviceInfo.value("eseeid").toString();
			m_tDeviceInfo.sAddress=tDeviceInfo.value("address").toString();
			m_tDeviceInfo.uiPort=tDeviceInfo.value("port").toInt();
			pDeviceManager->Release();
			pDeviceManager=NULL;
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getChannelInfo fail as pDeviceManager is null";
			return false;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getChannelInfo fail as pChannelManager is null";
		return false;
	}
}
