#ifndef __ICHANNELMANAGER_HEAD_FILE__09SYV89H0QASNDVP__
#define __ICHANNELMANAGER_HEAD_FILE__09SYV89H0QASNDVP__

#include "libpcom.h"
#include <QtCore/QString>
#include <QtCore/QVariantMap>

interface IChannelManager : public IPComBase
{
	virtual int ModifyChannelName(int chl_id,QString sName) = 0;
	virtual int ModifyChannelStream(int chl_id,int nStream) = 0;
	virtual int GetChannelCount(int dev_id) = 0;
	virtual QStringList GetChannelList(int dev_id) = 0;
	virtual int GetChannelName(int chl_id,QString & sName) = 0;
	virtual int GetChannelStream(int chl_id,int & nStream) = 0;
	virtual int GetChannelNumber(int chl_id,int & nChannelNum) = 0;
	virtual int GetChannelInfo(int chl_id,QString &sName,int &nStream,int &nChannelNum) = 0;
	virtual QVariantMap GetChannelInfo(int chl_id) = 0;

	enum _enError{
		OK,
		E_CHANNEL_NOT_FOUND,
		E_SYSTEM_FAILED
	};
};

#endif