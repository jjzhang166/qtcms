#include "qpreviewwindowsex.h"
#include "guid.h"
#include "IRecorderEx.h"

qpreviewwindowsex::qpreviewwindowsex(QWidget *parent)
	:QWidget(parent),
	QWebPluginFWBase(this),
	m_divMode(NULL),
	m_pAutoPollingTimer(NULL),
	m_nCurrentWnd(0),
	m_bAudioEnabled(false),
	m_bIsEnableDigitalZoom(false),
	m_bIsRestroreView(false),
	m_nRestoreViewNum(-1)
{
	//绑定信号
	for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		m_sPreviewWnd[i].setParentWnd(this);
		m_sPreviewWnd[i].setCurWindId(i);
		m_sPreviewWnd[i].initAfterConstructor();

		connect(&m_sPreviewWnd[i],SIGNAL(sgmousePressEvent(QWidget *,QMouseEvent *)),this,SLOT(subWindowMousePress(QWidget *,QMouseEvent *)));
		connect(&m_sPreviewWnd[i],SIGNAL(sgmouseReleaseEvent(QWidget *,QMouseEvent *)),this,SLOT(subWindowMouseRelease(QWidget *,QMouseEvent *)));
		connect(&m_sPreviewWnd[i],SIGNAL(sgmouseDoubleClick(QWidget *,QMouseEvent *)),this,SLOT(subWindowDblClick(QWidget *,QMouseEvent *)));
		connect(&m_sPreviewWnd[i],SIGNAL(sgconnectStatus(QVariantMap,QWidget *)),this,SLOT(subWindowConnectStatus(QVariantMap,QWidget *)));
		connect(&m_sPreviewWnd[i],SIGNAL(sgconnectRefuse(QVariantMap,QWidget *)),this,SLOT(subWindowConnectRefuse(QVariantMap,QWidget *)));
		connect(&m_sPreviewWnd[i],SIGNAL(sgAuthority(QVariantMap,QWidget *)),this,SLOT(subWindowAuthority(QVariantMap,QWidget *)));
		connect(&m_sPreviewWnd[i],SIGNAL(sgScreenShot(QVariantMap,QWidget *)),this,SLOT(subWindowScreenShot(QVariantMap,QWidget *)));
		connect(&m_sPreviewWnd[i], SIGNAL(sgbackToMainWnd()), this, SLOT(OnBackToMainWnd()));
		connect(&m_sPreviewWnd[i], SIGNAL(sgShutDownDigtalZoom()), this, SLOT(shutDownDigtalZoom()));
		connect(&m_sPreviewWnd[i], SIGNAL(sgVerify(QVariantMap)), this, SLOT(subWindowVerify(QVariantMap)));
		connect(&m_sPreviewWnd[i],SIGNAL(sgEnableDigtalZoom()),this,SLOT(enableDigtalZoom()));
		connect(&m_sPreviewWnd[i],SIGNAL(sgViewNewPosition(QRect,int,int)),this,SLOT(ViewNewPosition(QRect,int ,int)));
		m_pPreviewWndList.insert(m_pPreviewWndList.size(),&m_sPreviewWnd[i]);
	}
	// 读取配置文件，将第一个读到的divmode作为默认分割方式
	QString sAppPath = QCoreApplication::applicationDirPath();
	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConfFile;
	ConfFile.setContent(file);

	QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	int n;
	for (n = 0; n < itemList.count(); n++)
	{
		QDomNode item = itemList.at(n);
		QString sItemName = item.toElement().attribute("name");

		if (sItemName.left(strlen("divmode.")) == QString("divmode."))
		{
			if (NULL != m_divMode)
			{
				m_divMode->Release();
				m_divMode = NULL;
			}
			CLSID divModeClsid = pcomString2GUID(item.toElement().attribute("clsid"));
			pcomCreateInstance(divModeClsid,NULL,IID_IWindowDivMode,(void **)&m_divMode);
			if (NULL != m_divMode)
			{
				m_divMode->setParentWindow(this);
				m_divMode->setSubWindows(m_pPreviewWndList,ARRAY_SIZE(m_sPreviewWnd));
				m_divMode->flush();
			}
			break;
		}
	}

	file->close();
	delete file;

	//fix exceptional record
	IRecorderEx *pRecorderEx = NULL;
	pcomCreateInstance(CLSID_Recorder,NULL,IID_IRecorderEx,(void**)&pRecorderEx);
	if (pRecorderEx)
	{
		pRecorderEx->FixExceptionalData();
		pRecorderEx->Release();
	}
}


