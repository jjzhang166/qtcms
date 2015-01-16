#include "suspensionwnd.h"

SuspensionWnd::SuspensionWnd(QWidget *parent)
	: QWidget(parent)
{

}

SuspensionWnd::~SuspensionWnd()
{

}

void SuspensionWnd::mousePressEvent( QMouseEvent *ev )
{
	m_pressPoint = ev->pos();
}

void SuspensionWnd::mouseReleaseEvent( QMouseEvent *ev )
{
	if (m_cbFunc && m_puser){
		QVariantMap msg;
		msg.insert("EvName", QString("ZoomRect"));
		msg.insert("ZoRect", QRect(m_pressPoint, ev->pos()));
		msg.insert("CurWnd", (quintptr)m_wndList.last());
		m_cbFunc(msg, m_puser);
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

void SuspensionWnd::enterEvent(QEvent *ev)
{
	if (QEvent::Close == ev->type()){
		QWidget* pwnd = m_wndList.takeLast();
		if (m_cbFunc && m_puser){
			QVariantMap msg;
			msg.insert("EvName", QString("CloseWnd"));
			msg.insert("CurWnd", (quintptr)pwnd);
			m_cbFunc(msg, m_puser);
		}
		if (m_wndList.isEmpty()){
			this->close();
		}
	}
}

void SuspensionWnd::setCbFunc( callbackFc pfunc, void* puser )
{
	m_cbFunc = pfunc;
	m_puser = puser;
}
