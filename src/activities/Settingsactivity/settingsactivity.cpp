#include "settingsactivity.h"
#include <guid.h>
#include <QDebug>

settingsActivity::settingsActivity():
	m_nRef(0),
	m_bMouseTrace(false)
{

}

long __stdcall settingsActivity::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IActivities == iid)
	{
		*ppv = static_cast<IActivities *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall settingsActivity::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	qDebug("Addref:%d",m_nRef);
	return m_nRef;
}

unsigned long __stdcall settingsActivity::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

void settingsActivity::Active( QWebFrame * frame)
{
	m_MainView = (frame->page())->view();
	m_MainView->setMouseTracking(true);
	QWFW_MSGMAP_BEGIN(frame);
	QWFW_MSGMAP("top_act","dblclick","OnTopActDbClick()");
	QWFW_MSGMAP("top_act","mousedown","OnMouseDown()");
	QWFW_MSGMAP("top_act","mouseup","OnMouseUp()");
	QWFW_MSGMAP("top_act","mousemove","OnMouseMove()");
	QWFW_MSGMAP("max","click","OnMaxClick()");
	QWFW_MSGMAP("min","click","OnMinClick()");
	QWFW_MSGMAP("close","click","OnCloseClick()");
	QWFW_MSGMAP("add_ok","click","OnAddUserOk()");
	QWFW_MSGMAP("modify_ok","click","OnModifyUserOk()");
	QWFW_MSGMAP("delete_ok","click","OnDeleteUserOk()");
	QWFW_MSGMAP("AddDevice_ok","click","OnAddDevice()");
	QWFW_MSGMAP("RemoveDevice_ok","click","OnRemoveDevice()");
	QWFW_MSGMAP("ModifyDevice_ok","click","OnModifyDevice()");
	QWFW_MSGMAP_END;
}

void settingsActivity::OnTopActDbClick()
{
	if (m_MainView->isMaximized())
	{
		m_MainView->showNormal();
		if (m_MainView->frameGeometry().width() < 950)
		{
			m_MainView->setFixedWidth(950);
		}
	}
	else
	{
		m_MainView->showMaximized();
	}
}

void settingsActivity::OnMinClick()
{
	m_MainView->showMinimized();
}

void settingsActivity::OnMaxClick()
{
	if (m_MainView->isMaximized())
	{
		m_MainView->showNormal();
		if (m_MainView->frameGeometry().width() < 950)
		{
			m_MainView->setFixedWidth(950);
		}
	}else
	{
		m_MainView->showMaximized();
	}
}

void settingsActivity::OnCloseClick()
{
	m_MainView->close();
}

void settingsActivity::OnMouseDown()
{
	m_pos = QCursor::pos();
	m_bMouseTrace = true;
}

void settingsActivity::OnMouseUp()
{
	m_bMouseTrace = false;
}

void settingsActivity::OnMouseMove()
{
	int nMainFormXcoord = m_MainView->frameGeometry().topLeft().x();
	int nMainFormYcoord = m_MainView->frameGeometry().topLeft().y();
	if (m_bMouseTrace)
	{
		if (! m_MainView->isMaximized())
		{
			QPoint curPos = QCursor::pos();
			int dx = curPos.x() - m_pos.x();
			int dy = curPos.y() - m_pos.y();
			nMainFormXcoord += dx;
			nMainFormYcoord += dy;
			m_MainView->move(nMainFormXcoord,nMainFormYcoord);
			m_pos = curPos;
		}
	}
}

