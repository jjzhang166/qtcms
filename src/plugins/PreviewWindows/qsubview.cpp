#include "qsubview.h"
#include <guid.h>
#include <QtGui/QPainter>
#include <QPoint>

#include <QtCore>
#include <QSettings>
#include <QMouseEvent>
#include <QtXml/QtXml>

QSubView::QSubView(QWidget *parent)
	: QWidget(parent),m_IVideoDecoder(NULL),
	m_IVideoRender(NULL),
	m_IDeviceClientDecideByVendor(NULL),
	m_pRecorder(NULL),
	m_pRecordTime(NULL),
	iInitHeight(0),
	iInitWidth(0),
	bRendering(false),
	m_bIsRecording(false),
	m_bStateAutoConnect(false),
	m_bIsAutoConnecting(false),
	m_bIsAutoRecording(false),
	ui(new Ui::titleview),
	m_QActionCloseView(NULL),
	m_CurrentState(QSubView::QSubViewConnectStatus::STATUS_DISCONNECTED),
	m_HistoryState(QSubView::QSubViewConnectStatus::STATUS_DISCONNECTED),
	CountConnecting(0),
	CountDisConnecting(0),
	m_DisConnectedTimeId(0),
	m_DisConnectingTimeId(0),
	m_RenderTimeId(0),
	m_RecordFlushTime(0),
	m_AutoConnectTimeId(0)
{
	this->lower();
	this->setAttribute(Qt::WA_PaintOutsidePaintEvent);
	//申请解码器接口
//	pcomCreateInstance(CLSID_HiH264Decoder,NULL,IID_IVideoDecoder,(void**)&m_IVideoDecoder);
 	pcomCreateInstance(CLSID_h264Decoder,NULL,IID_IVideoDecoder,(void**)&m_IVideoDecoder);
	//申请渲染器接口
	pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_IVideoRender);
	//申请DeviceClient接口,修改成动态生成，此处去掉
	/*pcomCreateInstance(CLSID_DeviceClient,NULL,IID_IDeviceClient,(void**)&m_IDeviceClient);*/
	//申请IRecorder接口
	pcomCreateInstance(CLSID_Recorder,NULL,IID_IRecorder,(void **)&m_pRecorder);

	//申请ISetRecordTime接口
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ISetRecordTime,(void **)&m_pRecordTime);
	connect(&m_checkTime, SIGNAL(timeout()), this, SLOT(OnCheckTime()));

	connect(this,SIGNAL(DisConnecting()),this,SLOT(OnDisConnecting()),Qt::QueuedConnection);
	connect(this,SIGNAL(Connectting()),this,SLOT(OnConnectting()),Qt::QueuedConnection);//Ui显示正在连接中
	connect(this,SIGNAL(DisConnected()),this,SLOT(OnDisConnected()),Qt::QueuedConnection);
	connect(this,SIGNAL(RenderHistoryPix()),this,SLOT(OnRenderHistoryPix()),Qt::QueuedConnection);
	connect(this,SIGNAL(AutoConnectSignals()),this,SLOT(In_OpenAutoConnect()),Qt::QueuedConnection);
	connect(this,SIGNAL(CreateAutoConnectTimeSignals()),this,SLOT(OnCreateAutoConnectTime()),Qt::QueuedConnection);
	//修改成动态生成，此处去掉
//	m_QSubViewObject.SetDeviceClient(m_IDeviceClient);

	m_QActionCloseView=m_RMousePressMenu.addAction("close preview");
	connect(this,SIGNAL(RMousePressMenu()),this,SLOT(OnRMousePressMenu()));
	connect(m_QActionCloseView,SIGNAL(triggered(bool)),this,SLOT(OnCloseFromMouseEv()));
	//初始历史render的值
	m_HistoryRenderInfo.pData=NULL;
}

