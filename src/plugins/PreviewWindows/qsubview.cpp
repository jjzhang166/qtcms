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
	m_IDeviceClient(NULL),
	m_pRecorder(NULL),
	m_pRecordTime(NULL),
	iInitHeight(0),
	iInitWidth(0),
	bIsInitFlags(false),
	bRendering(false),
	ui(new Ui::titleview),
	m_QActionCloseView(NULL),
	m_CurrentState(QSubView::QSubViewConnectStatus::STATUS_DISCONNECTED)
{
	this->lower();
	this->setAttribute(Qt::WA_PaintOutsidePaintEvent);
	//申请解码器接口
	pcomCreateInstance(CLSID_h264Decoder,NULL,IID_IVideoDecoder,(void**)&m_IVideoDecoder);
	qDebug("IVideoDecoder:%x",m_IVideoDecoder);
	//申请渲染器接口
	pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_IVideoRender);
	qDebug("m_IVideoRender:%x",m_IVideoRender);
	//申请DeviceClient接口
	pcomCreateInstance(CLSID_DeviceClient,NULL,IID_IDeviceClient,(void**)&m_IDeviceClient);
	qDebug("m_IDeviceClient:%x",m_IDeviceClient);
	//申请IRecorder接口
	pcomCreateInstance(CLSID_Recorder,NULL,IID_IRecorder,(void **)&m_pRecorder);

	//申请ISetRecordTime接口
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ISetRecordTime,(void **)&m_pRecordTime);
	connect(&m_checkTime, SIGNAL(timeout()), this, SLOT(OnCheckTime()));

	connect(this,SIGNAL(FreshWindow()),this,SLOT(OnFreshWindow()),Qt::QueuedConnection);
	m_QSubViewObject.SetDeviceClient(m_IDeviceClient);

	m_QActionCloseView=m_RMousePressMenu.addAction("close view");
	connect(this,SIGNAL(RMousePressMenu()),this,SLOT(OnRMousePressMenu()));
	connect(m_QActionCloseView,SIGNAL(triggered(bool)),this,SLOT(OnCloseFromMouseEv()));
}

QSubView::~QSubView()
{
	CloseWndCamera();

	if (NULL!=m_IDeviceClient)
	{
		m_IDeviceClient->Release();
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
	//if (IDeviceClient::STATUS_CONNECTED==GetWindowConnectionStatus())
	//{
	//	return;
	//}
	QPainter p(this);

	QString image;
	QColor LineColor;
	QColor FontColor;
	int FontSize;
	QString FontFamily;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);

	image = IniFile.value("background/background-image", NULL).toString();
	LineColor.setNamedColor(IniFile.value("background/background-color", NULL).toString());
	FontColor.setNamedColor(IniFile.value("font/font-color", NULL).toString());
	FontSize = IniFile.value("font/font-size", NULL).toString().toInt();
	FontFamily = IniFile.value("font/font-family", NULL).toString();

 	QRect rcClient = contentsRect();
 
 	QPixmap pix;
	QString PixPaht = sAppPath + image;
 	bool ret = pix.load(PixPaht);
 
  	pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);

 	p.drawPixmap(rcClient,pix);

	QPen pen = QPen(LineColor, 2);
 	p.setPen(pen);

	p.drawRect(rcClient);
	if (this->hasFocus())
	{
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		rcClient.getCoords(&x, &y, &width, &height);
		p.drawRect(QRect(x + 2,y + 2,width - 2, height - 2));
	}
	else
	{
	 	p.drawRect(rcClient);
	}

	QFont font(FontFamily, FontSize, QFont::Bold);
	
	p.setFont(font);

 	pen.setColor(FontColor);
	
 	p.setPen(pen);

	p.drawText(rcClient, Qt::AlignCenter, "No Video");

}

