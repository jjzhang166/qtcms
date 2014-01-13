#include "RecordPlayerView.h"
#include <QtCore>
#include <QtGui/QPainter>
#include <QSettings>
#include <QMouseEvent>

RecordPlayerView::RecordPlayerView(QWidget *parent)
	: QWidget(parent)
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
	p.drawText(rcClient, Qt::AlignCenter, "Pixmap");
}

void RecordPlayerView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
}

void RecordPlayerView::mousePressEvent(QMouseEvent *ev)
{
	setFocus(Qt::MouseFocusReason);
	emit SetCurrentWindSignl(this);
}