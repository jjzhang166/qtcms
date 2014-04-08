#include "RecordPlayerView.h"
#include <QtCore>
#include <QtGui/QPainter>
#include <QSettings>
#include <QMouseEvent>


bool RecordPlayerView::m_bGlobalAudioStatus = false;


RecordPlayerView::RecordPlayerView(QWidget *parent)
	: QWidget(parent),
	m_pLocalPlayer(NULL),
	_bIsFocus(false)
{
	this->lower();
	this->setAttribute(Qt::WA_PaintOutsidePaintEvent);

}


RecordPlayerView::~RecordPlayerView(void)
{
}


void RecordPlayerView::paintEvent( QPaintEvent * e)
{
	QPainter p(this);

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
	LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
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
		p.drawRect(QRect(x,y ,width , height));
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
	if (m_bGlobalAudioStatus && NULL != m_pLocalPlayer && ev->button() == Qt::LeftButton)
	{
		m_pLocalPlayer->GroupSetVolume(0xAECBCA, this);
	}
}

void RecordPlayerView::setLocalPlayer(ILocalPlayer* pPlayer)
{
	if (NULL != pPlayer)
	{
		m_pLocalPlayer = pPlayer;
	}
}

int RecordPlayerView::AudioEnabled(bool bEnabled)
{
	if (NULL == m_pLocalPlayer)
	{
		return 1;
	}
	m_pLocalPlayer->GroupSetVolume(0xAECBCB,this);
	m_pLocalPlayer->GroupEnableAudio(bEnabled);
	m_bGlobalAudioStatus = bEnabled;

	return 0;
}

QVariantMap RecordPlayerView::ScreenShot()
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

void RecordPlayerView::SetFocus( bool flags )
{
	bool history=_bIsFocus;
	_bIsFocus=flags;
	if (_bIsFocus==true||history==true)
	{
		update();
	}
}