qpreviewwindowsex::~qpreviewwindowsex()
{
	for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		m_sPreviewWnd[i].closePreview();
	}
	if (NULL != m_divMode)
	{
		m_divMode->Release();
		m_divMode = NULL;
	}
}

void qpreviewwindowsex::resizeEvent( QResizeEvent *ev )
{
	if (NULL!=m_divMode)
	{
		m_divMode->parentWindowResize(ev);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_divMode is null";
	}
}

void qpreviewwindowsex::nextPage()
{
	if (NULL!=m_divMode)
	{
		m_divMode->nextPage();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_divMode is null";
	}
}

void qpreviewwindowsex::prePage()
{
	if (NULL!=m_divMode)
	{
		m_divMode->prePage();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_divMode is null";
	}
}

int qpreviewwindowsex::getCurrentPage()
{
	if (NULL!=m_divMode)
	{
		return m_divMode->getCurrentPage();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_divMode is null";
		return -1;
	}
}

int qpreviewwindowsex::getPages()
{
	if (NULL!=m_divMode)
	{
		return m_divMode->getPages();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_divMode is null";
		return -1;
	}
}

int qpreviewwindowsex::setDivMode( QString divModeName )
{
	// configuration
	QString sAppPath = QCoreApplication::applicationDirPath();
	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConfFile;
	ConfFile.setContent(file);

	QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	int n;
	bool bFound = false;
	for (n = 0; n < itemList.count(); n++)
	{
		QDomNode item = itemList.at(n);
		QString sItemName = item.toElement().attribute("name");

		if (sItemName.left(strlen("divmode.")) == QString("divmode."))
		{
			QString sDivMode = item.toElement().attribute("mode");
			if (divModeName == sDivMode)
			{
				bFound = true;
				if (NULL != m_divMode)
				{
					m_divMode->Release();
					m_divMode = NULL;
				}
				CLSID divModeClsid = pcomString2GUID(item.toElement().attribute("clsid"));
				pcomCreateInstance(divModeClsid,NULL,IID_IWindowDivMode,(void **)&m_divMode);
				if (NULL != m_divMode)
				{
					m_divMode->setParentWindow(this);
					m_divMode->setSubWindows(m_pPreviewWndList,ARRAY_SIZE(m_sPreviewWnd));
					m_divMode->flush();
					m_nCurrentWnd=0;
				}
				ILocalSetting * pi;
				pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_ILocalSetting,(void **)&pi);
				if (NULL != pi)
				{
					pi->setSplitScreenMode(divModeName);
					pi->Release();
				}
			}
		}
	}

	file->close();
	delete file;

	if (!bFound)
	{
		return IDisplayWindowsManager::E_MODE_NOT_SUPPORT;
	}

	return IDisplayWindowsManager::OK;
}

QString qpreviewwindowsex::getCureentDivMode()
{
	if (NULL!=m_divMode)
	{
		return m_divMode->getModeName();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_divMode is null";
		return "";
	}
}