QSubView::~QSubView()
{
	CloseWndCamera();
	
	if (NULL!=m_IDeviceClientDecideByVendor)
	{
		m_IDeviceClientDecideByVendor->Release();
		m_IDeviceClientDecideByVendor=NULL;
	}

	if (NULL!=m_IVideoDecoder)
	{
		m_IVideoDecoder->Release();
	}
	if (NULL!=m_IVideoRender)
	{
		m_IVideoRender->Release();
	}
	delete ui;

	m_checkTime.stop();
	if (NULL != m_pRecordTime)
	{
		m_pRecordTime->Release();
	}

	if (NULL != m_pRecorder)
	{
		m_pRecorder->Stop();
		m_pRecorder->Release();
	}
}


void QSubView::paintEvent( QPaintEvent * e)
{
	QPainter p(this);
	if (m_CurrentState==QSubViewConnectStatus::STATUS_CONNECTED)
	{
		return;
	}
	QString image;
	QColor LineColor;
	QColor LineCurColor;
	QColor FontColor;
	int FontSize;
	QString FontFamily;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);

	image = IniFile.value("background/background-image", NULL).toString();
	LineColor.setNamedColor(IniFile.value("background/background-color", NULL).toString());
	LineCurColor.setNamedColor(IniFile.value("background/background-color-current", NULL).toString());
	FontColor.setNamedColor(IniFile.value("font/font-color", NULL).toString());
	FontSize = IniFile.value("font/font-size", NULL).toString().toInt();
	FontFamily = IniFile.value("font/font-family", NULL).toString();

 	QRect rcClient = contentsRect();
 
 	QPixmap pix;
	QString PixPaht = sAppPath + image;
 	bool ret = pix.load(PixPaht);
 
  	pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);

 	p.drawPixmap(rcClient,pix);

	QPen pen = QPen(LineColor);
	pen.setWidth(2);
 	p.setPen(pen);
	p.drawRect(rcClient);
	if (this->hasFocus())
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
	int awidth=0;
	int bheight=0;
	int ax=0;
	int ay=0;

	rcClient.getCoords(&ax, &ay, &awidth, &bheight);
	int aFontSize=10;
	int aw=400;
	int ah=300;
	aFontSize=awidth*FontSize/(aw);
	QFont font(FontFamily, aFontSize, QFont::Bold);
	
	p.setFont(font);

 	pen.setColor(FontColor);
	
 	p.setPen(pen);
	if (m_CurrentState==QSubViewConnectStatus::STATUS_CONNECTING)
	{
		QString m_text;
		m_text.append(m_DevCliSetInfo.m_sEseeId);
		m_text+=QString(": %1").arg(m_DevCliSetInfo.m_uiChannelId+1);
		if (CountConnecting==4)
		{
			CountConnecting=0;
		}
		for (int i=0;i<CountConnecting;i++)
		{
			m_text+="..";
		}
		CountConnecting++;
		p.drawText(rcClient, Qt::AlignCenter, m_text);
	}
	else if (m_CurrentState==QSubViewConnectStatus::STATUS_DISCONNECTING)
	{
		QString m_text;
		m_text.append(m_DevCliSetInfo.m_sEseeId);
		m_text+=QString(": %1").arg(m_DevCliSetInfo.m_uiChannelId+1);
		if (CountDisConnecting==0)
		{
			CountConnecting=4;
		}
		for (int i=0;i<CountConnecting-1;i++)
		{
			m_text+="..";
		}
		CountConnecting--;
		p.drawText(rcClient, Qt::AlignCenter, m_text);
	}
	else{
		p.drawText(rcClient, Qt::AlignCenter, "No Video");
	}

}

void QSubView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
	//切换ipc主次码流
	if (this->parentWidget()->width()-this->width()<20)
	{
		if (NULL!=m_IDeviceClientDecideByVendor)
		{
			ISwitchStream *m_SwitchStream=NULL;
			m_IDeviceClientDecideByVendor->QueryInterface(IID_ISwitchStream,(void**)&m_SwitchStream);
			if (NULL!=m_SwitchStream)
			{
				m_SwitchStream->SwitchStream(0);
				m_SwitchStream->Release();
				m_SwitchStream=NULL;
			}
		}
	}
	else{
		if (NULL!=m_IDeviceClientDecideByVendor)
		{
			ISwitchStream *m_SwitchStream=NULL;
			m_IDeviceClientDecideByVendor->QueryInterface(IID_ISwitchStream,(void**)&m_SwitchStream);
			if (NULL!=m_SwitchStream)
			{
				m_SwitchStream->SwitchStream(1);
				m_SwitchStream->Release();
				m_SwitchStream=NULL;
			}
		}
	}

}