void settingsActivity::OnAddUserOk()
{
	IUserManager *Iuser = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IUserManager,(void **)&Iuser);
	if (NULL == Iuser)
	{
		return;
	}
	QVariant sUserName = QueryValue("add_username");
	if (Iuser->IsUserExists(sUserName.toString()))
	{
		DEF_EVENT_PARAM(arg);
		EP_ADD_PARAM(arg,"username",sUserName.toString());
		EventProcCall("AddFaild",arg);
		return;
	}

	QVariant sPassWd = QueryValue("add_passwd");
	QVariant sPassWd2 = QueryValue("add_again_passwd");
	QVariant sLevel = QueryValue("add_level");
	int nLevel = sLevel.toInt();
	int nMask1 = 0xFFFFFFFF;
	int nMask2 = 0xFFFFFFFF;
	bool bRes = Iuser->AddUser(sUserName.toString(),sPassWd.toString(),nLevel,nMask1,nMask2);
	
	if (!bRes)
	{
		DEF_EVENT_PARAM(arg);
		EP_ADD_PARAM(arg,"username",sUserName.toString());
		EP_ADD_PARAM(arg,"level",nLevel);
		EventProcCall("AddSuccess",arg);
	}
	else
	{
		DEF_EVENT_PARAM(arg);
		EP_ADD_PARAM(arg,"username",sUserName.toString());
		EP_ADD_PARAM(arg,"level",nLevel);
		EventProcCall("AddFaild",arg);
	}
	
	Iuser->Release();
}

void settingsActivity::OnModifyUserOk()
{
	IUserManager *Iuser = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IUserManager,(void **)&Iuser);
	if (NULL == Iuser)
	{
		return;
	}
	QVariant sUserName = QueryValue("user");
	QVariant sOldPassWd = QueryValue("modify_oldpasswd");
	//检查输入密码是否正确
 	if (Iuser->CheckUser(sUserName.toString(),sOldPassWd.toString()))
 	{
		QVariant sNewPassWd = QueryValue("modify_newpasswd");
		QVariant sNewPassWd2 = QueryValue("modify_again_passwd");
		//获取combox里的权限值，修改权限
		QVariant sLevel = QueryValue("modify_level");
		int nLevel = sLevel.toInt();
		if (!(sNewPassWd.toString().isEmpty() || sNewPassWd2.toString().isEmpty()))
		{
			bool bRes2 = Iuser->ModifyUserPassword(sUserName.toString(),sNewPassWd.toString());	
			if (!bRes2)
			{
				DEF_EVENT_PARAM(arg);
				EP_ADD_PARAM(arg,"username",sUserName.toString());
				EventProcCall("ModifyUserPasswdSuccess",arg);
			}else
			{
				DEF_EVENT_PARAM(arg);
				EP_ADD_PARAM(arg,"username",sUserName.toString());
				EventProcCall("ModifyUserPasswdFaild",arg);
			}
		}
		bool bRes = Iuser->ModifyUserLevel(sUserName.toString(),nLevel);
		if (!bRes)
		{
			DEF_EVENT_PARAM(arg);
			EP_ADD_PARAM(arg,"username",sUserName.toString());
			EP_ADD_PARAM(arg,"level",nLevel);
			EventProcCall("ModifyUserLevelSuccess",arg);
		}else
		{
			DEF_EVENT_PARAM(arg);
			EP_ADD_PARAM(arg,"username",sUserName.toString());
			EP_ADD_PARAM(arg,"level",nLevel);
			EventProcCall("ModifyUserLevelFaild",arg);
		}	
	}
	else
	{
		//event
		DEF_EVENT_PARAM(arg);
		EP_ADD_PARAM(arg,"username",sUserName);
		EP_ADD_PARAM(arg,"password",sOldPassWd);
		EventProcCall("ErrorPasswd",arg);
	}
	Iuser->Release();
}

void settingsActivity::OnDeleteUserOk()
{
	IUserManager *Iuser = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IUserManager,(void **)&Iuser);
	//获取用户列表
	QVariant sUser = QueryValue("user");
	QStringList user_list = sUser.toString().split(",");
	for (int i = 0;i<user_list.size();i++)
	{
		sUser = user_list.at(i).toLatin1().data();
		bool bRes = Iuser->RemoveUser(sUser.toString());
		if (!bRes)
		{
			DEF_EVENT_PARAM(arg);
			EP_ADD_PARAM(arg,"username",sUser.toString());
			EventProcCall("DeleteSuccess",arg);
		}else
		{
			DEF_EVENT_PARAM(arg);
			EP_ADD_PARAM(arg,"username",sUser.toString());
			EventProcCall("DeleteFaild",arg);
		}
	}
	Iuser->Release();
}

