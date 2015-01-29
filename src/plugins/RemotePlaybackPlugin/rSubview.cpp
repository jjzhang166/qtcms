#include <QtCore>
#include <QtGui/QPainter>
#include <QSettings>
#include <QMouseEvent>
#include <guid.h>
#include "rSubview.h"
#include "IUserManagerEx.h"
#include <QTime>

bool RSubView::m_bGlobalAudioStatus = false;
bool RSubView::m_bSuspensionVisable = false;
SuspensionWnd* RSubView::ms_susWnd = NULL;
QMap<quintptr, QRect> RSubView::ms_rectMap;

RSubView::RSubView(QWidget *parent)
	: QWidget(parent),
	m_LpClient(NULL),
	m_pRemotePlayBack(NULL),
	ui(new Ui::titleview),
	_curCache(0),
	_bSaveCacheImage(false),
	_cacheLable(NULL),
	_bIsFocus(false)
{
	this->lower();
	this->setAttribute(Qt::WA_PaintOutsidePaintEvent);

	connect(&m_checkTime,SIGNAL(timeout()),this,SLOT(connecttingUpdate()));

	connect(this,SIGNAL(connecttingUpdateSig()),this,SLOT(connecttingUpdateSlot()),Qt::DirectConnection);
	connect(this,SIGNAL(CacheStateSig(QVariantMap)),this,SLOT(CacheStateSlot(QVariantMap)));

	_curState=CONNECT_STATUS_DISCONNECTED;

	if (!ms_susWnd){
		ms_susWnd = new SuspensionWnd(this);
		connect(ms_susWnd, SIGNAL(sigClose()), this, SLOT(slCloseSusWnd()));
		ms_susWnd->setWindowFlags(Qt::Window);
		ms_susWnd->setWindowFlags(this->windowFlags() &~ (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint));
		ms_susWnd->setCbFunc(cbReciveMsg, this);
		ms_susWnd->setWindowTitle(tr("Zoom"));
	}
}

RSubView::~RSubView()
{
	delete ui;
	if (NULL!=m_LpClient)
	{
		m_LpClient->Release();
	}
	m_checkTime.stop();
	m_cacheTime.stop();
	if (NULL!=_cacheLable)
	{
		delete _cacheLable;
	}

// 	if (ms_susWnd){
// 		ms_susWnd->close();
// 		delete ms_susWnd;
// 		ms_susWnd = NULL;
// 	}
}

void RSubView::paintEvent( QPaintEvent * e)
{
	paintEventConnecting(e);
	paintEventNoVideo(e);
	paintEventCache(e);
}

void RSubView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
}
void RSubView::resizeEvent(QResizeEvent *e)
{
}
void RSubView::mousePressEvent(QMouseEvent *ev)
{
	m_bPressed = true;
	m_pressPoint = ev->pos();

	if (ms_susWnd->isVisible()){
		//notify play module current window need to zoom
		QVariantMap msg;
		msg.insert("SusWnd", (quintptr)ms_susWnd);
		msg.insert("CurWnd", (quintptr)this);
		if (ms_rectMap.contains((quintptr)this)){
			QRect drawRect = ms_rectMap[(quintptr)this];
			//Coordinate Conversion
			float widthRate = (float)ms_susWnd->width()/this->width();
			float heightRate = (float)ms_susWnd->height()/this->height();
			drawRect.setCoords(drawRect.left()*widthRate, drawRect.top()*heightRate, drawRect.right()*widthRate, drawRect.bottom()*heightRate);
			ms_rectMap[(quintptr)this] = drawRect;

			msg.insert("ZoRect", ms_rectMap[(quintptr)this]);
			ms_susWnd->setDrawRect(ms_rectMap[(quintptr)this]);
		}else{
			msg.insert("ZoRect", QRect(1, 1, 1, 1));
			ms_susWnd->setDrawRect(QRect(0, 0, 0, 0));
		}
		msg.insert("Width", ms_susWnd->width());
		msg.insert("Height", ms_susWnd->height());
		ms_susWnd->addWnd(this);
		m_pcbfn(QString("VedioZoom"), msg, m_pUser);
	}

	setFocus(Qt::MouseFocusReason);
	emit SetCurrentWindSignl(this);
	if (ev->button() == Qt::LeftButton && m_bGlobalAudioStatus && NULL != m_pRemotePlayBack)
	{
		m_pRemotePlayBack->GroupSetVolume(0xAECBCA, this);
	}
	if (0 == _curCache)
	{
		if (NULL!=_cacheLable)
		{
			_cacheLable->hide();
		}
	}
	m_pressPoint = ev->pos();
}