void QSubView::mousePressEvent(QMouseEvent *ev)
{
	setFocus(Qt::MouseFocusReason);
	if (ev->button()==Qt::RightButton)
	{
		emit RMousePressMenu();
	}
	emit mousePressEvent(this,ev);
	emit SetCurrentWindSignl(this);
}

int QSubView::GetCurrentWnd()
{
	return 1;
}

int QSubView::OpenCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sCameraname,const QString & sVendor)
{
	//保存设备信息
	m_DevCliSetInfo.m_sAddress.clear();
	m_DevCliSetInfo.m_sEseeId.clear();
	m_DevCliSetInfo.m_sUsername.clear();
	m_DevCliSetInfo.m_sPassword.clear();
	m_DevCliSetInfo.m_sCameraname.clear();
	m_DevCliSetInfo.m_sVendor.clear();
	m_DevCliSetInfo.m_sAddress=sAddress;
	m_DevCliSetInfo.m_uiPort=uiPort;
	m_DevCliSetInfo.m_sEseeId=sEseeId;
	m_DevCliSetInfo.m_uiChannelId=uiChannelId;
	m_DevCliSetInfo.m_uiStreamId=uiStreamId;
	m_DevCliSetInfo.m_sUsername=sUsername;
	m_DevCliSetInfo.m_sPassword=sPassword;
	m_DevCliSetInfo.m_sCameraname=sCameraname;
	m_DevCliSetInfo.m_sVendor=sVendor;
	//设置自动手动关闭标志位
	if (m_bStateAutoConnect==true)
	{
		In_CloseAutoConnect();
	}
	//关闭上一次的连接
	CloseWndCamera();
	//生成设备组件
	SetDeviceByVendor(sVendor);
	//注册事件，需检测是否注册成功
		if (1==cbInit())
		{
			if (NULL!=m_IDeviceClientDecideByVendor)
			{
				m_IDeviceClientDecideByVendor->Release();
				m_IDeviceClientDecideByVendor=NULL;
			}
			return 1;
		}
	/*SetCameraInWnd(sAddress,uiPort,sEseeId,uiChannelId,uiStreamId,sUsername,sPassword,sCameraname,sVendor);*/
	m_QSubViewObject.SetCameraInWnd(sAddress,uiPort,sEseeId,uiChannelId,uiStreamId,sUsername,sPassword,sCameraname,sVendor);
	//0.5s检测一次是否需要刷新历史图片
	m_RenderTimeId=startTimer(500);

	m_QSubViewObject.OpenCameraInWnd();

	m_checkTime.start(1000);

	return 0;
}
int QSubView::CloseWndCamera()
{
	In_CloseAutoConnect();
	m_QSubViewObject.CloseWndCamera();
	//释放动态生成的指针
	if (NULL!=m_IDeviceClientDecideByVendor)
	{
		m_IDeviceClientDecideByVendor->Release();
		m_IDeviceClientDecideByVendor=NULL;
	}
	//录像
	if (m_bIsRecording && NULL != m_pRecorder)
	{
		m_pRecorder->Stop();
		m_bIsRecording = false;
	}
	return 0;
}
int QSubView::GetWindowConnectionStatus()
{
	return m_CurrentState;
}