void qpreviewwindowsex::subWindowDblClick( QWidget*wind,QMouseEvent *ev )
{
	if (NULL!=m_divMode)
	{
		m_divMode->subWindowDblClick(wind,ev);
		QVariantMap evMap;
		EventProcCall("DivModeChange",evMap);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_divMode is null";
		return;
	}
}
void qpreviewwindowsex::subWindowMouseRelease( QWidget* wnd,QMouseEvent * )
{
	if (m_bIsEnableDigitalZoom)
	{
		bool bIsSuitForDigitalZoom=false;
		int nCurrentWndIndex;
		int i=0;
		for (i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
		{
			if (&m_sPreviewWnd[i]==wnd)
			{
				if (m_sPreviewWnd[i].isSuitForDigitalZoom())
				{
					bIsSuitForDigitalZoom=true;
				}else{
					//
				}
				nCurrentWndIndex=i;
				break;;
			}else{
				//do nothing
			}
		}
		if (bIsSuitForDigitalZoom)
		{
			m_sPreviewWnd[nCurrentWndIndex].showDigitalView();
			for (i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
			{
				if (&m_sPreviewWnd[i]==wnd)
				{
					//do nothing
				}else{
					m_sPreviewWnd[i].closeDigitalView();
				}
			}
		}else{
			//do nothing
		}
	}else{
		//do nothing
	}
}

void qpreviewwindowsex::subWindowMousePress( QWidget* wnd,QMouseEvent * ev)
{
	Q_UNUSED(ev);
	int i=0;
	for(i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++){
		if (&m_sPreviewWnd[i]==wnd)
		{
			break;
		}
	}
	DEF_EVENT_PARAM(arg);
	EP_ADD_PARAM(arg,"Wid",i);
	EventProcCall("CurrentWindows",arg);
	m_nCurrentWnd=i;
	for (i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		if (&m_sPreviewWnd[i]==wnd)
		{
			m_sPreviewWnd[i].setCurrentFocus(true);
		}else{
			m_sPreviewWnd[i].setCurrentFocus(false);
		}
	}
}

int qpreviewwindowsex::GetCurrentWnd()
{
	return m_nCurrentWnd;
}

int qpreviewwindowsex::OpenCameraInWnd( unsigned int uiWndIndex ,const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sCameraname ,const QString & sVendor )
{
	if ((int)uiWndIndex+1<0||uiWndIndex>=ARRAY_SIZE(m_sPreviewWnd))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"input uiWndIndex value is error ";
		return 1;
	}else{
		m_sPreviewWnd[uiWndIndex].openPreview(uiChannelId);
		return 0;
	}
}

int qpreviewwindowsex::OpenCameraInWnd( unsigned int uiWndIndex,int chlId )
{
	if ((int)uiWndIndex+1<0||uiWndIndex>=ARRAY_SIZE(m_sPreviewWnd))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"input uiWndIndex value is error ";
		return 1;
	}else{
		m_sPreviewWnd[uiWndIndex].openPreview(chlId);
		return 0;
	}
}

int qpreviewwindowsex::CloseWndCamera( unsigned int uiWndIndex )
{
	if ((int)uiWndIndex+1<0||uiWndIndex>=ARRAY_SIZE(m_sPreviewWnd))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"input uiWndIndex value is error ";
		return 1;
	}else{
		m_sPreviewWnd[uiWndIndex].closePreview();
		return 0;
	}
}

int qpreviewwindowsex::SwithStream( unsigned int uiWndIndex,int chlId )
{
	if ((int)uiWndIndex+1<0||uiWndIndex>=ARRAY_SIZE(m_sPreviewWnd))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"input uiWndIndex value is error ";
		return 1;
	}else{
		m_sPreviewWnd[uiWndIndex].switchStream();
		return 0;
	}
}

int qpreviewwindowsex::SetDevChannelInfo( unsigned int uiWndIndex,int ChannelId )
{
	return m_sPreviewWnd[uiWndIndex].setDevChannelInfo(ChannelId);
	return 0;
}

int qpreviewwindowsex::GetWindowConnectionStatus( unsigned int uiWndIndex )
{
	if ((int)uiWndIndex+1<0||uiWndIndex>=ARRAY_SIZE(m_sPreviewWnd))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<uiWndIndex<<"input uiWndIndex value is error ";
		return 0;
	}else{
		return m_sPreviewWnd[uiWndIndex].getCurrentConnectStatus();
	}
}

