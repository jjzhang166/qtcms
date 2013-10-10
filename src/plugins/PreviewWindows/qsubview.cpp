#include "qsubview.h"
#include <QtGui/QPainter>

QSubView::QSubView(QWidget *parent)
	: QWidget(parent)
{

}

QSubView::~QSubView()
{

}

void QSubView::paintEvent( QPaintEvent * e)
{
	QPainter p(this);
	QRect rcClient = contentsRect();
	p.fillRect(rcClient,QColor(0,0,0));
	p.setPen(QColor(255,0,0));
	p.drawRect(rcClient);
}

void QSubView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
}