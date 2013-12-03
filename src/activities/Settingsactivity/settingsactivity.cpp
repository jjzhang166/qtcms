#include "settingsactivity.h"
#include <guid.h>
#include <QDebug>
#include <QString>

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
	QWFW_MSGMAP("AddDeviceDouble_ok","click","OnAddDeviceDouble()");
	QWFW_MSGMAP("RemoveDevice_ok","click","OnRemoveDevice()");
	QWFW_MSGMAP("ModifyDevice_ok","click","OnModifyDevice()");

	QWFW_MSGMAP("AddGroup_ok","click","OnAddGroup()");
	QWFW_MSGMAP("RemoveGroup_ok","click","OnRemoveGroup()");
	QWFW_MSGMAP("ModifyGroup_ok","click","OnModifyGroup()");

	QWFW_MSGMAP("AddArea_ok","click","OnAddArea()");
	QWFW_MSGMAP("RemoveArea_ok","click","OnRemoveArea()");
	QWFW_MSGMAP("ModifyArea_ok","click","OnModifyArea()");

	QWFW_MSGMAP("AddChannel_ok","click","OnAddChannel()");
	QWFW_MSGMAP("RemoveChannel_ok","click","OnRemoveChannel()");
	QWFW_MSGMAP("ModifyChannel_ok","click","OnModifyChannel()");

	QWFW_MSGMAP("AddChannelInGroupDouble_ok","click","OnAddChannelInGroupDouble()");
	QWFW_MSGMAP("AddChannelInGroup_ok","click","OnAddChannelInGroup()");
	QWFW_MSGMAP("RemoveChannelFromGroup_ok","click","OnRemoveChannelFromGroup()");
	QWFW_MSGMAP("ModifyGroupChannelName_ok","click","OnModifyGroupChannelName()");
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
void settingsActivity::OnAddDeviceDouble()
{
	int nRet_id;
	qDebug("========OnAddDeviceDouble========");
	IDeviceManager *Idevice=NULL;
	IAreaManager *Iarea=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void**)&Idevice);
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IAreaManager,(void**)&Iarea);

	DEF_EVENT_PARAM(arg);
	QString Content="system fail";
	QString State_num="0";
	QString sDevice_Name="0";

	if(NULL==Idevice||NULL==Iarea){
		EP_ADD_PARAM(arg,"fail",Content);
		EP_ADD_PARAM(arg,"state",State_num);
		EP_ADD_PARAM(arg,"name",sDevice_Name);
		EventProcCall("AddDeviceDoubleFail",arg);
		if(NULL!=Idevice){Idevice->Release();}
		if(NULL!=Iarea){Iarea->Release();}
		return;
	}

	QVariant DevListFile=QueryValue("adddevicedouble_ID");
	QDomDocument ConfFile;
	ConfFile.setContent(DevListFile.toString());

	QDomNode DevListInfoNode=ConfFile.elementsByTagName("devListInfo").at(0);
	QDomNodeList itemList=DevListInfoNode.childNodes();
	qDebug()<<itemList.count();
	if(0==itemList.count()){
		arg.clear();
		Content.clear();
		Content.append("please choose the device");
		EP_ADD_PARAM(arg,"fail",Content);
		EP_ADD_PARAM(arg,"state",State_num);
		EP_ADD_PARAM(arg,"name",sDevice_Name);
		EventProcCall("AddDeviceDoubleFail",arg);
		if(NULL!=Idevice){Idevice->Release();}
		if(NULL!=Iarea){Iarea->Release();}
		return;
	}
	int n;
	//判断添加的区域是否存在
	int Area_ID=ConfFile.firstChild().toElement().attribute("area_id").toInt();
	bool nRet_bool=false;
	nRet_bool=Iarea->IsAreaIdExist(Area_ID);
	if(false==nRet_bool){
		Content.clear();
		arg.clear();
		Content.append("AreaID is not exist");
		EP_ADD_PARAM(arg,"fail",Content);
		EP_ADD_PARAM(arg,"state",State_num);
		EP_ADD_PARAM(arg,"name",sDevice_Name);
		EventProcCall("AddDeviceDoubleFail",arg);
		if(NULL!=Idevice){Idevice->Release();}
		if(NULL!=Iarea){Iarea->Release();}
		return;
	}

	for (n=0;n<itemList.count();n++)
	{
		QDomNode item;
		item= itemList.at(n);
		//搜索返回的参数
		QString SearchVendor_ID=item.toElement().attribute("SearchVendor_ID");
		QString SearchDeviceName_ID=item.toElement().attribute("SearchDeviceName_ID");
		QString SearchDeviceId_ID=item.toElement().attribute("SearchDeviceId_ID");
		QString SearchDeviceModelId_ID=item.toElement().attribute("SearchDeviceModelId_ID");
		QString SearchSeeId_ID=item.toElement().attribute("SearchSeeId_ID");
		QString SearchChannelCount_ID=item.toElement().attribute("SearchChannelCount_ID");
		QString SearchIP_ID=item.toElement().attribute("SearchIP_ID");
		QString SearchMask_ID=item.toElement().attribute("SearchMask_ID");
		QString SearchMac_ID=item.toElement().attribute("SearchMac_ID");
		QString SearchGateway_ID=item.toElement().attribute("SearchGateway_ID");
		QString SearchHttpport_ID=item.toElement().attribute("SearchHttpport_ID");
		QString SearchMediaPort_ID=item.toElement().attribute("SearchMediaPort_ID");

		//添加的默认参数
	
		QString	UserName_ID=item.toElement().attribute("username");
		QString	PassWord_ID=item.toElement().attribute("password");
		//设置默认参数
		QString ConnectMethod="0";
		SearchDeviceName_ID.clear();
			if (0==SearchDeviceId_ID.size()||SearchDeviceId_ID.isNull()||-1==SearchDeviceId_ID.toInt())
			{
				SearchDeviceName_ID.append(SearchIP_ID);
			}
			else{
				SearchDeviceName_ID.append(SearchDeviceId_ID);
			}


		if (0==UserName_ID.size()||UserName_ID.isNull())
		{
			UserName_ID.append("admin");
		}


		//默认IP模式连接，判断ip模式连接的参数是否齐全

		if(SearchIP_ID.isNull()||SearchHttpport_ID.isNull()||SearchMediaPort_ID.isNull()){
			Content.clear();
			arg.clear();
			State_num.clear();
			sDevice_Name.clear();
			QString Num=QString("%1").arg(n);
			sDevice_Name.append(SearchDeviceName_ID);
			Content.append(Num);
			Content.append("AddDeviceDoubleFail");
			EP_ADD_PARAM(arg,"fail",Content);
			EP_ADD_PARAM(arg,"name",sDevice_Name);
			EP_ADD_PARAM(arg,"state",State_num);
			EventProcCall("AddDeviceDoubleFail",arg);
			continue;
		}
		
		//添加设备
		nRet_id=Idevice->AddDevice(Area_ID,SearchDeviceName_ID,SearchIP_ID,SearchMediaPort_ID.toInt(),SearchHttpport_ID.toInt(),SearchSeeId_ID,UserName_ID,PassWord_ID,SearchChannelCount_ID.toInt(),ConnectMethod.toInt(),SearchVendor_ID);
//		nRet_id=Idevice->AddDevice(Area_Id.toInt(),sDeviceName.toString(),sAddress.toString(),port.toInt(),http.toInt(),sEseeId.toString(),sUserName.toString(),sPassWord.toString(),chlCount.toInt(),ConnectMethod.toInt(),sVendor.toString());
		if(-1==nRet_id){
			Content.clear();
			arg.clear();
			State_num.clear();
			sDevice_Name.clear();
			sDevice_Name.append(SearchDeviceName_ID);
			QString Num=QString("%1").arg(n);
			Content.append(Num);
			Content.append("AddDeviceDoubleFail");
			EP_ADD_PARAM(arg,"fail",Content);
			EP_ADD_PARAM(arg,"name",sDevice_Name);
			EP_ADD_PARAM(arg,"state",State_num);
			EventProcCall("AddDeviceDoubleFail",arg);
			continue;
		}

		Content.clear();
		arg.clear();
		sDevice_Name.clear();
		State_num.clear();
		sDevice_Name.append(SearchDeviceName_ID);
		QString Num=QString("%1").arg(n);
		State_num.append(Num);
		QString nSret=QString("%1").arg(nRet_id);
		Content.append(nSret);
		EP_ADD_PARAM(arg,"state",State_num);
		EP_ADD_PARAM(arg,"name",sDevice_Name);
		EP_ADD_PARAM(arg,"deviceid",Content);
		EventProcCall("AddDeviceDoubleSuccess",arg);
		continue;
	}

	if(NULL!=Idevice){Idevice->Release();}
	if(NULL!=Iarea){Iarea->Release();}
}
void settingsActivity::OnAddDevice()
{	
	int nRet_id;
	qDebug("========OnAddDevice========");
	IDeviceManager *Idevice=NULL;
	IAreaManager *Iarea=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void**)&Idevice);
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IAreaManager,(void**)&Iarea);
	if(NULL==Idevice||NULL==Iarea){
		DEF_EVENT_PARAM(arg);
		QString Content="system fail";
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddDeviceFail",arg);
		if(NULL!=Idevice){Idevice->Release();}
		if(NULL!=Iarea){Iarea->Release();}
		return;
	}
	QVariant Area_Id=QueryValue("area_id_ID");
	QVariant sDeviceName=QueryValue("device_name_ID");
	QVariant sAddress=QueryValue("address_ID");
	QVariant port=QueryValue("port_ID");
	QVariant http=QueryValue("http_ID");
	QVariant sEseeId=QueryValue("eseeid_ID");
	QVariant sUserName=QueryValue("username_ID");
	QVariant sPassWord=QueryValue("password_ID");
	QVariant chlCount=QueryValue("channel_count_ID");
	QVariant ConnectMethod=QueryValue("connect_method_ID");
	QVariant sVendor=QueryValue("vendor_ID");

	DEF_EVENT_PARAM(arg);
	QString Content;

	bool nRet_bool=false;
	nRet_bool=Iarea->IsAreaIdExist(Area_Id.toInt());
	if(false==nRet_bool){
		Content="AreaID is not exist";
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddDeviceFail",arg);
		if(NULL!=Idevice){Idevice->Release();}
		if(NULL!=Iarea){Iarea->Release();}
		return;
	}
	if(sDeviceName.isNull()||sUserName.isNull()||chlCount.isNull()||ConnectMethod.isNull()||sVendor.isNull()){
		Content.clear();
		arg.clear();
		Content.append("the params are not complete");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddDeviceFail",arg);
		if(NULL!=Idevice){Idevice->Release();}
		if(NULL!=Iarea){Iarea->Release();}
		return;
	}
	if("0"==ConnectMethod.toString()){
		if(sAddress.isNull()||port.isNull()||http.isNull()){
			Content.clear();
			arg.clear();
			Content.append("the params are not complete");
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("AddDeviceFail",arg);
			if(NULL!=Idevice){Idevice->Release();}
			if(NULL!=Iarea){Iarea->Release();}
			return;
		}
		nRet_id=Idevice->AddDevice(Area_Id.toInt(),sDeviceName.toString(),sAddress.toString(),port.toInt(),http.toInt(),sEseeId.toString(),sUserName.toString(),sPassWord.toString(),chlCount.toInt(),ConnectMethod.toInt(),sVendor.toString());
		if(-1==nRet_id){
			Content.clear();
			arg.clear();
			Content.append("AddDeviceFail");
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("AddDeviceFail",arg);
			if(NULL!=Idevice){Idevice->Release();}
			if(NULL!=Iarea){Iarea->Release();}
			return;
		}
		goto end1;
	}
	else if("1"==ConnectMethod.toString()){
		if(sEseeId.isNull()){
			Content.clear();
			arg.clear();
			Content.append("the params are not complete");
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("AddDeviceFail",arg);
			if(NULL!=Idevice){Idevice->Release();}
			if(NULL!=Iarea){Iarea->Release();}
			return;
		}
		 nRet_id=Idevice->AddDevice(Area_Id.toInt(),sDeviceName.toString(),sAddress.toString(),port.toInt(),http.toInt(),sEseeId.toString(),sUserName.toString(),sPassWord.toString(),chlCount.toInt(),ConnectMethod.toInt(),sVendor.toString());
		if(0!=nRet_id){
			Content.clear();
			arg.clear();
			Content.append("AddDeviceFail");
			EP_ADD_PARAM(arg,"fail",Content);
			EventProcCall("AddDeviceFail",arg);
			if(NULL!=Idevice){Idevice->Release();}
			if(NULL!=Iarea){Iarea->Release();}
			return;
		}
		goto end1;
	}

	Content.clear();
	arg.clear();
	Content.append("ConnectMethod fail");
	EP_ADD_PARAM(arg,"fail",Content);
	EventProcCall("AddDeviceFail",arg);
	if(NULL!=Idevice){Idevice->Release();}
	if(NULL!=Iarea){Iarea->Release();}
	return;
