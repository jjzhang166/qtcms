#include "qsubviewEx.h"
#include <QTime>
#include "vld.h"

bool qsubviewEx::ms_bIsFullScreen = false;
qsubviewEx::qsubviewEx(QWidget *parent):
QWidget(parent),
	m_tCurConnectStatus(STATUS_DISCONNECTED),
	m_tHistoryConnectStatus(STATUS_DISCONNECTED),
	m_pManageWidget(NULL),
	m_bIsFocus(false),
	m_bIsRecording(false),
	m_pClosePreviewAction(NULL),
	m_pSwitchStreamAciton(NULL),
	m_pRecorderAction(NULL),
	m_pBackMainViewAction(NULL),
	m_pTtanslator(NULL),
	m_nConnectingCount(0)
{
	//注册回调函数
	m_sSubviewRun.registerEvent(QString("CurrentStatus"),cbStateChangeEx,this);
	m_sSubviewRun.registerEvent(QString("RecordState"),cbRecordStateEx,this);
	m_sSubviewRun.registerEvent(QString("ConnectRefuse"),cbConnectRefuseEx,this);
	m_sSubviewRun.registerEvent(QString("Authority"),cbAuthorityEx,this);
	//生成渲染的窗口
	m_pManageWidget=new ManageWidget(this);
	//右键菜单
	m_pClosePreviewAction=m_mRightMenu.addAction(tr("Close Preview"));
	m_pSwitchStreamAciton=m_mRightMenu.addAction(tr("Switch Stream"));
	m_pRecorderAction=m_mRightMenu.addAction(tr("Start Record"));
	m_pBackMainViewAction = m_mRightMenu.addAction(tr("Back to Main Window"));
	m_pBackMainViewAction->setEnabled(false);
	//多语言
	m_pTtanslator=new QTranslator();
	QApplication::installTranslator(m_pTtanslator);
	//绑定信号
	connect(this,SIGNAL(sgrecordState(bool)),m_pManageWidget,SLOT(RecordState(bool)));
	connect(this,SIGNAL(sgmouseMenu()),this,SLOT(slmouseMenu()));
	connect(this,SIGNAL(sgbackToMainThread(QVariantMap)),this,SLOT(slbackToMainThread(QVariantMap)));
	connect(m_pClosePreviewAction,SIGNAL(triggered(bool)),this,SLOT(slclosePreview()));
	connect(m_pSwitchStreamAciton,SIGNAL(triggered(bool)),this,SLOT(slswitchStreamEx()));
	connect(m_pRecorderAction,SIGNAL(triggered(bool)),this,SLOT(slMenRecorder()));
	connect(m_pBackMainViewAction,SIGNAL(triggered(bool)),this,SLOT(slbackToManiWnd()));

	m_tDeviceInfo.m_uiChannelIdInDataBase=-1;
}


qsubviewEx::~qsubviewEx(void)
{
	m_sSubviewRun.stopPreview();
	//need to wait m_sSubviewRun terminate
	tagDeviceInfo tDeviceInfo=m_sSubviewRun.deviceInfo();
	int ncount =0;
	while(m_tCurConnectStatus!=STATUS_DISCONNECTED&&ncount<800){
		QTime dieTime=QTime::currentTime().addMSecs(1);
		while(QTime::currentTime()<dieTime){
			QCoreApplication::processEvents(QEventLoop::AllEvents,10);
		}
		ncount++;
		if (ncount>500&&ncount%100==0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.m_sDeviceName<<ncount/100<<"there may be some error as thread can not terminal";
		}
	}
	if (NULL!=m_pManageWidget)
	{
		delete m_pManageWidget;
		m_pManageWidget=NULL;
	}
	if (NULL!=m_pTtanslator)
	{
		delete m_pTtanslator;
		m_pTtanslator=NULL;
	}
}

void qsubviewEx::paintEvent( QPaintEvent *ev )
{
	switch(m_tCurConnectStatus){
	case STATUS_CONNECTED:{
		paintEventConnected(ev);
						  }
						  break;
	case STATUS_CONNECTING:{
		paintEventConnecting(ev);
						   }
						   break;
	case STATUS_DISCONNECTED:{
		paintEventDisconnected(ev);
							 }
							 break;
	case STATUS_DISCONNECTING:{
		paintEventDisconnecting(ev);
							  }
							  break;
	}
}

