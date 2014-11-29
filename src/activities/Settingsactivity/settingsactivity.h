#ifndef TESTACTIVITY_H
#define TESTACTIVITY_H

#include "settingsactivity_global.h"
#include <IActivities.h>
#include <qwfw.h>
#include <IUserManager.h>
#include <IDeviceManager.h>
#include <QDomDocument>
#include <IAreaManager.h>
#include <IGroupManager.h>
#include <IChannelManager.h>
#include <IDisksSetting.h>
#include <ISetRecordTime.h>
#include <ILocalSetting.h>
#include <QMutex>
#include <QObject>
#include <QRect>
#include <QPoint>
#include <QThread>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/qdesktopwidget.h>
#include "SettingsactivityThread.h"
class settingsActivity : public QWebUiFWBase,
	public IActivities
{
	Q_OBJECT
public:
	settingsActivity();

public:
	virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	virtual void Active( QWebFrame * frame);

public slots:
	void OnTopActDbClick();
	void OnMouseDown();
	void OnMouseUp();
	void OnMouseMove();
	void OnMaxClick();
	void OnMinClick();
	void OnCloseClick();

	/*user module*/
	void OnAddUserOk();
	void OnModifyUserOk();
	void OnDeleteUserOk();
	
	/*device module*/
	void OnAddDevice();
	void OnAddDeviceDouble();
	void OnAddDeviceALL();
	void OnAddDeviceALLThread();
	void OnRemoveDevice();
	void OnModifyDevice();
	void OnModifyDeviceEx();
	void OnRemoveDeviceALL();
	void OnRemoveDeviceALLThread();

	/*group module*/
	void OnAddGroup();
	void OnRemoveGroup();
	void OnModifyGroup();

	/*area module*/
	void OnAddArea();
	void OnRemoveArea();
	void OnModifyArea();

	/*channel module*/
	void OnAddChannel();
	void OnRemoveChannel();
	void OnModifyChannel();

	/*channel in group module*/
	void OnAddChannelInGroup();
	void OnAddChannelInGroupDouble();
	void OnRemoveChannelFromGroup();
	void OnModifyGroupChannelName();
	/*录像存储设置管理*/
	void OnSettingStorageParm();
	/*通用设置管理*/
	void OnSettingCommonParm();
	/*录像时间设置*/
	void OnSettingRecordTimeParm();
	void OnSettingRecordTimeParmDouble();
	void OnSettingRecordTimeParmCopy();
private:
	QWidget * m_MainView;
	int m_nRef;
	QMutex m_csRef;
 	bool m_bMouseTrace;
	QPoint m_pos;
	SettingsactivityThread *m_SettingThread;
	bool m_bIsRemoving;
    bool m_bIsAdding;
signals:
	void DeleteAllDevSig();

};

#endif // TESTACTIVITY_H