end1:
	Content.clear();
	arg.clear();
	QString nSret=QString("%1").arg(nRet_id);
	Content.append(nSret);
	EP_ADD_PARAM(arg,"deviceid",Content);
	EventProcCall("AddDeviceSuccess",arg);
	if(NULL!=Idevice){Idevice->Release();}
	if(NULL!=Iarea){Iarea->Release();}
	return;
}

void settingsActivity::OnRemoveDevice()
{
	IDeviceManager *Idevice=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void**)&Idevice);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if(NULL==Idevice){
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("RemoveDeviceFail",arg);
		return;
	}


	QVariant Dev_Id=QueryValue("dev_id_ID");
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

	QVariant Dev_Id=QueryValue("dev_id_ID");
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

	QVariant sDeviceName=QueryValue("device_name_ID");
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

	QVariant sUsername=QueryValue("userName_ID");
	QVariant sPassword=QueryValue("password_ID");
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

	QVariant chlCount=QueryValue("channel_count_ID");
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

	QVariant sVentor=QueryValue("vendor_ID");
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

	QVariant sEseeId=QueryValue("eseeid_ID");
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

	QVariant sAddress=QueryValue("address_ID");
	QVariant port=QueryValue("port_ID");
	QVariant http=QueryValue("http_ID");
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

