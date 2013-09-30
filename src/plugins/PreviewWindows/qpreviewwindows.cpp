#include "qpreviewwindows.h"
#include "qsubview.h"
#include <QtGui/QResizeEvent>
#include <qwfw_tools.h>
#include <QtCore/QtCore>
#include <QtXml/QtXml>

QPreviewWindows::QPreviewWindows(QWidget *parent)
	: QWidget(parent),
	QWebPluginFWBase(this),
	m_DivMode(NULL)
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

void QPreviewWindows::nextPage()
{
}

void QPreviewWindows::prePage()
{

}

int QPreviewWindows::getCurrentPage()
{
}

int QPreviewWindows::getPages()
{

}

int QPreviewWindows::setDivMode( QString divModeName )
{
	// configuration
	QString sAppPath = QCoreApplication::applicationDirPath();
	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConfFile;
	ConfFile.setContent(file);

	// 根据配置创建对应的divMode实例

	// 设置相关的参数


}