void RSubView::SetLpClient( IDeviceGroupRemotePlayback *m_GroupPlayback )
{
	if (NULL==m_GroupPlayback)
	{
		return;
	}
	m_GroupPlayback->QueryInterface(IID_IDeviceClient,(void**)&m_LpClient);
	m_pRemotePlayBack = m_GroupPlayback;
	return;
}

int RSubView::AudioEnabled(bool bEnabled)
{
	if (NULL == m_pRemotePlayBack)
	{
		return 1;
	}
	m_pRemotePlayBack->GroupEnableAudio(bEnabled);
	m_bGlobalAudioStatus = bEnabled;
		
	return 0;
}

void RSubView::SetCurConnectState( __enConnectStatus parm  )
{
	_curState=parm;

	/*emit connecttingUpdateSig();*/
}

void RSubView::CacheState( QVariantMap evMap )
{
// 	int *wind = reinterpret_cast<int*>(this);
	if (evMap.value("wind").toInt() == (int)this)
	{
		emit CacheStateSig(evMap);
	}
}

void RSubView::connecttingUpdateSlot()
{
	if (_curState==CONNECT_STATUS_CONNECTING)
	{
		m_checkTime.start(1000);
		_countConnecting=3;

	}else 
	{
		m_checkTime.stop();
		_countConnecting=0;
	}
}

void RSubView::connecttingUpdate()
{
	update();
}

void RSubView::CacheStateSlot( QVariantMap evMap )
{
	_curCache=evMap.value("Persent").toInt();
	update();
	_cacheLableShow();
}



void RSubView::paintEventNoVideo( QPaintEvent * )
{
	if (_curState==CONNECT_STATUS_DISCONNECTED)
	{
		//
		QPainter p(this);

		QString image;
		QColor LineColor(45,49,54);
		QColor FontColor(32,151,219);
		QColor LineCurColor;
		int FontSize;
		QString FontFamily;
		QString sBackground;

		QString sAppPath = QCoreApplication::applicationDirPath();
		QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
		QSettings IniFile(path, QSettings::IniFormat, 0);

		image = IniFile.value("background/background-image", NULL).toString();
	/*	LineColor.setNamedColor(IniFile.value("background/background-color", NULL).toString());*/
		LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
		FontColor.setNamedColor(IniFile.value("font/font-color", NULL).toString());
		FontSize = IniFile.value("font/font-size", NULL).toString().toInt();
		FontFamily = IniFile.value("font/font-family", NULL).toString();
		sBackground=IniFile.value("text/background", QVariant("")).toString();

		QRect rcClient = contentsRect();

		QPixmap pix;
		QString PixPaht = sAppPath + image;
		bool ret = pix.load(PixPaht);

		pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);

		p.drawPixmap(rcClient,pix);

		QPen pen = QPen(LineColor, 2);
		p.setPen(pen);

		p.drawRect(rcClient);
		if (_bIsFocus)
		{
			int x = 0;
			int y = 0;
			int width = 0;
			int height = 0;
			rcClient.getCoords(&x, &y, &width, &height);
			pen.setWidth(2);
			pen.setColor(LineCurColor);
			p.setPen(pen);
			p.drawRect(QRect(x,y,width, height));
		}
		else
		{
			p.drawRect(rcClient);
		}

		QFont font(FontFamily, FontSize, QFont::Bold|QFont::System);

		p.setFont(font);

		pen.setColor(FontColor);

		p.setPen(pen);

		p.drawText(rcClient, Qt::AlignCenter, sBackground);
	}
}