/*Group Module*/
void settingsActivity::OnAddGroup()
{
	int nRet_id;
	IGroupManager *Igroup=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IGroupManager,(void**)&Igroup);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if(NULL==Igroup){
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddGroupFail",arg);
		return;
	}
	QVariant sName_Id=QueryValue("group_name_ID");

	if(sName_Id.isNull()){
		Content.clear();
		arg.clear();
		Content.append("the params are not complete");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddGroupFail",arg);
		Igroup->Release();
		return;
	}

	nRet_id=Igroup->AddGroup(sName_Id.toString());
	if(-1==nRet_id){
		Content.clear();
		arg.clear();
		Content.append("AddDeviceFail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddDeviceFail",arg);
		Igroup->Release();
		return;
	}
	Content.clear();
	arg.clear();
	//Content.append("add group success");
	//EP_ADD_PARAM(arg,"success",Content);
	//EventProcCall("AddGroupSuccess",arg);
	QString nSret=QString("%1").arg(nRet_id);
	Content.append(nSret);
	EP_ADD_PARAM(arg,"groupid",Content);
	EventProcCall("AddGroupSuccess",arg);
	Igroup->Release();
	return;
}

void settingsActivity::OnModifyGroup()
{
	IGroupManager *Igroup=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IGroupManager,(void**)&Igroup);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if (NULL==Igroup)
	{
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("ModifyGroupFail",arg);
		return;
	}
	QVariant Group_id=QueryValue("group_id_ID");
	QVariant Group_name=QueryValue("group_name_ID");
	bool nRet_bool=false;
	nRet_bool=Igroup->IsGroupExists(Group_id.toInt());

	if (false==nRet_bool)
	{
		arg.clear();
		Content.clear();
		Content.append("Group_Id is not exist");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("ModifyGroupFail",arg);
		Igroup->Release();
		return;
	}
	int nret_0=Igroup->ModifyGroupName(Group_id.toInt(),Group_name.toString());
	if (0!=nret_0)
	{
		arg.clear();
		Content.clear();
		Content.append("ModifyGroupFail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("ModifyGroupFail",arg);
		Igroup->Release();
		return;
	}
	arg.clear();
	Content.clear();
	Content.append("ModifyGroupSuccess");
	EP_ADD_PARAM(arg,"success",Content);
	EventProcCall("ModifyGroupSuccess",arg);
	Igroup->Release();

	return;
}

