#include "RecordPlayerView.h"
#include <QtCore>
#include <QtGui/QPainter>
#include <QSettings>
#include <QMouseEvent>


RecordPlayerView* RecordPlayerView::m_pCurView = NULL;
bool RecordPlayerView::m_bLocalAudioStatus = false;
bool RecordPlayerView::m_bGlobalAudioStatus = false;


RecordPlayerView::RecordPlayerView(QWidget *parent)
	: QWidget(parent),
	m_pLocalPlayer(NULL)
{
	this->lower();
	this->setAttribute(Qt::WA_PaintOutsidePaintEvent);

	m_ActionOpenAudio = m_rMousePressMenu.addAction("open audio");
	connect(this,SIGNAL(RMousePressMenu()),this,SLOT(OnRMousePressMenu()));
	connect(m_ActionOpenAudio, SIGNAL(triggered(bool)), this, SLOT(OnOpenAudio()));
}


RecordPlayerView::~RecordPlayerView(void)
{
}


void RecordPlayerView::paintEvent( QPaintEvent * e)
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
	p.drawText(rcClient, Qt::AlignCenter, "No Video");
}

void RecordPlayerView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
}

void RecordPlayerView::mousePressEvent(QMouseEvent *ev)
{
	setFocus(Qt::MouseFocusReason);
	emit SetCurrentWindSignl(this);
	if (ev->button()==Qt::RightButton)
	{
		emit RMousePressMenu();
	}
}

void RecordPlayerView::OnRMousePressMenu()
{
	m_rMousePressMenu.exec(QCursor::pos());
}

void RecordPlayerView::setLocalPlayer(ILocalPlayer* pPlayer)
{
	if (NULL != pPlayer)
	{
		m_pLocalPlayer = pPlayer;
	}
}

void RecordPlayerView::OnOpenAudio()
{
	if (!m_bGlobalAudioStatus)
	{
		return;
	}
	if (!m_bLocalAudioStatus)
	{
		if (NULL == m_pLocalPlayer)
		{
			return;
		}
		m_pLocalPlayer->GroupSetVolume(0xAECBCA, this);
		m_pLocalPlayer->GroupEnableAudio(true);

		m_ActionOpenAudio->setText("close audio");
		m_bLocalAudioStatus = true;
		m_pCurView = this;
	}
	else if (NULL != m_pCurView)
	{
		if (m_pCurView == this)
		{
			if (NULL == m_pLocalPlayer)
			{
				return;
			}
			m_pLocalPlayer->GroupEnableAudio(false);

			m_ActionOpenAudio->setText("open audio");
			m_bLocalAudioStatus = false;
			m_pCurView = NULL;
		}
		else
		{
			emit ChangeAudioHint(QString("open audio"), m_pCurView);

			m_pLocalPlayer->GroupSetVolume(0xAECBCA, this);
			m_pCurView = this;
			m_ActionOpenAudio->setText("close audio");
		}
	}
}

void RecordPlayerView::setAudioHint(QString &statement)
{
	m_ActionOpenAudio->setText(statement);
}

int RecordPlayerView::AudioEnabled(bool bEnabled)
{
	m_bGlobalAudioStatus = bEnabled;
	if (!bEnabled && NULL != m_pLocalPlayer && m_bLocalAudioStatus)
	{
		emit ChangeAudioHint(QString("open audio"), m_pCurView);

		m_pLocalPlayer->GroupEnableAudio(false);
		m_bLocalAudioStatus = false;
		m_pCurView = NULL;
	}
	return 0;
}
