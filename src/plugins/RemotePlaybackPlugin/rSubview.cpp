#include <QtCore>
#include <QtGui/QPainter>
#include <QSettings>
#include <QMouseEvent>
#include <guid.h>
#include "rSubview.h"

RSubView* RSubView::m_pCurView = NULL;
bool RSubView::m_bIsAudioOpen = false;


RSubView::RSubView(QWidget *parent)
	: QWidget(parent),
	m_LpClient(NULL),
	m_pRemotePlayBack(NULL),
	ui(new Ui::titleview)
{
	this->lower();
	this->setAttribute(Qt::WA_PaintOutsidePaintEvent);
	//ui->setupUi(this);
	//QVBoxLayout *layout = new QVBoxLayout;
	//layout->addWidget(ui->gridLayoutWidget);
	//setLayout(layout);

	m_ActionOpenAudio = m_rMousePressMenu.addAction("open audio");
	connect(this,SIGNAL(RMousePressMenu()),this,SLOT(OnRMousePressMenu()));
	connect(m_ActionOpenAudio, SIGNAL(triggered(bool)), this, SLOT(OnOpenAudio()));

}

RSubView::~RSubView()
{
	delete ui;
	if (NULL!=m_LpClient)
	{
		m_LpClient->Release();
	}
}

void RSubView::paintEvent( QPaintEvent * e)
{
	//if (NULL!=m_LpClient)
	//{
	//	if (IDeviceClient::STATUS_CONNECTED==m_LpClient->getConnectStatus())
	//	{
	//		return;
	//	}
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

void RSubView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
}

void RSubView::mousePressEvent(QMouseEvent *ev)
{
	setFocus(Qt::MouseFocusReason);
	emit SetCurrentWindSignl(this);
	if (ev->button()==Qt::RightButton)
	{
		emit RMousePressMenu();
	}
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

void RSubView::OnRMousePressMenu()
{
	m_rMousePressMenu.exec(QCursor::pos());
}

void RSubView::OnOpenAudio()
{
	if (!m_bIsAudioOpen)
	{
		if (NULL == m_pRemotePlayBack)
		{
			return;
		}
		m_pRemotePlayBack->GroupSetVolume(0xAECBCA, this);
		m_pRemotePlayBack->GroupEnableAudio(true);

		m_ActionOpenAudio->setText("close audio");
		m_bIsAudioOpen = true;
		m_pCurView = this;
	}
	else
	{
		if (m_pCurView == this)
		{
			if (NULL == m_pRemotePlayBack)
			{
				return;
			}
			m_pRemotePlayBack->GroupEnableAudio(false);

			m_ActionOpenAudio->setText("open audio");
			m_bIsAudioOpen = false;
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