QVariantMap qpreviewwindowsex::GetWindowInfo( unsigned int uiWndIndex )
{
	if ((int)uiWndIndex+1<0||uiWndIndex>=ARRAY_SIZE(m_sPreviewWnd))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<uiWndIndex<<"input uiWndIndex value is error ";
		return QVariantMap();
	}else{
		return m_sPreviewWnd[uiWndIndex].getWindowInfo();
	}
}

void qpreviewwindowsex::subWindowConnectStatus( QVariantMap evMap,QWidget *wnd )
{
	int i;
	for (i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		if (&m_sPreviewWnd[i]==wnd)
		{
			break;
		}
	}
	evMap.insert("WPageId",i);
	EventProcCall("CurrentStateChange",evMap);
	return ;
}

int qpreviewwindowsex::StartRecord( int nWndID )
{
	if ((int)nWndID+1<0||nWndID>=ARRAY_SIZE(m_sPreviewWnd))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"input uiWndIndex value is error ";
		return 0;
	}else{
		return m_sPreviewWnd[nWndID].startRecord();
	}
}

int qpreviewwindowsex::StopRecord( int nWndID )
{
	if ((int)nWndID+1<0||nWndID>=ARRAY_SIZE(m_sPreviewWnd))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"input uiWndIndex value is error ";
		return 0;
	}else{
		return m_sPreviewWnd[nWndID].stopRecord();
	}
}

int qpreviewwindowsex::SetDevInfo( const QString&devname,int nChannelNum,int nWndID )
{
	return 0;
}

int qpreviewwindowsex::SetVolume( unsigned int uiPersent )
{
	return m_sPreviewWnd[m_nCurrentWnd].setVolume(uiPersent);
}

int qpreviewwindowsex::AudioEnabled( bool bEnabled )
{
	m_bAudioEnabled=bEnabled;
	for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		m_sPreviewWnd[i].audioEnabled(bEnabled);
	}
	return 0;
}

void qpreviewwindowsex::screenShot(QString sUser,int nType)
{
	return m_sPreviewWnd[m_nCurrentWnd].screenShot( sUser, nType,m_nCurrentWnd);
}

void qpreviewwindowsex::showEvent( QShowEvent *ev )
{
	m_sPreviewWnd[m_nCurrentWnd].audioEnabled(m_bAudioEnabled);
	for(int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++){
		QVariantMap item=m_sPreviewWnd[i].getWindowInfo();
		if (item.value("chlId").toInt()!=-1)
		{
			if (chlIsExist(item.value("chlId").toInt())==false)
			{
				m_sPreviewWnd[i].closePreview();
			}
		}
	}
	QString sLable=getLanguageLable();
	if (m_sLastLanguageLabel!=sLable)
	{
		for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
		{
			m_sPreviewWnd[i].loadLanguage(sLable);
		}
		m_sLastLanguageLabel=sLable;
	}
	//setDataBase flush
	for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		m_sPreviewWnd[i].setDataBaseFlush();
	}
	restoreDigitalView();
}

bool qpreviewwindowsex::chlIsExist( int chlId )
{
	bool flags=false;
	IChannelManager *pChannelManager=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void **)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		QVariantMap channalInfo=pChannelManager->GetChannelInfo(chlId);
		if (channalInfo.value("dev_id").toInt()==-1)
		{
			pChannelManager->Release();
			pChannelManager=NULL;
			return false;
		}else{
			pChannelManager->Release();
			pChannelManager=NULL;
			return true;
		}

	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"checkout chlIsExist fail as pChannelManager is null";
		return false;
	}
}

void qpreviewwindowsex::hideEvent( QHideEvent * )
{
	m_sPreviewWnd[m_nCurrentWnd].audioEnabled(false);
	hideDigitalView();
}

QString qpreviewwindowsex::getLanguageLable()
{
	ILocalSetting *pLocalSetting=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		QString sLable=pLocalSetting->getLanguage();
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return sLable;
	}else{
		return "en_GB";
	}
}

int qpreviewwindowsex::OpenPTZ( int nCmd, int nSpeed )
{
	return m_sPreviewWnd[m_nCurrentWnd].openPTZ(nCmd,nSpeed);
}