/*device module*/
void settingsActivity::OnAddDevice()
{
	IDeviceManager *Idevice=NULL;
	IAreaManager *Iarea=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void**)&Idevice);
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IAreaManager,(void**)&Iarea);
	if(NULL==Idevice||NULL==Iarea){
		DEF_EVENT_PARAM(arg);
		QString Content="system fail";
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddDeviceFail",arg);
		Idevice->Release();
		return;
	}
	QVariant Area_Id=QueryValue("Device_area_id");
	QVariant sDeviceName=QueryValue("Device_sDeviceName");
	QVariant sAddress=QueryValue("Device_sAddress");
	QVariant port=QueryValue("Device_port");
	QVariant http=QueryValue("Device_http");
	QVariant sEseeId=QueryValue("Device_sEseeid");
	QVariant sUserName=QueryValue("Device_sUsername");
	QVariant sPassWord=QueryValue("Device_sPassword");
	QVariant chlCount=QueryValue("Device_chlCount");
	QVariant ConnectMethod=QueryValue("Device_connectMethod");
	QVariant sVendor=QueryValue("Device_sVendor");

	DEF_EVENT_PARAM(arg);
	QString Content;

	bool nRet_bool=false;
	nRet_bool=Iarea->IsAreaIdExist(Area_Id.toInt());
	if(false==nRet_bool){
		Content="AreaID is not exist";
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddDeviceFail",arg);
		Idevice->Release();
		return;
	}
	if(sDeviceName.isNull()||sUserName.isNull()||chlCount.isNull()||ConnectMethod.isNull()||sVendor.isNull()){
		Content.clear();
		arg.clear();
		Content.append("the params are not complete");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddDeviceFail",arg);
		Idevice->Release();
		return;
	}
	if("IP"==ConnectMethod.toString()){
		if(sAddress.isNull()||port.isNull()||http.isNull()){
			Content.clear();
			arg.clear();
			Content.append("the params are not complete");
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("AddDeviceFail",arg);
			Idevice->Release();
			return;
		}
		int nRet_int=Idevice->AddDevice(Area_Id.toInt(),sDeviceName.toString(),sAddress.toString(),port.toInt(),http.toInt(),sEseeId.toString(),sUserName.toString(),sPassWord.toString(),chlCount.toInt(),ConnectMethod.toInt(),sVendor.toString());
		if(0!=nRet_int){
			Content.clear();
			arg.clear();
			Content.append("AddDeviceFail");
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("AddDeviceFail",arg);
			Idevice->Release();
			return;
		}
		goto end1;
	}
	else if("sEsee"==ConnectMethod.toString()){
		if(sEseeId.isNull()){
			Content.clear();
			arg.clear();
			Content.append("the params are not complete");
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("AddDeviceFail",arg);
			Idevice->Release();
			return;
		}
		int nRet_int=Idevice->AddDevice(Area_Id.toInt(),sDeviceName.toString(),sAddress.toString(),port.toInt(),http.toInt(),sEseeId.toString(),sUserName.toString(),sPassWord.toString(),chlCount.toInt(),ConnectMethod.toInt(),sVendor.toString());
		if(0!=nRet_int){
			Content.clear();
			arg.clear();
			Content.append("AddDeviceFail");
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("AddDeviceFail",arg);
			Idevice->Release();
			return;
		}
		goto end1;
	}

	Content.clear();
	arg.clear();
	Content.append("ConnectMethod fail");
	EP_ADD_PARAM(arg,"fail",Content);
	Idevice->Release();
	return;
end1:
	Content.clear();
	arg.clear();
	Content.append("add device success");
	EP_ADD_PARAM(arg,"success",Content);
	Idevice->Release();
	return;
}