int QSubView::cbInit()
{
	//注册设备服务回调函数
	QString evName="LiveStream";
	IEventRegister *pRegist=NULL;
	if (NULL==m_IDeviceClientDecideByVendor)
	{
		return 1;
	}
	m_IDeviceClientDecideByVendor->QueryInterface(IID_IEventRegister,(void**)&pRegist);
	if (NULL==pRegist)
	{
		return 1;
	}
	pRegist->registerEvent(evName,cbLiveStream,this);
	evName.clear();
	evName.append("SocketError");
	pRegist->registerEvent(evName,cbConnectError,this);
	evName.clear();
	evName.append("CurrentStatus");
	pRegist->registerEvent(evName,cbStateChange,this);

	evName.clear();
	evName.append("ForRecord");
	pRegist->registerEvent(evName,cbForRecord,this);

	pRegist->Release();
	pRegist=NULL;
	//注册解码回调函数
	evName.clear();
	evName.append("DecodedFrame");
	if (NULL==m_IVideoDecoder)
	{
		return 1;
	}
	m_IVideoDecoder->QueryInterface(IID_IEventRegister,(void**)&pRegist);
	if (NULL==pRegist)
	{
		return 1;
	}
	pRegist->registerEvent(evName,cbDecodedFrame,this);
	pRegist->Release();
	pRegist=NULL;
	//初始化渲染器
	if (NULL==m_IVideoRender)
	{
		return 1;
	}
	if (0!=m_IVideoRender->setRenderWnd(this))
	{
		return 1;
	}
	return 0;
}
int QSubView::PrevPlay(QVariantMap evMap)
{
	unsigned int nLength=evMap.value("length").toUInt();
	char * lpdata=(char *)evMap.value("data").toUInt();

	if (NULL==m_IVideoDecoder)
	{
		return 1;
	}

	qDebug()<<"=======PrevPlay=======";
	m_IVideoDecoder->decode(lpdata,nLength);
	return 0;
}
int QSubView::CurrentStateChange(QVariantMap evMap)
{

	m_CurrentState=(QSubViewConnectStatus)evMap.value("CurrentStatus").toInt();
	
	
	if (1==m_CurrentState)
	{
		emit Connectting();
		QVariantMap evMapToUi;
		evMapToUi.insert("CurrentState",m_CurrentState);
		evMapToUi.insert("ChannelId",m_DevCliSetInfo.m_uiChannelIdInDataBase);
		emit CurrentStateChangeSignl(evMapToUi,this);
		qDebug()<<m_DevCliSetInfo.m_sEseeId<<m_DevCliSetInfo.m_uiChannelId<<"connecting";
	}
	if (0==m_CurrentState)
	{
		m_bIsAutoConnect=true;
		QVariantMap evMapToUi;
		evMapToUi.insert("CurrentState",m_CurrentState);
		evMapToUi.insert("ChannelId",m_DevCliSetInfo.m_uiChannelIdInDataBase);
		emit CurrentStateChangeSignl(evMapToUi,this);
		qDebug()<<m_DevCliSetInfo.m_sEseeId<<m_DevCliSetInfo.m_uiChannelId<<"connected";
	}
	if (3==m_CurrentState)
	{
		emit DisConnecting();
		QVariantMap evMapToUi;
		evMapToUi.insert("CurrentState",m_CurrentState);
		evMapToUi.insert("ChannelId",m_DevCliSetInfo.m_uiChannelIdInDataBase);
		emit CurrentStateChangeSignl(evMapToUi,this);
		qDebug()<<m_DevCliSetInfo.m_sEseeId<<m_DevCliSetInfo.m_uiChannelId<<"disconnecting";
	}
	if (2==m_CurrentState)
	{
		//断开后，把历史图片数据清空
		m_HistoryRenderInfo.pData=NULL;

		QVariantMap evMapToUi;
		evMapToUi.insert("CurrentState",m_CurrentState);
		evMapToUi.insert("ChannelId",m_DevCliSetInfo.m_uiChannelIdInDataBase);
		emit CurrentStateChangeSignl(evMapToUi,this);
		qDebug()<<m_DevCliSetInfo.m_sEseeId<<m_DevCliSetInfo.m_uiChannelId<<"disconnected";
	}
	
	
	/*emit CurrentStateChangeSignl(evMap.value("CurrentStatus").toInt(),this);*/
	//自动重连
	if (QSubViewConnectStatus::STATUS_DISCONNECTED==m_CurrentState&&QSubViewConnectStatus::STATUS_CONNECTED==m_HistoryState)
	{
		if (m_bIsAutoConnect==true)
		{
			if (m_bStateAutoConnect==false)
			{
				//开启自动重连时钟
				emit CreateAutoConnectTimeSignals();
				m_bStateAutoConnect=true;
			}
			else{

			}
		}
	}

	m_HistoryState=m_CurrentState;
	return 0;
}
int QSubView::PrevRender(QVariantMap evMap)
{
	QVariantMap::const_iterator it;
	for (it=evMap.begin();it!=evMap.end();++it)
	{
		QString sKey=it.key();
		QString sValue=it.value().toString();
	}
	if (NULL==m_IVideoRender)
	{
		return 1;
	}
	char* pData=(char*)evMap.value("data").toUInt();
	char* pYdata=(char*)evMap.value("Ydata").toUInt();
	char* pUdata=(char*)evMap.value("Udata").toUInt();
	char* pVdata=(char*)evMap.value("Vdata").toUInt();
	int iWidth=evMap.value("width").toInt();
	int iHeight=evMap.value("height").toInt();
	int iYStride=evMap.value("YStride").toInt();
	int iUVStride=evMap.value("UVStride").toInt();
	int iLineStride=evMap.value("lineStride").toInt();
	QString iPixeFormat=evMap.value("pixelFormat").toString();
	int iFlags=evMap.value("flags").toInt();

	if (iInitHeight!=iHeight||iInitWidth!=iWidth)
	{
		m_IVideoRender->init(iWidth,iHeight);
		iInitHeight=iHeight;
		iInitWidth=iWidth;
	}
	bRendering=true;
	m_csRender.lock();
	m_HistoryRenderInfo.pData=pData;
	m_HistoryRenderInfo.pYdata=pYdata;
	m_HistoryRenderInfo.pUdata=pUdata;
	m_HistoryRenderInfo.pVdata=pVdata;
	m_HistoryRenderInfo.iWidth=iWidth;
	m_HistoryRenderInfo.iHeight=iHeight;
	m_HistoryRenderInfo.iYStride=iYStride;
	m_HistoryRenderInfo.iUVStride=iUVStride;
	m_HistoryRenderInfo.iLineStride=iLineStride;
	m_HistoryRenderInfo.iPixeFormat=iPixeFormat;
	m_HistoryRenderInfo.iFlags=iFlags;
	m_bIsRenderHistory=false;
	m_IVideoRender->render(pData,pYdata,pUdata,pVdata,iWidth,iHeight,iYStride,iUVStride,iLineStride,iPixeFormat,iFlags);
	m_csRender.unlock();
	bRendering=false;
	return 0;
}



