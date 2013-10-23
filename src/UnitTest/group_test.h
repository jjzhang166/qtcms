#ifndef GROUP_TEST_H
#define GROUP_TEST_H

#include <QObject>
#include <QString>
#include <QtTest>

class group_test : public QObject
{
	Q_OBJECT

public:
	group_test();
	~group_test();

private:
	void beforeGroupTest();
	int beforeChannelTest();
	int beforeGroupChannelTest(int group_id,int channel_id);
private Q_SLOTS:
	void Group_AddGroup_test();
	void Group_IsGroupExist_test();
	void Group_RemoveGroup_test();
	void Group_ModifyGroup_test();
	void Group_GetGroupCount_test();
	void Group_GetGroupList_test();
	void Group_GetGroupName_int_test();
	void Group_GetGroupName_QString_test();

	void Group_IsChannelExists_test();
	void Group_IsR_Channel_GroupExist_test();
	void Group_AddChannelInGroup_test();
	void Group_RemoveChannelFromGroup_test();
	void Group_ModifyGroupChannelName_test();
	void Group_MoveChannelToGroup_test();
	void Group_GetGroupChannelName_int_test();
	void Group_GetGroupChannelCount_test();
	void Group_GetGroupChannelList_test();
	void Group_GetGroupChannelName_qstring_test();
	void Group_GetChannelIdFromGroup_test();
	void Group_GetChannelIdFromGroup_one_test();
	void Group_GetChannelInfoFromGroup_int_test();
	void Group_GetChannelInfoFromGroup_QVariant_test();
};

#endif // GROUP_TEST_H
