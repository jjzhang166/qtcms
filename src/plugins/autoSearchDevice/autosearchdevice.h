#ifndef AUTOSEARCHDEVICE_H
#define AUTOSEARCHDEVICE_H

#include "autosearchdevice_global.h"
#include "IDeviceManager.h"
#include "IAutoSearchDevice.h"
#include <IUserManagerEx.h>
#include "guid.h"
#include <qwfw.h>
#include <autoSearchDeviceWindow.h>
#include <QTimer>
#include <QList>
#include <QTimer>

class  autoSearchDevice:public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	autoSearchDevice();
	~autoSearchDevice();
public:
	void autoSearchDeviceCb(QVariantMap tItem);
public slots:
		void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}
		//自动搜索 设备接口
		void startAutoSearchDevice(int nTime,int nWidth,int nHeight);
		void cancelSearch();
		void autoSearchDeviceTimeout();

		//用户登录接口
		int showUserLoginUi(int nWidth,int nHeight);//0：用户登录，1：用户注销，2：关闭页面（没有进行用户登录操作）
		int checkUserLimit(quint64 uiCode,quint64 uiSubCode);//0:认证通过 1：用户没有登录 2：用户权限不足
		void cancelLoginUI();//退出用户登录界面
		int login(QString sUserName,QString sPassword,int nCode);//0:操作成功 1：操作失败；nCode:0(用户登录)，1（用户注销）
		int loginEx();
		QStringList getUserList();
		QVariantMap getUserLimit(QString sUserName);
		QString getCurrentUser();
		int getUserInDatabaseId(QString sUserName);//返回值为 ID，获取失败返回 -1；
		void setIsKeepCurrentUserPassWord(bool bFlags);
		QVariantMap getIsKeepCurrentUserPassWord();
		void startGetUserLoginStateChangeTime();
		int getLoginOutInterval(QString sUserName);
private slots:
	void slCheckUserStatusChange();
private:
	autoSearchDeviceWindow m_tAutoSearchDeviceWindow;
	IAutoSearchDevice *m_pDeviceSearch;
	IUserManagerEx *m_pUserMangerEx;
	QList<QVariantMap> m_tDeviceList;
	QTimer m_tCheckUserStatsTimer;
	QString g_sHisUserName;
	bool m_bCancelAutoSearchDevice;
	bool m_bCallHide;
	int m_nLoginRet;
};

#endif // AUTOSEARCHDEVICE_H