void QSubView::timerEvent( QTimerEvent * ev)
{
	if (m_CurrentState!=QSubViewConnectStatus::STATUS_CONNECTED)
	{
		update();
	}
	
	if (m_CurrentState==QSubViewConnectStatus::STATUS_CONNECTED)
	{
		if (ev->timerId()==m_DisConnectedTimeId)
		{
			killTimer(ev->timerId());
			m_DisConnectedTimeId=0;
		}
		else if (ev->timerId()==m_DisConnectingTimeId)
		{
			killTimer(ev->timerId());
			m_DisConnectingTimeId=0;
		}
	}
	else if (m_CurrentState==QSubViewConnectStatus::STATUS_DISCONNECTED)
	{
		if (ev->timerId()==m_DisConnectedTimeId)
		{
			killTimer(ev->timerId());
			m_DisConnectedTimeId=0;
		}
		else if (ev->timerId()==m_DisConnectingTimeId)
		{
			killTimer(ev->timerId());
			m_DisConnectingTimeId=0;
		}
		if (ev->timerId()==m_RenderTimeId)
		{
			killTimer(ev->timerId());
			m_RenderTimeId=0;
		}
	}
	if (m_RenderTimeId==ev->timerId())
	{
		emit RenderHistoryPix();
	}
	//自动重连 时钟信号
	if (m_AutoConnectTimeId==ev->timerId())
	{
		//关闭自动重连,设备连接上或者禁止自动重连
		if (m_bIsAutoConnect==false||QSubViewConnectStatus::STATUS_CONNECTED==m_CurrentState)
		{
			killTimer(ev->timerId());
			m_AutoConnectTimeId=0;
			m_bStateAutoConnect=false;
		}
		else if (m_bIsAutoConnect==true)
		{
			if (QSubViewConnectStatus::STATUS_CONNECTED!=m_CurrentState&&false==m_bIsAutoConnecting)
			{
				qDebug()<<"AutoConnectSignals";
				emit AutoConnectSignals();
			}
		}
	}
}