void settingsActivity::OnRemoveGroup()
{	
	IGroupManager *Igroup=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IGroupManager,(void**)&Igroup);
	DEF_EVENT_PARAM(arg);
	QString Content="";
	if(NULL==Igroup){
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("RemoveGroupFail",arg);
		return;
	}

	QVariant Group_Id=QueryValue("group_id_ID");
	bool nRet_bool=false;
	nRet_bool=Igroup->IsGroupExists(Group_Id.toInt());
	if(false==nRet_bool){
		arg.clear();
		Content.clear();
		Content.append("Group_Id is not exist");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("RemoveGroupFail",arg);
		Igroup->Release();
		return;
	}

	int nRet_int = -1;
	nRet_int = Igroup->RemoveGroup(Group_Id.toInt());
	if(0!=nRet_int){
		arg.clear();
		Content.clear();
		Content.append("remove fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("RemoveGroupFail",arg);
		return;
	}

	arg.clear();
	Content.clear();
	Content.append("Remove Success");
	EP_ADD_PARAM(arg,"success",Content);
	EventProcCall("RemoveGroupSuccess",arg);
	Igroup->Release();
	return;
}

/*Area Module*/
void settingsActivity::OnAddArea()
{
	int nRet_id;
	IAreaManager *Iarea=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IAreaManager,(void**)&Iarea);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if(NULL==Iarea){
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddAreaFail",arg);
		return;
	}

	QVariant nPid_Id=QueryValue("pid_ID");
	QVariant sName_Id=QueryValue("area_name_ID");
	 nRet_id=Iarea->AddArea(nPid_Id.toInt(),sName_Id.toString());
	if(-1==nRet_id){
		Content.clear();
		arg.clear();
		Content.append("AddArea Fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddAreaFail",arg);
		Iarea->Release();
		return;
	}
	Content.clear();
	arg.clear();
	//Content.append("AddArea success");
	//EP_ADD_PARAM(arg,"success",Content);
	//EventProcCall("AddAreaSuccess",arg);
	QString nSret=QString("%1").arg(nRet_id);
	Content.append(nSret);
	EP_ADD_PARAM(arg,"areaid",Content);
	EventProcCall("AddAreaSuccess",arg);
	Iarea->Release();
	return;
}

