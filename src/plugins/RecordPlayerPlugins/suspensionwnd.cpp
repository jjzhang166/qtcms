#include "suspensionwnd.h"
#include <QApplication>
#include <QDesktopWidget>

SuspensionWnd::SuspensionWnd(QWidget *parent)
	: QWidget(parent),
	m_posInRect(false)
{

}

SuspensionWnd::~SuspensionWnd()
{

}

void SuspensionWnd::mousePressEvent( QMouseEvent *ev )
{
	m_pressPoint = ev->pos();
	if (m_drawRect.contains(m_pressPoint)){
		m_posInRect = true;
		m_lastMovePos = m_pressPoint;
	}else{
		m_posInRect = false;
	}
}

void SuspensionWnd::mouseReleaseEvent( QMouseEvent *ev )
{
	QPoint releasePoint = ev->pos();
	if (m_pressPoint != releasePoint && !m_posInRect && m_cbFunc && m_puser){
		QVariantMap msg;
		msg.insert("EvName", QString("ZoomRect"));
		msg.insert("ZoRect", QRect(m_pressPoint, releasePoint));
		msg.insert("CurWnd", (quintptr)m_wndList.last());
		m_cbFunc(msg, m_puser);

		m_drawRect = QRect(m_pressPoint, releasePoint);
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
		emit sigClose();
	}
}

void SuspensionWnd::setCbFunc( callbackFc pfunc, void* puser )
{
	m_cbFunc = pfunc;
	m_puser = puser;
}

void SuspensionWnd::mouseMoveEvent( QMouseEvent *ev )
{
	if (m_posInRect && m_cbFunc && m_puser){
		m_drawRect.translate(ev->pos() - m_lastMovePos);
		QVariantMap msg;
		msg.insert("EvName", QString("ZoomRect"));
		msg.insert("ZoRect", m_drawRect);
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
		QRect curRect = this->geometry();
		QRect geom = QApplication::desktop()->availableGeometry();
		int width = geom.width();
		int height = geom.height();
		if (curRect.width() > width/10 && curRect.height() > height/40){
			geom.setX(width - width/10);
			geom.setY(height - height/40);
			this->setGeometry(geom);
			m_originRect = curRect;
		}else{
			this->setGeometry(m_originRect);
		}
	}
	return QWidget::event(ev);
}

void SuspensionWnd::setOriginGeog( QRect rect )
{
	m_originRect = rect;
}