void QSubView::OnRMousePressMenu()
{
	m_RMousePressMenu.exec(QCursor::pos());
}

void QSubView::OnCloseFromMouseEv()
{
	CloseWndCamera();
}

int cbLiveStream(QString evName,QVariantMap evMap,void*pUser)
{
	//检测数据包，把数据包扔给解码器

	if (evName=="LiveStream")
	{
		((QSubView*)pUser)->PrevPlay(evMap);
		return 0;
	}
	else
		return 1;
}
int cbForRecord(QString evName,QVariantMap evMap,void*pUser)
{
	//检测数据包，把数据包扔给解码器

	if (evName=="ForRecord")
	{
		((QSubView*)pUser)->ForRecord(evMap);
		return 0;
	}
	else
		return 1;
}
int cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser)
{
	if (evName=="DecodedFrame")
	{
		((QSubView*)pUser)->PrevRender(evMap);
		return 0;
	}
	else 
		return 1;
}

int cbConnectError(QString evName,QVariantMap evMap,void*pUser)
{
	qDebug()<<"cbConnectError";

	QVariantMap::const_iterator it;
	for (it=evMap.begin();it!=evMap.end();++it)
	{
		QString sKey=it.key();
		QString sValue=it.value().toString();
	}
	return 1;
}

int cbStateChange(QString evName,QVariantMap evMap,void*pUser)
{
	QVariantMap::const_iterator it;
	for (it=evMap.begin();it!=evMap.end();++it)
	{
		QString sKey=it.key();
		QString sValue=it.value().toString();
	}
	if (evName=="CurrentStatus")
	{
		((QSubView*)pUser)->CurrentStateChange(evMap);
		return 0;
	}
	return 1;
}

int QSubView::StartRecord()
{
	if (NULL == m_pRecorder || m_bIsAutoRecording)
	{
		return 1;
	}

	m_bIsRecording = true;
	int nRet = m_pRecorder->Start();
	return nRet;
}

int QSubView::StopRecord()
{
	if (NULL == m_pRecorder || m_bIsAutoRecording)
	{
		return 1;
	}
	int nRet = m_pRecorder->Stop();
	m_bIsRecording = false;

	return nRet;
}

int QSubView::SetDevInfo(const QString &devname,int nChannelNum)
{
	if (NULL == m_pRecorder)
	{
		return 1;
	}

	int nRet = m_pRecorder->SetDevInfo(devname, nChannelNum);
	return nRet;
}

