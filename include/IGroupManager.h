#ifndef __IGROUPMANAGER_HEAD_FILE__0NSDVP9QUA9FNB89A__
#define __IGROUPMANAGER_HEAD_FILE__0NSDVP9QUA9FNB89A__


#include "libpcom.h"
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

interface IGroupManager : public IPComBase
{
	virtual int AddGroup(QString sName) = 0;
	virtual int RemoveGroup(int group_id) = 0;
	virtual int ModifyGroupName(int group_id,QString sName) = 0;
	virtual int GetGroupCount() = 0;
	virtual QStringList GetGroupList() = 0;
	virtual int GetGroupName(int group_id,QString & sName) = 0;
	virtual QString GetGroupName(int group_id) = 0;
	virtual bool IsGroupExists(int group_id) = 0;

	virtual bool IsChannelExists(int chl_id) = 0;
	virtual bool IsR_Channel_GroupExist(int rgc_id)=0;
	virtual int AddChannelInGroup(int group_id,int chl_id,QString sName) = 0;
	virtual int RemoveChannelFromGroup(int rgc_id) = 0;
	virtual int ModifyGroupChannelName(int rgc_id,QString sName) = 0;
	virtual int MoveChannelToGroup(int rgc_id,int group_id) = 0;
	virtual int GetGroupChannelCount(int group_id) = 0;
	virtual QStringList GetGroupChannelList(int group_id) = 0;
	virtual int GetGroupChannelName(int rgc_id,QString & sName) = 0;
	virtual QString GetGroupChannelName(int rgc_id) = 0;
	virtual int GetChannelIdFromGroup(int rgc_id,int & chl_id) = 0;
	virtual int GetChannelIdFromGroup(int rgc_id) = 0;
	virtual int GetChannelInfoFromGroup(int rgc_id,int & chl_id, int & group_id, QString & sName) = 0;
	virtual QVariantMap GetChannelInfoFromGroup(int rgc_id) = 0;

	enum _enError{
		OK,
		E_GROUP_NOT_FOUND,
		E_CHANNEL_NOT_FOUND,
		E_CHANNEL_NOT_IN_GROUP,
		E_SYSTEM_FAILED,
	};
};




#endif