void settingsActivity::OnRemoveArea()
{
	IAreaManager *Iarea=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IAreaManager,(void **)&Iarea);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if(NULL==Iarea){
		arg.clear();
		Content.clear();
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("RemoveAreaFail",arg);
		return;
	}
	QVariant Area_id_ID=QueryValue("area_id_ID");
	QVariant Area_Name_ID=QueryValue("area_name_ID");
	int nRet1=Iarea->RemoveAreaById(Area_id_ID.toInt());
	int nRet2=Iarea->RemoveAreaByName(Area_Name_ID.toString());
	if(0==nRet1||0==nRet2){
		arg.clear();
		Content.clear();
		Content.append("remove success");
		EP_ADD_PARAM(arg,"success",Content);
		EventProcCall("RemoveAreaSuccess",arg);
		Iarea->Release();
		return;
	}
	arg.clear();
	Content.clear();
	Content.append("remove fail");
	EP_ADD_PARAM(arg,"fail",Content);
	EventProcCall("RemoveAreaFail",arg);
	Iarea->Release();
	return;
}

void settingsActivity::OnModifyArea()
{
	IAreaManager *IArea=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IAreaManager,(void **)&IArea);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if(NULL==IArea){
		arg.clear();
		Content.clear();
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("ModifyAreaFail",arg);
		return;
	}

	QVariant Area_id_ID=QueryValue("area_id_ID");
	QVariant Area_Name_ID=QueryValue("area_name_ID");

	int nRet=-1;
	nRet=IArea->SetAreaName(Area_id_ID.toInt(),Area_Name_ID.toString());
	if(0==nRet){
		arg.clear();
		Content.clear();
		Content.append("modify area success");
		EP_ADD_PARAM(arg,"success",Content);
		EventProcCall("ModifyAreaSuccess",arg);
		IArea->Release();
		return;
	}

	arg.clear();
	Content.clear();
	Content.append("modify area fail");
	EP_ADD_PARAM(arg,"fail",Content);
	EventProcCall("ModifyAreaFail",arg);
	IArea->Release();
	return;
}