void QSubView::OnCheckTime()
{
	if (NULL == m_pRecorder || m_bIsRecording)
	{
		return;
	}

	if (NULL == m_pRecordTime)
	{
		//重新申请ISetRecordTime接口
		pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ISetRecordTime,(void **)&m_pRecordTime);
		if (NULL == m_pRecordTime)
		{
			return;
		}
	}

	QTime currentTime; 
	RecordTimeInfo recTimeInfo;
	if (0 == m_RecordFlushTime%600)
	{
		m_RecordFlushTime = 0;
		++m_RecordFlushTime;
		m_lstReocrdTimeInfoList.clear();
		QStringList recordIdList = m_pRecordTime->GetRecordTimeBydevId(m_DevCliSetInfo.m_uiChannelIdInDataBase);
		for (int i = 0; i < recordIdList.size(); i++)
		{
			QString recordID = recordIdList[i];
			QVariantMap timeInfo = m_pRecordTime->GetRecordTimeInfo(recordID.toInt());
			recTimeInfo.nEnable = timeInfo.value("enable").toInt();
			recTimeInfo.nWeekDay = timeInfo.value("weekday").toInt();
			int weekDay = QDate::currentDate().dayOfWeek();
			if (0 == recTimeInfo.nEnable && weekDay != recTimeInfo.nWeekDay)
			{
				continue;
			}

			currentTime = QTime::currentTime();
			recTimeInfo.startTime = QTime::fromString(timeInfo.value("starttime").toString().mid(11), "hh:mm:ss");
			recTimeInfo.endTime = QTime::fromString(timeInfo.value("endtime").toString().mid(11), "hh:mm:ss");
			if (!m_bIsAutoRecording && currentTime >= recTimeInfo.startTime && currentTime < recTimeInfo.endTime)
			{
				m_pRecorder->SetDevInfo(m_DevCliSetInfo.m_sEseeId, m_DevCliSetInfo.m_uiChannelId);
				m_pRecorder->Start();
				m_bIsAutoRecording = true;
			}

			if (m_bIsAutoRecording && currentTime >= recTimeInfo.endTime)
			{
				m_pRecorder->Stop();
				m_bIsAutoRecording = false;
			}
			m_lstReocrdTimeInfoList.append(recTimeInfo);
		}
	}
	else
	{
		for (int j = 0; j < m_lstReocrdTimeInfoList.size(); ++j)
		{
			if (0 == m_lstReocrdTimeInfoList[j].nEnable && m_lstReocrdTimeInfoList[j].nWeekDay)
			{
				continue;
			}
			currentTime = QTime::currentTime();
			if (!m_bIsAutoRecording && currentTime >= m_lstReocrdTimeInfoList[j].startTime && currentTime < m_lstReocrdTimeInfoList[j].endTime)
			{
				m_pRecorder->SetDevInfo(m_DevCliSetInfo.m_sEseeId, m_DevCliSetInfo.m_uiChannelId);
				m_pRecorder->Start();
				m_bIsAutoRecording = true;
			}
			if (m_bIsAutoRecording && currentTime >= m_lstReocrdTimeInfoList[j].endTime)
			{
				m_pRecorder->Stop();
				m_bIsAutoRecording = false;
			}
		}
	}
}

int QSubView::ForRecord( QVariantMap evMap )
{
	if (NULL != m_pRecorder)
	{
		m_pRecorder->InputFrame(evMap);
	}
	return 0;
}

