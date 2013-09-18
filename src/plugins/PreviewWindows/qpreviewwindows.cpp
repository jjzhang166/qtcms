#include "qpreviewwindows.h"
#include "qsubview.h"
#include <QtGui/QResizeEvent>

QPreviewWindows::QPreviewWindows(QWidget *parent)
	: QWidget(parent)
{
	for (int i = 0; i < 4; i ++ )
	{
		m_PreviewWnd[i] = new QSubView(this);
	}
}

QPreviewWindows::~QPreviewWindows()
{

}

void QPreviewWindows::resizeEvent( QResizeEvent * ev)
{
	QSize curSize = ev->size();
	QRect rcClient = contentsRect();
	rcClient.setSize(curSize);
	for (int i = 0; i < 4; i++)
	{
		m_PreviewWnd[i]->move((i % 2) * (rcClient.width() / 2),(i / 2) * (rcClient.height() / 2));
		m_PreviewWnd[i]->resize(rcClient.width() / 2,rcClient.height() / 2);
	}
}