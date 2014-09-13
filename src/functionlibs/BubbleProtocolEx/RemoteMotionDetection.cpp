#include "bubbleprotocolex.h"

void mdSignal(QString sEvent,QVariantMap eParam,void * pUser)
{
	BubbleProtocolEx * pTemp = (BubbleProtocolEx *)pUser;
	pTemp->mdSignal(sEvent,eParam);
}

void BubbleProtocolEx::mdSignal(QString sEvent,QVariantMap eParam)
{
	QVariantMap paramTemp;
	if (eParam["signal"].toBool())
	{
		paramTemp.insert("signal",QVariant(true));
	}
	else
	{
		paramTemp.insert("signal",QVariant(false));
	}
	eventProcCall("MDSignal",paramTemp);
}

int BubbleProtocolEx::startMotionDetection()
{
	m_MdWorkObj.registerEvent("mdsignal",::mdSignal,this);
	m_MdWorkObj.setHostInfo(m_tDeviceInfo.tIpAddr.toString(),m_tDeviceInfo.tPorts["http"].toInt());
	m_MdWorkObj.setUserInfo(m_tDeviceInfo.sUserName,m_tDeviceInfo.sPassword);
	return m_MdWorkObj.startMd();
}

int BubbleProtocolEx::stopMotionDetection()
{
	return m_MdWorkObj.stopMd();
}
