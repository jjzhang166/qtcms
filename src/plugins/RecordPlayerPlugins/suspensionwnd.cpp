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
	QPoint releasePoint = ev->pos();
	if (m_pressPoint != releasePoint && m_cbFunc && m_puser){
		QVariantMap msg;
		msg.insert("EvName", QString("ZoomRect"));
		msg.insert("ZoRect", QRect(m_pressPoint, releasePoint));
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

void SuspensionWnd::closeEvent(QCloseEvent *ev)
{
	if (QEvent::Close == ev->type()){
		QWidget* pwnd = m_wndList.takeLast();
		if (m_cbFunc && m_puser){
			QVariantMap msg;
			msg.insert("EvName", QString("CloseWnd"));
			msg.insert("CurWnd", (quintptr)pwnd);
			msg.insert("ListSize", m_wndList.size());
			m_cbFunc(msg, m_puser);
		}
		ev->ignore();
	}
}

void SuspensionWnd::setCbFunc( callbackFc pfunc, void* puser )
{
	m_cbFunc = pfunc;
	m_puser = puser;
}