void qsubviewEx::mouseDoubleClickEvent( QMouseEvent *ev )
{
	emit sgmouseDoubleClick(this,ev);
}

void qsubviewEx::resizeEvent( QResizeEvent * )
{
	m_pManageWidget->resize(this->size());
	m_sSubviewRun.ipcSwitchStream();
}

int qsubviewEx::cbCStateChange( QVariantMap evMap )
{
	evMap.insert("eventName","cbCStateChange");
	emit sgbackToMainThread(evMap);
	return 0;
}

int qsubviewEx::cbCRecordState( QVariantMap evMap )
{
	evMap.insert("eventName","cbCRecordState");
	emit sgbackToMainThread(evMap);
	return 0;
}

void qsubviewEx::slbackToMainThread( QVariantMap evMap )
{
	if (evMap.contains("eventName")&&evMap.value("eventName")=="cbCRecordState")
	{
		//录像状态转变
		if (m_pManageWidget->GetRecordItem()!=NULL)
		{
			if (evMap.value("RecordState").toBool()==true)
			{
				emit sgrecordState(true);
				m_bIsRecording=true;
			}else{
				emit sgrecordState(false);
				m_bIsRecording=false;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"there is no record item";
		}
	}
	//
	if (evMap.contains("eventName")&&evMap.value("eventName")=="cbCStateChange")
	{
		if(evMap.contains("CurrentStatus")){
			m_tCurConnectStatus=(tagConnectStatus)evMap.value("CurrentStatus").toInt();
			if (m_tCurConnectStatus==STATUS_CONNECTED)
			{
				//连接状态
				//关闭正在连接中的屏幕显示
				m_tConnectingTimer.stop();
				update();
				disconnect(&m_tConnectingTimer,SIGNAL(timeout()),this,SLOT(update()));
				qDebug()<<__FUNCTION__<<__LINE__<<getDeviceInfo().m_sDeviceName<<getDeviceInfo().m_uiChannelId<<"::connected";

			}else if (m_tCurConnectStatus==STATUS_CONNECTING)
			{
				//正在连接中 
				//启动正在连接中的屏幕显示
				m_tConnectingTimer.start(1000);
				connect(&m_tConnectingTimer,SIGNAL(timeout()),this,SLOT(update()));
			}else if (m_tCurConnectStatus==STATUS_DISCONNECTING)
			{
				//正在断开
				//关闭正在连接中的屏幕显示
				m_tConnectingTimer.stop();
				disconnect(&m_tConnectingTimer,SIGNAL(timeout()),this,SLOT(update()));
			}else if (m_tCurConnectStatus==STATUS_DISCONNECTED)
			{
				//已经断开
				//关闭正在连接中的屏幕显示
				m_tConnectingTimer.stop();
				QTimer::singleShot(500, this, SLOT(update()));
				disconnect(&m_tConnectingTimer,SIGNAL(timeout()),this,SLOT(update()));
				qDebug()<<__FUNCTION__<<__LINE__<<getDeviceInfo().m_sDeviceName<<getDeviceInfo().m_uiChannelId<<"::disconnected";
			}
			//抛出事件
			if (m_tHistoryConnectStatus!=m_tCurConnectStatus)
			{
				QVariantMap evMapToUi;
				evMapToUi.insert("CurrentState",m_tCurConnectStatus);
				evMapToUi.insert("ChannelId",m_tDeviceInfo.m_uiChannelIdInDataBase);
				emit sgconnectStatus(evMapToUi,this);
				m_tHistoryConnectStatus=m_tCurConnectStatus;
			}
		}
	}
	if (evMap.contains("eventName")&&evMap.value("eventName")=="cbCConnectRefuse")
	{
		QVariantMap evMapToUi;
		evMapToUi.insert("ConnectRefuse",true);
		evMapToUi.insert("ChannelId",m_tDeviceInfo.m_uiChannelIdInDataBase);
		qDebug()<<__FUNCTION__<<__LINE__<<getDeviceInfo().m_sDeviceName<<getDeviceInfo().m_uiChannelId<<"stop preview because as the device resource had been full";
		emit sgconnectRefuse(evMapToUi,this);
	}else{
		//do noting
	}
	if (evMap.value("eventName")=="Authority")
	{
		QVariantMap evMapToUi;
		evMapToUi.insert("Authority",false);
		evMapToUi.insert("ChannelId",m_tDeviceInfo.m_uiChannelIdInDataBase);
		qDebug()<<__FUNCTION__<<__LINE__<<getDeviceInfo().m_sDeviceName<<getDeviceInfo().m_uiChannelId<<"stop preview because as the device resource had been full";
		emit sgAuthority(evMapToUi,this);
	}else{
		//do nothing
	}
}

void qsubviewEx::paintEventConnected( QPaintEvent *ev )
{
	Q_UNUSED(ev);
	QPainter p(this);
	QString image;
	QColor LineColor(45,49,54);
	QColor LineCurColor;
	QColor FontColor;
	int FontSize;
	QString FontFamily;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);

	image = IniFile.value("background/background-image", QVariant("")).toString();
	/*LineColor.setNamedColor(IniFile.value("background/background-color", QVariant("")).toString());*/
	LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
	FontColor.setNamedColor(IniFile.value("font/font-color", QVariant("")).toString());
	FontSize = IniFile.value("font/font-size", QVariant("")).toString().toInt();
	FontFamily = IniFile.value("font/font-family", QVariant("")).toString();

	QRect rcClient = contentsRect();
	this->geometry().center();
	QPixmap pix;

	QString PixPaht;
	PixPaht= sAppPath + image;
	pix.load(PixPaht);
	pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
	//背景
	p.drawPixmap(rcClient,pix);
	//边框
	QPen pen = QPen(LineColor);
	pen.setWidth(2);
	p.setPen(pen);
	p.drawRect(rcClient);
	//焦点

	if (m_bIsFocus)
	{
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		rcClient.getCoords(&x, &y, &width, &height);
		pen.setWidth(5);
		pen.setColor(LineCurColor);
		p.setPen(pen);
		p.drawRect(QRectF(x,y,width, height));
	}
	else
	{
		p.drawRect(rcClient);
	}
}