int QSubView::SetDeviceByVendor( const QString & sVendor )
{
	QString sAppPath=QCoreApplication::applicationDirPath();
	QFile *file=new QFile(sAppPath+"/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConFile;
	ConFile.setContent(file);
	QDomNode clsidNode=ConFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList=clsidNode.childNodes();
	int n;
	for (n=0;n<itemList.count();n++)
	{
		QDomNode item=itemList.at(n);
		QString sItemName=item.toElement().attribute("vendor");
		if (sItemName==sVendor)
		{
			CLSID DeviceVendorClsid=pcomString2GUID(item.toElement().attribute("clsid"));
			if (NULL!=m_IDeviceClientDecideByVendor)
			{
				m_IDeviceClientDecideByVendor->Release();
				m_IDeviceClientDecideByVendor=NULL;
			}
			pcomCreateInstance(DeviceVendorClsid,NULL,IID_IDeviceClient,(void**)&m_IDeviceClientDecideByVendor);
			if (NULL!=m_IDeviceClientDecideByVendor)
			{
				//设置主次码流
				if (this->parentWidget()->width()==this->width())
				{
					if (NULL!=m_IDeviceClientDecideByVendor)
					{
						ISwitchStream *m_SwitchStream=NULL;
						m_IDeviceClientDecideByVendor->QueryInterface(IID_ISwitchStream,(void**)&m_SwitchStream);
						if (NULL!=m_SwitchStream)
						{
							m_SwitchStream->SwitchStream(0);
							m_SwitchStream->Release();
							m_SwitchStream=NULL;
						}
					}
				}
				else{
					if (NULL!=m_IDeviceClientDecideByVendor)
					{
						ISwitchStream *m_SwitchStream=NULL;
						m_IDeviceClientDecideByVendor->QueryInterface(IID_ISwitchStream,(void**)&m_SwitchStream);
						if (NULL!=m_SwitchStream)
						{
							m_SwitchStream->SwitchStream(1);
							m_SwitchStream->Release();
							m_SwitchStream=NULL;
						}
					}
				}

				m_QSubViewObject.SetDeviceClient(m_IDeviceClientDecideByVendor);				
				return 0;
			}		
		}
	}
	return 1;
}

void QSubView::In_OpenAutoConnect()
{
	//关闭上一次的连接
	m_bIsAutoConnecting=true;
	m_QSubViewObject.CloseWndCamera();
	//释放动态生成的指针
	if (NULL!=m_IDeviceClientDecideByVendor)
	{
		m_IDeviceClientDecideByVendor->Release();
		m_IDeviceClientDecideByVendor=NULL;
	}
	//录像
	if (m_bIsRecording && NULL != m_pRecorder)
	{
		m_pRecorder->Stop();
		m_bIsRecording = false;
	}
	//生成设备组件
	SetDeviceByVendor(m_DevCliSetInfo.m_sVendor);
	//注册事件，需检测是否注册成功
	if (1==cbInit())
	{
		if (NULL!=m_IDeviceClientDecideByVendor)
		{
			m_IDeviceClientDecideByVendor->Release();
			m_IDeviceClientDecideByVendor=NULL;
		}
		return ;
	}
	m_QSubViewObject.SetCameraInWnd(m_DevCliSetInfo.m_sAddress,m_DevCliSetInfo.m_uiPort,m_DevCliSetInfo.m_sEseeId,m_DevCliSetInfo.m_uiChannelId,m_DevCliSetInfo.m_uiStreamId,m_DevCliSetInfo.m_sUsername,m_DevCliSetInfo.m_sPassword,m_DevCliSetInfo.m_sCameraname,m_DevCliSetInfo.m_sVendor);
	//0.5s检测一次是否需要刷新历史图片
	m_RenderTimeId=startTimer(500);

	m_QSubViewObject.OpenCameraInWnd();

	m_checkTime.start(1000);
	m_bIsAutoConnecting=false;
}

void QSubView::OnConnectting()
{
	startTimer(500);
}

void QSubView::OnDisConnecting()
{
	int m_DisConnectingTimeId=startTimer(500);
}

void QSubView::OnDisConnected()
{
	int m_DisConnectedTimeId=startTimer(500);
}

void QSubView::OnRenderHistoryPix()
{
	if (m_CurrentState==QSubViewConnectStatus::STATUS_CONNECTED)
	{
		if (NULL!=m_IVideoRender&&NULL!=m_HistoryRenderInfo.pData&&true==m_bIsRenderHistory)
		{
			m_csRender.lock();
			m_IVideoRender->render(m_HistoryRenderInfo.pData,m_HistoryRenderInfo.pYdata,m_HistoryRenderInfo.pUdata,m_HistoryRenderInfo.pVdata,m_HistoryRenderInfo.iWidth,m_HistoryRenderInfo.iHeight,m_HistoryRenderInfo.iYStride,m_HistoryRenderInfo.iUVStride,m_HistoryRenderInfo.iLineStride,m_HistoryRenderInfo.iPixeFormat,m_HistoryRenderInfo.iFlags);
			m_csRender.unlock();
		}
		m_bIsRenderHistory=true;
	}
}

void QSubView::In_CloseAutoConnect()
{
	//关闭自动重连的时钟
	if (m_AutoConnectTimeId!=0)
	{
		killTimer(m_AutoConnectTimeId);
		m_AutoConnectTimeId=0;
	}
	//关闭连接
	m_bStateAutoConnect=false;
	m_bIsAutoConnect=false;
}

void QSubView::OnCreateAutoConnectTime()
{
	m_AutoConnectTimeId=startTimer(10000);
}

int QSubView::SetDevChannelInfo( int ChannelId )
{
	m_DevCliSetInfo.m_uiChannelIdInDataBase=ChannelId;
	return 0;
}