void settingsActivity::OnRemoveDevice()
{
	IDeviceManager *Idevice=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void**)&Idevice);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if(NULL==Idevice){
		Content.append("system");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("RemoveDeviceFail",arg);
		Idevice->Release();
		return;
	}


	QVariant Dev_Id=QueryValue("Device_id");
	bool nRet_bool=false;
	nRet_bool=Idevice->IsDeviceExist(Dev_Id.toInt());
	if(false==nRet_bool){
		arg.clear();
		Content.clear();
		Content.append("Dev_Id is not exist");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("RemoveDeviceFail",arg);
		Idevice->Release();
		return;
	}
	int nRet_int=-1;
	nRet_int=Idevice->RemoveDevice(Dev_Id.toInt());
	if(0!=nRet_int){
		arg.clear();
		Content.clear();
		Content.append("RemoveFail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("RemoveDeviceFail",arg);
		Idevice->Release();
		return;
	}
	arg.clear();
	Content.clear();
	Content.append("Remove Success");
	EP_ADD_PARAM(arg,"success",Content);
	EventProcCall("RemoveDeviceSuccess",arg);
	Idevice->Release();
	return;
}

void settingsActivity::OnModifyDevice()
{
	IDeviceManager *Idevice=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void**)&Idevice);
	DEF_EVENT_PARAM(arg);
	QString Content="system fail";
	if(NULL==Idevice){
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("ModifyDeviceFail",arg);
		return;
	}

	QVariant Dev_Id=QueryValue("Device_id");
	bool nRet_bool=Idevice->IsDeviceExist(Dev_Id.toInt());
	if(false==nRet_bool){
		arg.clear();
		Content.clear();
		Content.append("Dev_Id is not exist");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("ModifyDeviceFail",arg);
		Idevice->Release();
		return;
	}

	QVariant sDeviceName=QueryValue("Device_sDeviceName");
	if(false==Dev_Id.isNull()&&false==sDeviceName.isNull()){
		int nRet_int=Idevice->ModifyDeviceName(Dev_Id.toInt(),sDeviceName.toString());
		if(0!=nRet_int){
			arg.clear();
			Content.clear();
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("ModifyDeviceFail",arg);
			Idevice->Release();
			return;
		}
	}

	QVariant sUsername=QueryValue("Device_sUsername");
	QVariant sPassword=QueryValue("Device_sPassword");
	if(false==sUsername.isNull()){
		int nRet_int=Idevice->ModifyDeviceAuthority(Dev_Id.toInt(),sUsername.toString(),sPassword.toString());
		if(0!=nRet_int){
			arg.clear();
			Content.clear();
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("ModifyDeviceFail",arg);
			Idevice->Release();
			return;
		}
	}

	QVariant chlCount=QueryValue("Device_chlCount");
	if(false==chlCount.isNull()){
		int nRet_int=Idevice->ModifyDeviceChannelCount(Dev_Id.toInt(),chlCount.toInt());
		if(0!=nRet_int){
			arg.clear();
			Content.clear();
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("ModifyDeviceFail",arg);
			Idevice->Release();
			return;
		}
	}

	QVariant sVentor=QueryValue("Device_sVendor");
	if(false==sVentor.isNull()){
		int nRet_int=Idevice->ModifyDeviceVendor(Dev_Id.toInt(),sVentor.toString());
		if(0!=nRet_int){
			arg.clear();
			Content.clear();
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("ModifyDeviceFail",arg);
			Idevice->Release();
			return;
		}
	}

	QVariant sEseeId=QueryValue("Device_sEseeid");
	if(false==sEseeId.isNull()){
		int nRet_int=Idevice->ModifyDeviceEseeId(Dev_Id.toInt(),sEseeId.toString());
		if(0!=nRet_int){
			arg.clear();
			Content.clear();
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("ModifyDeviceFail",arg);
			Idevice->Release();
			return;
		}
	}

	QVariant sAddress=QueryValue("Device_sAddress");
	QVariant port=QueryValue("Device_port");
	QVariant http=QueryValue(",Device_http");
	if(false==sAddress.isNull()&&false==port.isNull()&&http.isNull()){
		int nRet_int=Idevice->ModifyDeviceHost(Dev_Id.toInt(),sAddress.toString(),port.toInt(),http.toInt());
		if(0!=nRet_int){
			arg.clear();
			Content.clear();
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("ModifyDeviceFail",arg);
			Idevice->Release();
			return;
		}
	}

	arg.clear();
	Content.clear();
	Content.append("modify success");
	EP_ADD_PARAM(arg,"success",Content);
	EventProcCall("ModifyDeviceSuccess",arg);
	Idevice->Release();
	return;
}