void qsubviewEx::paintEventDisconnected( QPaintEvent *ev )
{
	Q_UNUSED(ev);
	QPainter p(this);
	QString image;
	QColor LineColor(45,49,54);
	QColor LineCurColor;
	QColor FontColor(32,151,219);
	int FontSize;
	QString FontFamily;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);

	image = IniFile.value("background/background-image", QVariant("")).toString();
	/*LineColor.setNamedColor(IniFile.value("background/background-color", QVariant("")).toString());*/
	LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
	/*FontColor.setNamedColor(IniFile.value("font/font-color", QVariant("")).toString());*/
	FontSize = IniFile.value("font/font-size", QVariant("")).toString().toInt();
	FontFamily = IniFile.value("font/font-family", QVariant("")).toString();

	QRect rcClient = contentsRect();
	this->geometry().center();
	QPixmap pix;
	QString PixPaht = sAppPath + image;
	pix.load(PixPaht);

	pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
	//背景
	p.drawPixmap(rcClient,pix);
	//边框
	QPen pen = QPen(LineColor);
	pen.setWidth(2);
	p.setPen(pen);
	p.drawRect(rcClient);
	//焦点

	if (m_bIsFocus)
	{
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		rcClient.getCoords(&x, &y, &width, &height);
		pen.setWidth(2);
		pen.setColor(LineCurColor);
		p.setPen(pen);
		p.drawRect(QRectF(x + 2,y + 2,width - 2, height - 2));
	}
	else
	{
		p.drawRect(rcClient);
	}
	//
	int awidth=0;
	int bheight=0;
	int ax=0;
	int ay=0;
	rcClient.getCoords(&ax, &ay, &awidth, &bheight);
	int aFontSize=10;
	int aw=400;
	aFontSize=awidth*FontSize/(aw);
	QFont font(FontFamily, aFontSize, QFont::Bold|QFont::System);
	p.setFont(font);
	pen.setColor(FontColor);
	p.setPen(pen);
	p.drawText(rcClient, Qt::AlignCenter, "No Video");
}