int qpreviewwindowsex::ClosePTZ( int nCmd )
{
	return m_sPreviewWnd[m_nCurrentWnd].closePTZ(nCmd);
}

int qpreviewwindowsex::CloseAll()
{
	for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		m_sPreviewWnd[i].closePreview();
	}
	return 0;
}

void qpreviewwindowsex::subWindowConnectRefuse( QVariantMap evMap,QWidget *wnd )
{
	int i;
	for (i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		if (&m_sPreviewWnd[i]==wnd)
		{
			break;
		}
	}
	evMap.insert("WPageId",i);
	EventProcCall("ConnectRefuse",evMap);
	return ;
}

void qpreviewwindowsex::SetFullScreenFlag()
{
	for (int i = 0; i < ARRAY_SIZE(m_sPreviewWnd); i++)
	{
		m_sPreviewWnd[i].SetFullScreen(true);
	}
}

void qpreviewwindowsex::OnBackToMainWnd()
{
	for (int i = 0; i < ARRAY_SIZE(m_sPreviewWnd); ++i)
	{
		m_sPreviewWnd[i].SetFullScreen(false);
	}

	DEF_EVENT_PARAM(item);
	EP_ADD_PARAM(item,"wndStatus","NormalScreen");
	EventProcCall("wndStatus",item);
}
void qpreviewwindowsex::subWindowScreenShot( QVariantMap evMap,QWidget * wnd)
{
	EventProcCall("screenShotInfo",evMap);
}
void qpreviewwindowsex::subWindowAuthority( QVariantMap evMap,QWidget *wnd )
{
	int i;
	for (i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		if (&m_sPreviewWnd[i]==wnd)
		{
			break;
		}
	}
	evMap.insert("WPageId",i);
	EventProcCall("Authority",evMap);
	return ;
}

void qpreviewwindowsex::AllWindowStretch( bool bEnable )
{
	int i;
	for (i = 0 ;i < ARRAY_SIZE(m_sPreviewWnd); i++)
	{
		m_sPreviewWnd[i].enableStretch(bEnable);
	}
	update();
}

void qpreviewwindowsex::StartAutoPolling()
{
	//if timer exist, destroys it
	if (m_pAutoPollingTimer){
		StopAutoPolling();
	}
	//create new timer
	m_pAutoPollingTimer = new QTimer;
	connect(m_pAutoPollingTimer, SIGNAL(timeout()), this, SLOT(slPolling()));
	//get interval time
	int interval = getPollInterval();

	m_pAutoPollingTimer->start(interval*1000);
}

void qpreviewwindowsex::StopAutoPolling()
{
	if (m_pAutoPollingTimer->isActive()){
		m_pAutoPollingTimer->stop();
	}
	delete m_pAutoPollingTimer;
	m_pAutoPollingTimer = NULL;
}

