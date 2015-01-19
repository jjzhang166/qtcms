#include "RecordPlayerView.h"
#include <QtCore>
#include <QtGui/QPainter>
#include <QSettings>
#include <QMouseEvent>
#include "guid.h"
#include "IVideoDisplayOption.h"
#include "ICommunicate.h"
#include <QApplication>
#include <QDomDocument>

bool RecordPlayerView::m_bGlobalAudioStatus = false;
SuspensionWnd* RecordPlayerView::ms_susWnd = NULL;
int RecordPlayerView::ms_playStatus = 4;//stop local play

RecordPlayerView::RecordPlayerView(QWidget *parent)
	: QWidget(parent),
	m_pLocalPlayer(NULL),
	_bIsFocus(false),
	m_bPlaying(false)
{
	this->lower();
	this->setAttribute(Qt::WA_PaintOutsidePaintEvent);
	m_pWindowsStretchAction = m_WindowMenu.addAction(tr("Suit For Window"));
	m_pWindowsStretchAction->setCheckable(true);
	m_pWindowsStretchAction->setChecked(false);

	connect(m_pWindowsStretchAction,SIGNAL(triggered(bool)),this,SLOT(slSuitForWindow(bool)));
}


RecordPlayerView::~RecordPlayerView(void)
{
}


void RecordPlayerView::paintEvent( QPaintEvent * e)
{
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
	QString sBackground;

	image = IniFile.value("background/background-image", NULL).toString();
//	LineColor.setNamedColor(IniFile.value("background/background-color", NULL).toString());
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
		p.drawRect(QRect(x,y ,width , height));
	}
	else
	{
		p.drawRect(rcClient);
	}

	//QFont font(FontFamily, FontSize, QFont::Bold);
	QFont font(FontFamily, FontSize, QFont::Bold|QFont::System);
	p.setFont(font);

	pen.setColor(FontColor);

	p.setPen(pen);

	p.drawText(rcClient, Qt::AlignCenter, sBackground);
}

void RecordPlayerView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
}

void RecordPlayerView::mousePressEvent(QMouseEvent *ev)
{
	m_pressPoint = ev->pos();

	setFocus(Qt::MouseFocusReason);
	emit SetCurrentWindSignl(this);
	if (m_bGlobalAudioStatus && NULL != m_pLocalPlayer && ev->button() == Qt::LeftButton)
	{
		m_pLocalPlayer->GroupSetVolume(0xAECBCA, this);
	}
	//if (Qt::RightButton == ev->button())
	//{
	//	if (NULL != m_pWindowsStretchAction)
	//	{
	//		m_pWindowsStretchAction->setText(tr("Suit For Window"));
	//	}
	//	m_WindowMenu.exec(QCursor::pos());
	//}
}

void RecordPlayerView::setLocalPlayer(ILocalPlayerEx* pPlayer)
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

void RecordPlayerView::slSuitForWindow( bool checked )
{
	if (NULL != m_pLocalPlayer)
	{
		IVideoDisplayOption * pi;
		m_pLocalPlayer->QueryInterface(IID_IVideoDisplayOption,(void **)&pi);
		if (NULL != pi)
		{
			pi->enableWindowStretch(this,!checked);
			pi->Release();
		}
	}
	m_pWindowsStretchAction->setChecked(checked);
}

void RecordPlayerView::changeEvent( QEvent * )
{
	if (NULL != m_pWindowsStretchAction)
	{
		m_pWindowsStretchAction->setText(tr("Suit For Window"));
	}
}

void RecordPlayerView::mouseReleaseEvent( QMouseEvent *ev )
{
	QRect rect = this->rect();
	QPoint releasePoint = ev->pos();
	//if release point in current window
	if (m_pressPoint != releasePoint && rect.contains(m_pressPoint) && rect.contains(releasePoint) && ms_playStatus < 4 && m_bPlaying){
		//if no suspension window, create it
		if (!ms_susWnd){
			ms_susWnd = new SuspensionWnd(this);
			ms_susWnd->setWindowFlags(Qt::Tool | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
			ms_susWnd->setWindowTitle(QString("Digital Zoom"));
			ms_susWnd->setCbFunc(cbReciveMsg, this);
			ms_susWnd->show();
		}
		//notify play module current window need to zoom
		if (m_pLocalPlayer){
			ICommunicate *pCom = NULL;
			m_pLocalPlayer->QueryInterface(IID_ICommunicate, (void**)&pCom);
			if (pCom){
				QVariantMap msg;
				msg.insert("SusWnd", (quintptr)ms_susWnd);
				msg.insert("CurWnd", (quintptr)this);
				ms_susWnd->addWnd(this);
				pCom->setInfromation(QString("VedioZoom"), msg);
				pCom->Release();
			}
		}
	}
}

void RecordPlayerView::recMsg( QVariantMap msg )
{
	QString evName = msg["EvName"].toString();
	ICommunicate *pCom = NULL;
	m_pLocalPlayer->QueryInterface(IID_ICommunicate, (void**)&pCom);
	if (pCom){
		msg.remove("EvName");
		pCom->setInfromation(evName, msg);
	}
	if ("CloseWnd" == evName){
		if (!msg["ListSize"].toInt()){
			ms_susWnd->close();
			delete ms_susWnd;
			ms_susWnd = NULL;
		}
	}
}

void RecordPlayerView::setPlayStatus( int status )
{
	ms_playStatus = status;
}

void RecordPlayerView::setPlayingFlag( bool bPlaying )
{
	qDebug()<<(int)this<<"origin flag:"<<m_bPlaying<<" set flag:"<<bPlaying;
	m_bPlaying = bPlaying;
}

void cbReciveMsg( QVariantMap evMap, void* pUser )
{
	((RecordPlayerView*)pUser)->recMsg(evMap);
}
