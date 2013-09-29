#include "qpreviewwindows.h"
#include "qsubview.h"
#include <QtGui/QResizeEvent>
#include <qwfw_tools.h>

QPreviewWindows::QPreviewWindows(QWidget *parent)
	: QWidget(parent),
	QWebPluginFWBase(this)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(m_PreviewWnd); i ++)
	{
		m_PreviewWnd[i].setParent(this);
	}
}

QPreviewWindows::~QPreviewWindows()
{

}

void QPreviewWindows::resizeEvent( QResizeEvent * ev)
{
}