void qsubviewEx::paintEventConnecting( QPaintEvent *ev )
{
	Q_UNUSED(ev);
	QPainter p(this);
	QString image;
	QColor LineColor(45,49,54);
	QColor LineCurColor;
	QColor FontColor(32,151,219);
	int FontSize;
	QString FontFamily;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);

	image = IniFile.value("background/background-image", QVariant("")).toString();
	/*LineColor.setNamedColor(IniFile.value("background/background-color", QVariant("")).toString());*/
	LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
	/*FontColor.setNamedColor(IniFile.value("font/font-color", QVariant("")).toString());*/
	FontSize = IniFile.value("font/font-size", QVariant("")).toString().toInt();
	FontFamily = IniFile.value("font/font-family", QVariant("")).toString();

	QRect rcClient = contentsRect();
	this->geometry().center();
	QPixmap pix;
	QString PixPaht = sAppPath + image;
	pix.load(PixPaht);

	pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
	//背景
	p.drawPixmap(rcClient,pix);
	//边框
	QPen pen = QPen(LineColor);
	pen.setWidth(2);
	p.setPen(pen);
	p.drawRect(rcClient);
	//焦点

	if (m_bIsFocus)
	{
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		rcClient.getCoords(&x, &y, &width, &height);
		pen.setWidth(5);
		pen.setColor(LineCurColor);
		p.setPen(pen);
		p.drawRect(QRectF(x + 2,y + 2,width - 2, height - 2));
	}
	else
	{
		p.drawRect(rcClient);
	}
	//
	int awidth=0;
	int bheight=0;
	int ax=0;
	int ay=0;

	rcClient.getCoords(&ax, &ay, &awidth, &bheight);
	int aFontSize=10;
	int aw=400;
	//	int ah=300;
	aFontSize=awidth*FontSize/(aw);
	QFont font(FontFamily, aFontSize, QFont::Bold);

	p.setFont(font);

	pen.setColor(FontColor);

	p.setPen(pen);

	QString m_text;
	QPoint dev_position=this->geometry().topLeft();
	if (m_nConnectingCount==4)
	{
		m_nConnectingCount=0;
	}
	for (int i=0;i<m_nConnectingCount;i++)
	{
		m_text+="..";
	}
	m_nConnectingCount++;
	p.drawText(rcClient, Qt::AlignCenter, m_text);
}

void qsubviewEx::paintEventDisconnecting( QPaintEvent *ev )
{
	Q_UNUSED(ev);
	QPainter p(this);
	QString image;
	/*QColor LineColor;*/
	QColor LineColor(45,49,54);
	QColor LineCurColor;
	/*QColor FontColor;*/
	QColor FontColor(32,151,219);
	int FontSize;
	QString FontFamily;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);

	image = IniFile.value("background/background-image", QVariant("")).toString();
	/*LineColor.setNamedColor(IniFile.value("background/background-color", QVariant("")).toString());*/
	LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
	/*FontColor.setNamedColor(IniFile.value("font/font-color", QVariant("")).toString());*/
	FontSize = IniFile.value("font/font-size", QVariant("")).toString().toInt();
	FontFamily = IniFile.value("font/font-family", QVariant("")).toString();

	QRect rcClient = contentsRect();
	this->geometry().center();
	QPixmap pix;
	QString PixPaht = sAppPath + image;
	pix.load(PixPaht);

	pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
	//背景
	p.drawPixmap(rcClient,pix);
	//边框
	QPen pen = QPen(LineColor);
	pen.setWidth(2);
	p.setPen(pen);
	p.drawRect(rcClient);
	//焦点

	if (m_bIsFocus)
	{
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		rcClient.getCoords(&x, &y, &width, &height);
		pen.setWidth(2);
		pen.setColor(LineCurColor);
		p.setPen(pen);
		p.drawRect(QRectF(x + 2,y + 2,width - 2, height - 2));
	}
	else
	{
		p.drawRect(rcClient);
	}
	//
	int awidth=0;
	int bheight=0;
	int ax=0;
	int ay=0;
	rcClient.getCoords(&ax, &ay, &awidth, &bheight);
	int aFontSize=10;
	int aw=400;
	aFontSize=awidth*FontSize/(aw);
	QFont font(FontFamily, aFontSize, QFont::Bold|QFont::System);
	p.setFont(font);
	pen.setColor(FontColor);
	p.setPen(pen);
	p.drawText(rcClient, Qt::AlignCenter, "No Video");
}

void qsubviewEx::mousePressEvent( QMouseEvent * ev)
{
	setFocus(Qt::MouseFocusReason);
	if (ev->button()==Qt::RightButton)
	{
		emit sgmouseMenu();
	}
	emit sgmousePressEvent(this,ev);
	m_sSubviewRun.setFoucs(true);
	m_bIsFocus=true;
}