void RSubView::paintEventConnecting( QPaintEvent * )
{
	if (_curState==CONNECT_STATUS_CONNECTING)
	{
		QPainter p(this);

		QString image;
		QColor LineColor(45,49,54);
		QColor LineCurColor;
		QColor FontColor(32,151,219);
		int FontSize;
		QString FontFamily;
		QString sBackground;

		QString sAppPath = QCoreApplication::applicationDirPath();
		QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
		QSettings IniFile(path, QSettings::IniFormat, 0);

		image = IniFile.value("background/background-image", NULL).toString();
		/*LineColor.setNamedColor(IniFile.value("background/background-color", NULL).toString());*/
		LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
		FontColor.setNamedColor(IniFile.value("font/font-color", NULL).toString());
		FontSize = IniFile.value("font/font-size", NULL).toString().toInt();
		FontFamily = IniFile.value("font/font-family", NULL).toString();
		sBackground=IniFile.value("text/background", QVariant("")).toString();

		QRect rcClient = contentsRect();

		QPixmap pix;
		QString PixPaht = sAppPath + image;
		bool ret = pix.load(PixPaht);

		pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);

		p.drawPixmap(rcClient,pix);

		QPen pen = QPen(LineColor, 2);
		p.setPen(pen);

		p.drawRect(rcClient);
		if (_bIsFocus)
		{
			int x = 0;
			int y = 0;
			int width = 0;
			int height = 0;
			rcClient.getCoords(&x, &y, &width, &height);
			pen.setWidth(2);
			pen.setColor(LineCurColor);
			p.setPen(pen);
			p.drawRect(QRect(x,y,width, height));
		}
		else
		{
			p.drawRect(rcClient);
		}

		QFont font(FontFamily, FontSize, QFont::Bold|QFont::System);
		p.setFont(font);

		pen.setColor(FontColor);

		p.setPen(pen);
		if (_curState==CONNECT_STATUS_CONNECTING)
		{
			QString m_test;
			if (_countConnecting==3)
			{
				_countConnecting=0;
			}

			for (int i=0;i<_countConnecting;i++)
			{
				m_test+="..";
			}
			_countConnecting++;
			p.drawText(rcClient, Qt::AlignCenter, m_test);
		}
	}
}

void RSubView::paintEventCache( QPaintEvent * )
{
	if (_curState==CONNECT_STATUS_CONNECTED)
	{
		QPainter p(this);

		QString image;
		/*QColor LineColor;*/
		QColor LineColor(45,49,54);
		QColor LineCurColor;
		QColor FontColor(32,151,219);
		int FontSize;
		QString FontFamily;
		QString sBackground;

		QString sAppPath = QCoreApplication::applicationDirPath();
		QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
		QSettings IniFile(path, QSettings::IniFormat, 0);

		image = IniFile.value("background/background-image", NULL).toString();
		/*LineColor.setNamedColor(IniFile.value("background/background-color", NULL).toString());*/
		LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
		FontColor.setNamedColor(IniFile.value("font/font-color", NULL).toString());
		FontSize = IniFile.value("font/font-size", NULL).toString().toInt();
		FontFamily = IniFile.value("font/font-family", NULL).toString();
		sBackground=IniFile.value("text/background", QVariant("")).toString();

		QRect rcClient = contentsRect();
		this->geometry().center();
		QPixmap pix;
		QString PixPaht = sAppPath + image;
		pix.load(PixPaht);

		pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
		//±³¾°
		QPixmap m_cacheImage=_cacheBackImage.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
		p.drawPixmap(rcClient,m_cacheImage);
		//±ß¿ò
		QPen pen = QPen(LineColor);
		pen.setWidth(2);
		p.setPen(pen);

		p.drawRect(rcClient);
		//½¹µã

		if (_bIsFocus)
		{
			int x = 0;
			int y = 0;
			int width = 0;
			int height = 0;
			rcClient.getCoords(&x, &y, &width, &height);
			pen.setWidth(2);
			pen.setColor(LineCurColor);
			p.setPen(pen);
			p.drawRect(QRectF(x,y,width, height));
		}
		else
		{
			p.drawRect(rcClient);
		}
	}
}