void QSubView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
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
int QSubView::SetCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sCameraname,const QString & sVendor)
{
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
	m_QSubViewObject.SetCameraInWnd(sAddress,uiPort,sEseeId,uiChannelId,uiStreamId,sUsername,sPassword,sCameraname,sVendor);
	return 0;
}
int QSubView::OpenCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sCameraname,const QString & sVendor)
{
	//注册事件，需检测是否注册成功
	if (false==bIsInitFlags)
	{
		if (1==cbInit())
		{
			return 1;
		}
	}
	m_CurrentState=QSubView::QSubViewConnectStatus::STATUS_CONNECTING;
	SetCameraInWnd(sAddress,uiPort,sEseeId,uiChannelId,uiStreamId,sUsername,sPassword,sCameraname,sVendor);
	m_QSubViewObject.SetCameraInWnd(sAddress,uiPort,sEseeId,uiChannelId,uiStreamId,sUsername,sPassword,sCameraname,sVendor);
	m_QSubViewObject.OpenCameraInWnd();

	m_checkTime.start(1000);

	return 0;
}
int QSubView::CloseWndCamera()
{
	m_CurrentState=QSubView::QSubViewConnectStatus::STATUS_DISCONNECTING;
	m_QSubViewObject.CloseWndCamera();
	return 0;
}
int QSubView::GetWindowConnectionStatus()
{
	//if (NULL==m_IDeviceClient)
	//{
	//	return 0;
	//}
	//return m_IDeviceClient->getConnectStatus();
	return m_CurrentState;
}


int QSubView::cbInit()
{
	//注册设备服务回调函数
	QString evName="LiveStream";
	IEventRegister *pRegist=NULL;
	if (NULL==m_IDeviceClient)
	{
		return 1;
	}
	m_IDeviceClient->QueryInterface(IID_IEventRegister,(void**)&pRegist);
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

	bIsInitFlags=true;
	return 0;
}
int QSubView::PrevPlay(QVariantMap evMap)
{
	unsigned int nLength=evMap.value("length").toUInt();
	char * lpdata=(char *)evMap.value("data").toUInt();

	if (NULL != m_pRecorder)
	{
		m_pRecorder->InputFrame(evMap);
	}

	if (NULL==m_IVideoDecoder)
	{
		return 1;
	}
	m_IVideoDecoder->decode(lpdata,nLength);
	return 0;
}
int QSubView::CurrentStateChange(QVariantMap evMap)
{
	if (evMap.value("CurrentStatus").toInt() == IDeviceClient::STATUS_DISCONNECTED)
	{
		emit FreshWindow();
	}
	m_CurrentState=(QSubViewConnectStatus)evMap.value("CurrentStatus").toInt();
	emit CurrentStateChangeSignl(evMap.value("CurrentStatus").toInt(),this);

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
	m_IVideoRender->render(pData,pYdata,pUdata,pVdata,iWidth,iHeight,iYStride,iUVStride,iLineStride,iPixeFormat,iFlags);
	bRendering=false;
	return 0;
}

void QSubView::OnFreshWindow()
{
	startTimer(400);
}

//void QSubView::emitOnFreshWindow()
//{
//	emit FreshWindow();
//}

void QSubView::timerEvent( QTimerEvent * ev)
{
	qDebug()<<this;
	repaint(this->x(),this->y(),this->width(),this->height());
	killTimer(ev->timerId());
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
	qDebug()<<"cbStateChange";
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
	if (NULL == m_pRecorder)
	{
		return 1;
	}

	int nRet = m_pRecorder->Start();
	return nRet;
}

int QSubView::StopRecord()
{
	if (NULL == m_pRecorder)
	{
		return 1;
	}
	int nRet = m_pRecorder->Stop();

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
	if (NULL == m_pRecorder)
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

	static bool recording = false;
	QStringList recordIdList = m_pRecordTime->GetRecordTimeBydevId(m_DevCliSetInfo.m_uiChannelId);
	for (int i = 0; i < recordIdList.size(); i++)
	{
		QString recordID = recordIdList[i];
		QVariantMap timeInfo = m_pRecordTime->GetRecordTimeInfo(recordID.toInt());
		if (1 == timeInfo.value("enable").toInt())
		{
			continue;
		}

		QDateTime currentTime = QDateTime::currentDateTime();
		QDateTime timeStart = QDateTime::fromString(timeInfo.value("starttime").toString(), "yyyy-MM-dd hh:mm:ss");
		QDateTime timeEnd = QDateTime::fromString(timeInfo.value("endtime").toString(), "yyyy-MM-dd hh:mm:ss");
		if (currentTime >= timeStart && currentTime < timeEnd)
		{
			m_pRecorder->SetDevInfo(m_DevCliSetInfo.m_sEseeId, m_DevCliSetInfo.m_uiChannelId);
			m_pRecorder->Start();
			recording = true;
		}

		if (currentTime >= timeEnd && recording)
		{
			m_pRecorder->Stop();
			recording = false;
		}
	}
}