/*Channel Module*/
void settingsActivity::OnAddChannel()
{
	//useless
	return;
}

void settingsActivity::OnModifyChannel()
{
	IChannelManager *IChannel=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void **)&IChannel);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if(NULL==IChannel){
		arg.clear();
		Content.clear();
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("ModifyChannelFail",arg);
		return;
	}

	QVariant Channel_id_ID=QueryValue("channel_id_ID");
	QVariant Stream_id_ID=QueryValue("stream_id_ID");
	QVariant Channel_Name_ID=QueryValue("channel_name_ID");

	int nRet1=-1;
	int nRet2=-1;

	nRet1=IChannel->ModifyChannelName(Channel_id_ID.toInt(),Channel_Name_ID.toString());
	nRet2=IChannel->ModifyChannelStream(Channel_id_ID.toInt(),Stream_id_ID.toInt());

	if(0!=nRet1&&0!=nRet2){
		arg.clear();
		Content.clear();
		Content.append("modify channel fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("ModifyChannelFail",arg);
		IChannel->Release();
		return;
	}

	arg.clear();
	Content.clear();
	Content.append("modify channel success");
	EP_ADD_PARAM(arg,"success",Content);
	EventProcCall("ModifyChannelSuccess",arg);
	IChannel->Release();
	return;
}

void settingsActivity::OnRemoveChannel()
{
	//useless
	return;
}
void settingsActivity::OnAddChannelInGroupDouble()
{
	int nRet_id=-1;
	IGroupManager *IGroup=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IGroupManager,(void**)&IGroup);
	DEF_EVENT_PARAM(arg);
	QString Content;
	QString ChannelName;
	//申请接口判定
	if(NULL==IGroup){
		arg.clear();
		Content.clear();
		ChannelName.clear();
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EP_ADD_PARAM(arg,"channelname",ChannelName);
		EventProcCall("AddChannelDoubleInGroupFail",arg);
		return;
	}

	QVariant ChlintoGroupFile=QueryValue("addchannelingroupdouble_ID");
	QDomDocument ConfFile;
	ConfFile.setContent(ChlintoGroupFile.toString());

	QDomNode ChlintoGroupInfoNode=ConfFile.elementsByTagName("chlintogroup").at(0);
	QDomNodeList itemList=ChlintoGroupInfoNode.childNodes();

	//若选择的通道号 为零，返回错误提示
	if(0==itemList.count()){
		arg.clear();
		Content.clear();
		ChannelName.clear();
		Content.append("please choose the channel");
		EP_ADD_PARAM(arg,"fail",Content);
		EP_ADD_PARAM(arg,"channelname",ChannelName);
		EventProcCall("AddChannelDoubleInGroupFail",arg);
		if(NULL!=IGroup){IGroup->Release();}
		return;
	}
	//读取组id号
	int Group_ID=ConfFile.firstChild().toElement().attribute("group_id").toInt(); 
	bool nRet_bool=false;
	nRet_bool=IGroup->IsGroupExists(Group_ID);
	if (false==nRet_bool)
	{
		Content.clear();
		Content.append("Group is not exist");
		arg.clear();
		EP_ADD_PARAM(arg,"fail",Content);
		EP_ADD_PARAM(arg,"channelname",ChannelName);
		EventProcCall("AddChannelDoubleInGroupFail",arg);
		if(NULL!=IGroup){IGroup->Release();}
		return;
	}
	//读取xml的节点，添加通道到组
	int n=0;
	int nRet_chl_group_id=-1;
	for(n=0;n<itemList.count();n++){
		QDomNode item;
		item=itemList.at(n);
		QString Channel_id_ID=item.toElement().attribute("channel_id_ID");
		QString R_Chl_Group_Name_ID=item.toElement().attribute("channel_name_ID");

		nRet_chl_group_id=IGroup->AddChannelInGroup(Group_ID,Channel_id_ID.toInt(),R_Chl_Group_Name_ID);
		if (-1==nRet_chl_group_id)
		{
			arg.clear();
			Content.clear();
			ChannelName.clear();

			Content.append("AddChannelDoubleInGroupFail");
			ChannelName.append(R_Chl_Group_Name_ID);
			EP_ADD_PARAM(arg,"fail",Content);
			EP_ADD_PARAM(arg,"channelname",ChannelName);
			EventProcCall("AddChannelDoubleInGroupFail",arg);
		}

		arg.clear();
		Content.clear();
		ChannelName.clear();
		QString nSret=QString("%1").arg(nRet_chl_group_id);
		Content.append(nSret);
		ChannelName.append(R_Chl_Group_Name_ID);

		EP_ADD_PARAM(arg,"chlgroupid",Content);
		EP_ADD_PARAM(arg,"channelname",ChannelName);
		EventProcCall("AddChannelDoubleInGroupSuccess",arg);
	}
	if(NULL!=IGroup){IGroup->Release();}
	return ;
}
void settingsActivity::OnAddChannelInGroup()
{
	int nRet_id=-1;
	IGroupManager *IGroup=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IGroupManager,(void**)&IGroup);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if(NULL==IGroup){
		arg.clear();
		Content.clear();
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddChannelInGroupFail",arg);
		return;
	}

	QVariant Group_id_ID=QueryValue("group_id_ID");
	QVariant Channel_id_ID=QueryValue("channel_id_ID");
	QVariant R_Chl_Group_Name_ID=QueryValue("r_chl_group_name_ID");


	nRet_id=IGroup->AddChannelInGroup(Group_id_ID.toInt(),Channel_id_ID.toInt(),R_Chl_Group_Name_ID.toString());
	if(-1==nRet_id){
		arg.clear();
		Content.clear();
		Content.append("AddChannelInGroup fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("AddChannelInGroupFail",arg);
		IGroup->Release();
		return;
	}
	arg.clear();
	Content.clear();
	//Content.append("AddChannelInGroup success");
	//EP_ADD_PARAM(arg,"success",Content);
	//EventProcCall("AddChannelInGroupSuccess",arg);
	QString nSret=QString("%1").arg(nRet_id);
	Content.append(nSret);
	EP_ADD_PARAM(arg,"chlgroupid",Content);
	EventProcCall("AddChannelInGroupSuccess",arg);
	IGroup->Release();
	return;
}