void qpreviewwindowsex::slPolling()
{
	if (!m_divMode){
		qDebug()<<__FUNCTION__<<__LINE__<<"m_divMode is null";
	}

	int interval = getPollInterval();
	if (m_pAutoPollingTimer->interval() != interval*1000){
		m_pAutoPollingTimer->setInterval(interval*1000);
	}
	//get divide number
	int devNum = 1;
	QString devName = m_divMode->getModeName();
	QRegExp rx("div(\\d+)_(\\d+)");
	if (-1 != rx.indexIn(devName)){
		devNum = rx.cap(1).toInt() * rx.cap(2).toInt();
	}
	//find first window which has picture from next page
	int index = 0, findIndex = 0, startWndIndex = 0, pageNum = 0;
	int page = m_divMode->getCurrentPage();
	int curWndIndex = (page + 1)*devNum > MAX_WINDOWS_NUM ? MAX_WINDOWS_NUM - devNum : page*devNum;
	if (curWndIndex >= MAX_WINDOWS_NUM -devNum){
		startWndIndex = 0;
	}else{
		page++;
		startWndIndex = (page + 1)*devNum > MAX_WINDOWS_NUM ? MAX_WINDOWS_NUM - devNum : page*devNum;
	}
	index = startWndIndex;
	findIndex = index%MAX_WINDOWS_NUM;
	while (curWndIndex != findIndex){
		int curStatus = m_sPreviewWnd[findIndex].getCurrentConnectStatus();
		if (curStatus < 2){
			break;
		}
		findIndex = ++index%MAX_WINDOWS_NUM;
	}
	//if find 
	if (findIndex != curWndIndex){
		//count skip pages
		//								boundary12			boundary23
		// |-----------------1----------------|--------2--------|---3---|
		int diff = findIndex - curWndIndex;
		int boundary12 = MAX_WINDOWS_NUM - devNum;
		int boundary23 = (MAX_WINDOWS_NUM/devNum)*devNum;
		pageNum = qAbs(diff)/devNum;
		if (curWndIndex < boundary12 && findIndex < boundary12){
			//|------find----1----cur------|
			if (curWndIndex > findIndex){
				pageNum = MAX_WINDOWS_NUM/devNum + 1 - pageNum;
			}
		}
		//|--------cur----1-------------------|---find---2-------|
		if (curWndIndex < boundary12 && (findIndex >= boundary12 && findIndex < boundary23) && boundary23 - boundary12 != devNum){
			++pageNum;
		}
		//|--------find----1------------------|----cur---2-------|
		if ((curWndIndex >= boundary12 && curWndIndex < boundary23) && findIndex < boundary12 && boundary23 - boundary12 != devNum){
			pageNum = MAX_WINDOWS_NUM/devNum - pageNum;
		}
		if (boundary23 - boundary12 == devNum && curWndIndex > findIndex){
			pageNum = MAX_WINDOWS_NUM - pageNum;
		}
		//start skip
		for (int i = 0; i < pageNum; ++i){
				m_divMode->nextPage();
		}
		EventProcCall("DivModeChange",QVariantMap());
	}
}

void qpreviewwindowsex::subWindowVerify( QVariantMap vmap )
{
	EventProcCall(QString("Validation"), vmap);
}

int qpreviewwindowsex::getPollInterval()
{
	int inteval = 10;
	ILocalSetting *pLocalSetting = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ILocalSetting,(void **)&pLocalSetting);
	if (pLocalSetting){
		inteval = pLocalSetting->getAutoPollingTime();
		pLocalSetting->Release();
	}
	return inteval;
}

void qpreviewwindowsex::shutDownDigtalZoom()
{
	bool bIsDeInitDigtalView=true;
	for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		if (m_sPreviewWnd[i].getDigtalViewIsClose()==false)
		{
			bIsDeInitDigtalView=false;
			break;
		}
	}
	if (bIsDeInitDigtalView&&m_bIsRestroreView==false)
	{
		for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
		{
			m_bIsEnableDigitalZoom=false;
			m_sPreviewWnd[i].deInitDigtalView();
		}
	}
}

void qpreviewwindowsex::enableDigtalZoom()
{
	m_bIsEnableDigitalZoom=true;
	for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		m_sPreviewWnd[i].disableOriginalWndDrawRect();
	}
}

void qpreviewwindowsex::hideDigitalView()
{
	for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		if (m_sPreviewWnd[i].getDigtalViewIsClose()==false)
		{
			m_nRestoreViewNum=i;
			m_bIsRestroreView=true;
			break;
		}
	}
	for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		m_sPreviewWnd[i].closeDigitalView();
	}
}

void qpreviewwindowsex::restoreDigitalView()
{
	if (m_nRestoreViewNum!=-1)
	{
		m_sPreviewWnd[m_nRestoreViewNum].showDigitalView();
		m_nRestoreViewNum=-1;
		m_bIsRestroreView=false;
	}else{
		//do nothing
	}
}

void qpreviewwindowsex::ViewNewPosition( QRect tRect,int nWidth,int nHeight )
{
	for (int i=0;i<ARRAY_SIZE(m_sPreviewWnd);i++)
	{
		m_sPreviewWnd[i].ViewNewPosition(tRect,nWidth,nHeight);
	}
}





