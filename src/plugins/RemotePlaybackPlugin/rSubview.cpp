#include <QtCore>
#include <QtGui/QPainter>
#include <QSettings>
#include <QMouseEvent>
#include <guid.h>
#include "rSubview.h"
#include <QTime>

RSubView* RSubView::m_pCurView = NULL;
bool RSubView::m_bLocalAudioStatus = false;
bool RSubView::m_bGlobalAudioStatus = false;

RSubView::RSubView(QWidget *parent)
	: QWidget(parent),
	m_LpClient(NULL),
	m_pRemotePlayBack(NULL),
	ui(new Ui::titleview),
	_curCache(0)
{
	this->lower();
	this->setAttribute(Qt::WA_PaintOutsidePaintEvent);


	m_ActionOpenAudio = m_rMousePressMenu.addAction("open audio");
	connect(this,SIGNAL(RMousePressMenu()),this,SLOT(OnRMousePressMenu()));
	connect(m_ActionOpenAudio, SIGNAL(triggered(bool)), this, SLOT(OnOpenAudio()));

	connect(&m_checkTime,SIGNAL(timeout()),this,SLOT(connecttingUpdate()));
	connect(&m_cacheTime,SIGNAL(timeout()),this,SLOT(CacheStateSlotUpdate()));
	connect(this,SIGNAL(connecttingUpdateSig()),this,SLOT(connecttingUpdateSlot()));
	connect(this,SIGNAL(CacheStateSig(QVariantMap)),this,SLOT(CacheStateSlot(QVariantMap)));

	_curState=CONNECT_STATUS_DISCONNECTED;
	_curPaint=PAINTEVENT_STATUS_NOVIDEO;
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

}

void RSubView::paintEvent( QPaintEvent * e)
{
	paintEventConnecting(e);
	paintEventNoVideo(e);
	paintEventCache(e);
}

void RSubView::paintEventCache( QPaintEvent * )
{
	if (_curPaint==PAINTEVENT_STATUS_CACHE)
	{
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


		_cacheBackImage.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
		p.drawPixmap(rcClient,_cacheBackImage);

	}
}

void RSubView::saveCacheImage()
{
	_cacheBackImage=QPixmap::grabWindow(this->winId(),0,0,this->width(),this->height());
	if (ev->button()==Qt::RightButton)
	{
		emit RMousePressMenu();
	}
	if (ev->button()==Qt::RightButton)
	{
		emit RMousePressMenu();
	}
}

void RSubView::_cacheLableShow()
{
	if (_cacheLable==NULL)
	{
		_cacheLable=new QLabel(this);
		_cacheLable->setParent(this);
		QPalette pa;
		pa.setColor(QPalette::WindowText,Qt::red);
		_cacheLable->setPalette(pa);
		QFont ft;
		ft.setPointSize(20);
		_cacheLable->setFont(ft);
	}
	m_GroupPlayback->QueryInterface(IID_IDeviceClient,(void**)&m_LpClient);
	m_pRemotePlayBack = m_GroupPlayback;
	return;
}

void RSubView::OnRMousePressMenu()
{
	m_rMousePressMenu.exec(QCursor::pos());
}

void RSubView::OnOpenAudio()
{
	if (!m_bGlobalAudioStatus)
	{
		return;
	}
	if (!m_bLocalAudioStatus)
	{
		if (NULL == m_pRemotePlayBack)
		{
			return;
		}
		m_pRemotePlayBack->GroupSetVolume(0xAECBCA, this);
		m_pRemotePlayBack->GroupEnableAudio(true);

		m_ActionOpenAudio->setText("close audio");
		m_bLocalAudioStatus = true;
		m_pCurView = this;
	}
	else if (NULL != m_pCurView)
	{
		if (m_pCurView == this)
		{
			if (NULL == m_pRemotePlayBack)
			{
				return;
			}
			m_pRemotePlayBack->GroupEnableAudio(false);

			m_ActionOpenAudio->setText("open audio");
			m_bLocalAudioStatus = false;
			m_pCurView = NULL;
		}
		else
		{
			emit ChangeAudioHint(QString("open audio"), m_pCurView);
			m_pRemotePlayBack->GroupSetVolume(0xAECBCA, this);
			m_pCurView = this;
			m_ActionOpenAudio->setText("close audio");
		}
	}
}

void RSubView::setAudioHint(QString &statement)
{
	m_ActionOpenAudio->setText(statement);
}

bool RSubView::AudioEnabled(bool bEnabled)
{
	m_bGlobalAudioStatus = bEnabled;
	if (!bEnabled && NULL != m_pRemotePlayBack && m_bLocalAudioStatus)
	{
		emit ChangeAudioHint(QString("open audio"), m_pCurView);

		m_pRemotePlayBack->GroupEnableAudio(false);
		m_bLocalAudioStatus = false;
		m_pCurView = NULL;
	}
	
	return true;
}

void RSubView::SetCurConnectState( __enConnectStatus parm  )
{
	_curState=parm;
	emit connecttingUpdateSig();
}

void RSubView::CacheState( QVariantMap evMap )
{
	int *wind = reinterpret_cast<int*>(this);
	if (evMap.value("wind").toInt()==*wind)
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
	_curPaint= PAINTEVENT_STATUS_CONNECTING;
	update();
}

void RSubView::CacheStateSlot( QVariantMap evMap )
{
	_curPaint=PAINTEVENT_STATUS_CACHE;
	_curCache=evMap.value("Persent").toInt();
	update();
}

void RSubView::CacheStateSlotUpdate()
{
	_curPaint=PAINTEVENT_STATUS_CACHE;
	update();
}

void RSubView::paintEventNoVideo( QPaintEvent * )
{
	if (_curPaint==PAINTEVENT_STATUS_NOVIDEO)
	{
		//
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
}

void RSubView::paintEventConnecting( QPaintEvent * )
{
	if (_curPaint==PAINTEVENT_STATUS_CONNECTING)
	{
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
	if (_curPaint==PAINTEVENT_STATUS_CACHE)
	{
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

		QString cachePercent = QString("%1").arg(_curCache);
		cachePercent.append("%");
		p.drawText(rcClient, Qt::AlignCenter, cachePercent);
	}
}