void RSubView::saveCacheImage()
{
	_cacheBackImage=QPixmap::grabWindow(this->winId(),0,0,this->width(),this->height());

}

void RSubView::_cacheLableShow()
{
	if (_cacheLable==NULL)
	{
		_cacheLable=new QLabel(this);
		_cacheLable->setParent(this);
		_cacheLable->raise();
		QPalette pa;
		QColor FontColor(32,151,219);
		pa.setColor(QPalette::WindowText,FontColor);
		_cacheLable->setPalette(pa);
		QFont ft;
		ft.setPointSize(20);
		_cacheLable->setFont(ft);
	}
	if (_cacheLable->isHidden())
	{
		_cacheLable->show();

	}else if (_curCache==100)
	{
		_cacheLable->hide();
	}
	QString cachePercent("load ");
	cachePercent+=QString("%1").arg(_curCache);
	cachePercent.append("%");
	_cacheLable->setText(cachePercent);
	_cacheLable->adjustSize();
	_cacheLable->move(this->width()/2,this->height()-35);
}

QVariantMap RSubView::ScreenShot()
{
	_ScreenShotImage=QPixmap::grabWindow(this->winId(),0,0,this->width(),this->height());
	QString dir=QCoreApplication::applicationDirPath();
	dir.append("/temp");
	QDir temp;
	bool exist=temp.exists(dir);
	if (exist==false)
	{
		temp.mkdir(dir);
	}
	QDateTime mtime=QDateTime::currentDateTime();
	uint mutime=mtime.toTime_t();
	QString imageName;
	imageName.append(dir);
	imageName+="/";
	imageName+=QString::number(mutime);
	imageName+=".jpg";
	_ScreenShotImage.save(imageName);
	QVariantMap item;
	item.insert("imageName",QString::number(mutime).append(".jpg"));
	item.insert("path",dir);
	return item;
}

void RSubView::SetFoucs( bool flags )
{
	bool bhistory=_bIsFocus;
	_bIsFocus=flags;
	if (_bIsFocus==true||bhistory==true)
	{
		update();
	}
}

void RSubView::setProgress( int progress )
{
	_curCache = progress;
	update();
	_cacheLableShow();
}

void RSubView::mouseReleaseEvent( QMouseEvent *ev )
{
	QRect mainRect = this->rect();
	QRect drawRect(m_pressPoint, ev->pos());
	m_bPressed = false;

	if (drawRect.width()*drawRect.height() < 1000){
		clearOriginRect();
		return;
	}
	//if release point in current window
	if (drawRect.width()*drawRect.height()/1000 && mainRect.contains(drawRect) 
		&& (QWidget*)this != ms_susWnd->getTopWnd() 
		&& CONNECT_STATUS_CONNECTED == _curState
		&& !ms_susWnd->isVisible()){
		//validation
		if (verify(100, 0)){
			clearOriginRect();
			return;
		}
		//Coordinate Conversion
		float widthRate = (float)ms_susWnd->width()/this->width();
		float heightRate = (float)ms_susWnd->height()/this->height();
		drawRect.setCoords(drawRect.left()*widthRate, drawRect.top()*heightRate, drawRect.right()*widthRate, drawRect.bottom()*heightRate);

		ms_susWnd->addWnd(this);
		ms_susWnd->setDrawRect(drawRect);
		ms_susWnd->show();
		ms_susWnd->setOriginGeog(ms_susWnd->geometry());
		//notify play module current window need to zoom
		QVariantMap msg;
		msg.insert("SusWnd", (quintptr)ms_susWnd);
		msg.insert("CurWnd", (quintptr)this);
		msg.insert("ZoRect", drawRect);
		msg.insert("Width", ms_susWnd->width());
		msg.insert("Height", ms_susWnd->height());
		m_pcbfn(QString("VedioZoom"), msg, m_pUser);
		ms_rectMap[(quintptr)this] = drawRect;
		m_bSuspensionVisable = true;
	}

}