int qsubviewEx::openPreview( int chlId )
{
	m_sSubviewRun.openPreview(m_tDeviceInfo.m_uiChannelIdInDataBase,m_pManageWidget->GetWidgetForVideo(),this->parentWidget());
	return 0;
}

int qsubviewEx::closePreview()
{
	m_sSubviewRun.stopPreview();
	return 0;
}

int qsubviewEx::setDevChannelInfo( int chlId )
{
	//do nothing ,this interface may be unnecessary
	m_tDeviceInfo.m_uiChannelIdInDataBase=chlId;
	return 0;
}

void qsubviewEx::slmouseMenu()
{
	m_tDeviceInfo=m_sSubviewRun.deviceInfo();
	if (m_tDeviceInfo.m_uiStreamId==0)
	{
		m_pSwitchStreamAciton->setText(tr("Switch to SubStream"));
	}else{
		m_pSwitchStreamAciton->setText(tr("Switch to MainStream"));
	}
	m_pClosePreviewAction->setText(tr("Close Preview"));
	if (m_tCurConnectStatus==STATUS_DISCONNECTED)
	{
		m_pClosePreviewAction->setDisabled(true);
		m_pSwitchStreamAciton->setDisabled(true);
	}else{
		m_pSwitchStreamAciton->setEnabled(true);
		m_pClosePreviewAction->setEnabled(true);
	}
	if (m_tDeviceInfo.m_sVendor=="IPC")
	{
		m_pSwitchStreamAciton->setDisabled(true);
	}else{
		//do nothing 
	}
	if (m_tCurConnectStatus==STATUS_CONNECTED)
	{
		m_pRecorderAction->setEnabled(true);
	}else{
		m_pRecorderAction->setDisabled(true);
	}
	if (2&m_sSubviewRun.getRecordStatus())
	{
		m_pRecorderAction->setText(tr("Stop Record"));
	}else{
		m_pRecorderAction->setText(tr("Start Record"));
	}
	m_mRightMenu.exec(QCursor::pos());
}

int qsubviewEx::getCurrentConnectStatus()
{
	return m_tCurConnectStatus;
}

QVariantMap qsubviewEx::getWindowInfo()
{
	QVariantMap vWindowInfo;
	vWindowInfo.insert("focus",m_bIsFocus);
	vWindowInfo.insert("currentState",m_tCurConnectStatus);
	vWindowInfo.insert("chlId",m_tDeviceInfo.m_uiChannelIdInDataBase);
	if (m_sSubviewRun.isRunning())
	{
		vWindowInfo.insert("usable",false);
	}else{
		vWindowInfo.insert("usable",true);
	}
	return vWindowInfo;
}

int qsubviewEx::switchStream()
{
	m_sSubviewRun.switchStream();
	return 0;
}


void qsubviewEx::slswitchStreamEx()
{
	m_sSubviewRun.switchStreamEx();
}

int qsubviewEx::startRecord()
{
	return m_sSubviewRun.startManualRecord();
}

int qsubviewEx::stopRecord()
{
	return m_sSubviewRun.stopManualRecord();
}

int qsubviewEx::setPlayWnd( int nwnd )
{
	return 0;
}

int qsubviewEx::setVolume( unsigned int uiPersent )
{
	m_sSubviewRun.setVolume(uiPersent);
	return 0;
}

int qsubviewEx::audioEnabled( bool bEnable )
{
	m_sSubviewRun.audioEnabled(bEnable);
	return 0;
}

QVariantMap qsubviewEx::screenShot()
{
	return m_sSubviewRun.screenShot();
}

int qsubviewEx::openPTZ( int ncmd,int nspeed )
{
	m_sSubviewRun.openPTZ(ncmd,nspeed);
	return 0;
}

int qsubviewEx::closePTZ( int ncmd )
{
	m_sSubviewRun.closePTZ(ncmd);
	return 0;
}

void qsubviewEx::changeEvent( QEvent *ev )
{
	if (ev->type()==QEvent::LanguageChange)
	{
		//do something
		translateLanguage();
	}
}

void qsubviewEx::slclosePreview()
{
	m_sSubviewRun.stopPreview();
}

tagDeviceInfo qsubviewEx::getDeviceInfo()
{
	return m_sSubviewRun.deviceInfo();
}