void settingsActivity::OnRemoveChannelFromGroup()
{
	IGroupManager * IGroup=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IGroupManager,(void **)&IGroup);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if(NULL==IGroup){
		arg.clear();
		Content.clear();
		Content.append("OnRemoveChannelFromGroup fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("RemoveChannelFromFail",arg);
		return;
	}

	QVariant R_Chl_Group_id_ID=QueryValue("r_chl_group_id_ID");
	int nRet=-1;
	nRet=IGroup->RemoveChannelFromGroup(R_Chl_Group_id_ID.toInt());
	if(-1==nRet){
		arg.clear();
		Content.clear();
		Content.append("RemoveChannelFromGroup fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("RemoveChannelFromFail",arg);
		IGroup->Release();
		return;
	}
	arg.clear();
	Content.clear();
	Content.append("RemoveChannelFromGroup success");
	EP_ADD_PARAM(arg,"success",Content);
	EventProcCall("RemoveChannelFromSuccess",arg);
	IGroup->Release();
	return;
}

void settingsActivity::OnModifyGroupChannelName()
{
	IGroupManager *IGroup=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IGroupManager,(void **)&IGroup);
	DEF_EVENT_PARAM(arg);
	QString Content;
	if(NULL==IGroup){
		arg.clear();
		Content.clear();
		Content.append("system fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("ModifyGroupChannelNameFail",arg);
		return;
	}

	QVariant R_Chl_Group_id_ID=QueryValue("r_chl_group_id_ID");
	QVariant R_Chl_Group_Name_ID=QueryValue("r_chl_group_name_ID");

	int nRet=-1;

	nRet=IGroup->ModifyGroupChannelName(R_Chl_Group_id_ID.toInt(),R_Chl_Group_Name_ID.toString());
	if(-1==nRet){
		arg.clear();
		Content.clear();
		Content.append("ModifyGroupChannelName fail");
		EP_ADD_PARAM(arg,"fail",Content);
		EventProcCall("ModifyGroupChannelNameFail",arg);
		IGroup->Release();
		return;
	}

	arg.clear();
	Content.clear();
	Content.append("ModifyGroupChannelName success");
	EP_ADD_PARAM(arg,"success",Content);
	EventProcCall("ModifyGroupChannelName",arg);
	IGroup->Release();
	return;
}