void RSubView::setCbpfn( pfnCb cbPro, void* pUser )
{
	m_pcbfn = cbPro;
	m_pUser = pUser;
}

void RSubView::recMsg( QVariantMap msg )
{
	QString evName = msg["EvName"].toString();
	msg.remove("EvName");
	if (m_pcbfn && m_pUser){
		m_pcbfn(evName, msg, m_pUser);
	}
	if ("ZoomRect" == evName){
		ms_rectMap[msg["CurWnd"].toUInt()] = msg["ZoRect"].toRect();
	}
}

void RSubView::mouseMoveEvent( QMouseEvent *ev )
{
	QRect mainRect = this->rect();
	QRect drawRect(m_pressPoint, ev->pos());
	if (m_bPressed && mainRect.contains(drawRect) && drawRect.width()*drawRect.height()/1000 && CONNECT_STATUS_CONNECTED == _curState){
		//notify play module current window need to zoom
		QVariantMap msg;
		msg.insert("CurWnd", (quintptr)this);
		msg.insert("ZoRect", drawRect);
		m_pcbfn(QString("RectToOrigion"), msg, m_pUser);
	}
}

void RSubView::slCloseSusWnd()
{
	m_pcbfn(QString("CloseWnd"), QVariantMap(), m_pUser);
	ms_rectMap.clear();
	ms_susWnd->hide();
	m_bSuspensionVisable = false;
}

void RSubView::showSusWnd( bool enabled )
{
	if (enabled && m_bSuspensionVisable){
		ms_susWnd->show();
	}else{
		ms_susWnd->hide();
	}
}

void RSubView::destroySusWnd()
{
	if (ms_susWnd){
// 		ms_susWnd->close();
		slCloseSusWnd();
		delete ms_susWnd;
		ms_susWnd = NULL;
	}
}

void RSubView::changeEvent( QEvent *ev )
{
	if (ms_susWnd && QEvent::LanguageChange == ev->type()){
		ms_susWnd->setWindowTitle(tr("Zoom"));
	}
}

bool RSubView::verify( quint64 mainCode, quint64 subCode )
{
	int ret = -1;
	IUserManagerEx *pUserMgr = NULL;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IUserMangerEx,(void **)&pUserMgr);
	if (pUserMgr){
		ret = pUserMgr->checkUserLimit(mainCode, subCode);
		if (ret){
			QVariantMap vmap;
			vmap.insert("MainPermissionCode", qint64(mainCode));
			vmap.insert("SubPermissionCode", qint64(subCode));
			vmap.insert("ErrorCode", ret);
			emit sigValidateFail(vmap);
		}
		pUserMgr->Release();
	}
	return ret;
}

void RSubView::clearOriginRect()
{
	if (m_pcbfn && m_pUser){
		QVariantMap msg;
		msg.insert("CurWnd", (quintptr)this);
		msg.insert("ZoRect", QRect(1, 1, 1, 1));
		m_pcbfn(QString("RectToOrigion"), msg, m_pUser);
	}
}

void cbReciveMsg( QVariantMap evMap, void* pUser )
{
	((RSubView*)pUser)->recMsg(evMap);
}