void qsubviewEx::loadLanguage( QString tags )
{
	if (NULL!=m_pTtanslator)
	{
		QString sAppPath = QCoreApplication::applicationDirPath();
		QString path = sAppPath + "/LocalSetting";
		QString tagName=getLanguageInfo(tags);
		m_pTtanslator->load(tagName,path);
	}
}

QString qsubviewEx::getLanguageInfo( QString tags )
{
	QString sAppPath=QCoreApplication::applicationDirPath();
	sAppPath+="/LocalSetting";
	QDomDocument ConFile;
	QFile *file=new QFile(sAppPath+"/language.xml");
	file->open(QIODevice::ReadOnly);
	ConFile.setContent(file);
	QDomNode clsidNode = ConFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	QString sFileName="en_GB";
	for (int n = 0; n < itemList.count(); n++)
	{
		QDomNode item = itemList.at(n);
		QString slanguage = item.toElement().attribute("name");
		if(slanguage == tags){
			sFileName =item.toElement().attribute("file");
			break;
		}
	}
	file->close();
	delete file;
	return sFileName;
}

void qsubviewEx::setCurrentFocus( bool flags )
{
	if (m_bIsFocus==true&&flags==false)
	{
		update();
	}
	m_bIsFocus=flags;
	m_sSubviewRun.setFoucs(flags);
}

void qsubviewEx::translateLanguage()
{
	if (NULL!=m_pSwitchStreamAciton)
	{
		m_pSwitchStreamAciton->setText(tr("Switch Stream"));
	}
	if (NULL!=m_pClosePreviewAction)
	{
		m_pClosePreviewAction->setText(tr("Close Preview"));
	}
	if (NULL != m_pBackMainViewAction)
	{
		m_pBackMainViewAction->setText(tr("Back to Main Window"));
	}
}

void qsubviewEx::setDataBaseFlush()
{
	m_sSubviewRun.setDatabaseFlush(true);
}

void qsubviewEx::slMenRecorder()
{
	if (2&m_sSubviewRun.getRecordStatus())
	{
		stopRecord();
	}else{
		startRecord();
	}
}


void qsubviewEx::setCurWindId( int nWindId )
{
	m_sSubviewRun.setWindId(nWindId);
}

int qsubviewEx::cbCConnectRefuse( QVariantMap evMap )
{
	evMap.insert("eventName","cbCConnectRefuse");
	emit sgbackToMainThread(evMap);
	return 0;
}

void qsubviewEx::slbackToManiWnd()
{
	ms_bIsFullScreen = false;
	emit sgbackToMainWnd();
}

int qsubviewEx::SetFullScreen( bool bFullScreen )
{
	ms_bIsFullScreen = bFullScreen;
	m_pBackMainViewAction->setEnabled(bFullScreen);
	return 0;
}

int qsubviewEx::getRecordStatus()
{
	return m_sSubviewRun.getRecordStatus();
}

int qsubviewEx::cbCAuthority( QVariantMap evMap )
{
	if (evMap.value("Authority").toBool()==false)
	{
		evMap.insert("eventName","Authority");
		emit sgbackToMainThread(evMap);
	}else{

	}

	return 0;
}


int cbStateChangeEx(QString evName,QVariantMap evMap,void*pUser)
{
	if (evName=="CurrentStatus")
	{
		((qsubviewEx*)pUser)->cbCStateChange(evMap);
		return 0;
	}
	return 1;
}
int cbRecordStateEx(QString evName,QVariantMap evMap,void*pUser)
{
	if (evName=="RecordState")
	{
		((qsubviewEx*)pUser)->cbCRecordState(evMap);
		return 0;
	}
	else
		return 1;
}

int cbConnectRefuseEx( QString evName,QVariantMap evMap,void*pUser )
{
	if (evName=="ConnectRefuse")
	{
		((qsubviewEx*)pUser)->cbCConnectRefuse(evMap);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cbConnectRefuseEx fail as the eventName is not collect";
		return 1;
	}
}

int cbAuthorityEx( QString evName,QVariantMap evMap,void*pUser )
{
	if (evName=="Authority")
	{
		((qsubviewEx*)pUser)->cbCAuthority(evMap);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cbConnectRefuseEx fail as the eventName is not collect";
		return 1;
	}
}
