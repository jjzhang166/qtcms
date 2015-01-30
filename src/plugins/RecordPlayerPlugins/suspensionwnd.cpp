#include "suspensionwnd.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QIcon>
#include <QPainter>

SuspensionWnd::SuspensionWnd(QWidget *parent)
	: QWidget(parent),
	m_posInRect(false),
	m_bMinimized(false)
{
	QString image;
	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);
	image = IniFile.value("background/zoom-icon-image", QVariant("")).toString();
	QString PixPath = sAppPath + image;
	QIcon tWindowIcon(PixPath);
	this->setWindowIcon(tWindowIcon);
	QRect deskRect = QApplication::desktop()->geometry();
	this->resize(deskRect.width()/3, deskRect.height()/3);
}

SuspensionWnd::~SuspensionWnd()
{

}

void SuspensionWnd::mousePressEvent( QMouseEvent *ev )
{
	if (Qt::LeftButton == ev->button()){
		m_pressPoint = ev->pos();
		if (m_drawRect.contains(m_pressPoint)){
			m_posInRect = true;
			m_lastMovePos = m_pressPoint;
		}else{
			m_posInRect = false;
		}
	}
}

void SuspensionWnd::mouseReleaseEvent( QMouseEvent *ev )
{
	if (Qt::LeftButton == ev->button()){
		QPoint releasePoint = ev->pos();
		if (m_pressPoint != releasePoint && !m_posInRect && m_cbFunc && m_puser){
			QVariantMap msg;
			msg.insert("EvName", QString("ZoomRect"));
			QRect rect(m_pressPoint, releasePoint);
			if (rect.width()*rect.height()/1000){
				msg.insert("ZoRect", rect);
			}else{
				msg.insert("ZoRect", QRect(1, 1, 1, 1));
			}
			msg.insert("Width", this->width());
			msg.insert("Height", this->height());
			msg.insert("CurWnd", (quintptr)m_wndList.last());
			m_cbFunc(msg, m_puser);

			m_drawRect = QRect(m_pressPoint, releasePoint);
		}
	}
}

void SuspensionWnd::addWnd( QWidget* pWnd )
{
	if (!m_wndList.contains(pWnd)){
		m_wndList.append(pWnd);
	}else{
		m_wndList.removeOne(pWnd);
		m_wndList.append(pWnd);
	}
}

void SuspensionWnd::closeEvent(QCloseEvent *ev)
{
	if (QEvent::Close == ev->type()){
		m_wndList.clear();
		m_bMinimized = false;
		emit sigClose();
		ev->ignore();
	}
}

void SuspensionWnd::setCbFunc( callbackFc pfunc, void* puser )
{
	m_cbFunc = pfunc;
	m_puser = puser;
}

void SuspensionWnd::mouseMoveEvent( QMouseEvent *ev )
{
	if (m_cbFunc && m_puser){
		m_drawRect.translate(ev->pos() - m_lastMovePos);
		QVariantMap msg;
		msg.insert("EvName", QString("ZoomRect"));
		if (m_posInRect){
			msg.insert("ZoRect", m_drawRect);
		}else{
			msg.insert("ZoRect", QRect(m_pressPoint, ev->pos()));
		}
		msg.insert("Width", this->width());
		msg.insert("Height", this->height());
		msg.insert("CurWnd", (quintptr)m_wndList.last());
		m_cbFunc(msg, m_puser);
		m_lastMovePos = ev->pos();
	}
}

void SuspensionWnd::mouseDoubleClickEvent( QMouseEvent * )
{
	if (m_cbFunc && m_puser){
		QVariantMap msg;
		msg.insert("EvName", QString("ZoomRect"));
		msg.insert("ZoRect", QRect(m_drawRect.topLeft(), m_drawRect.topLeft()));
		msg.insert("Width", 0);
		msg.insert("Height", 0);
		msg.insert("CurWnd", (quintptr)m_wndList.last());
		m_cbFunc(msg, m_puser);
		m_drawRect.setCoords(0, 0, 0, 0);
	}
}

void SuspensionWnd::setDrawRect( QRect rect )
{
	m_drawRect = rect;
}

QWidget* SuspensionWnd::getTopWnd()
{
	if (m_wndList.isEmpty()){
		return NULL;
	}else{
		return m_wndList.last();
	}
}

bool SuspensionWnd::event( QEvent *ev )
{
	if (ev->type() == QEvent::NonClientAreaMouseButtonDblClick){
		if (!m_bMinimized){
			QRect geom = QApplication::desktop()->availableGeometry();
			int width = geom.width();
			int height = geom.height();
			geom.setX(width - width/10);
			geom.setY(height - height/40);
			m_originRect = this->geometry();
			setGeometry(geom);
			m_bMinimized = true;
		}else{
			setGeometry(m_originRect);
			m_bMinimized = false;
		}
	}

	return QWidget::event(ev);
}

void SuspensionWnd::setOriginGeog( QRect rect )
{
	m_originRect = rect;
}

void SuspensionWnd::paintEvent( QPaintEvent *ev )
{
	Q_UNUSED(ev);
	QPainter p(this);
	QString image;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);

	image = IniFile.value("background/zoom-background-image", QVariant("")).toString();

	QRect rcClient = contentsRect();
	this->geometry().center();
	QPixmap pix;
	QString PixPaht = sAppPath + image;
	pix.load(PixPaht);
	pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
	//±³¾°
	p.drawPixmap(rcClient,pix);
}
void SuspensionWnd::changeEvent( QEvent *ev )
{
	if (ev->type()==QEvent::LanguageChange)
	{
		//do something
		this->setWindowTitle(tr("Zoom"));
	}
}