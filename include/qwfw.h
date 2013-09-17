#ifndef __QWFW_HEAD_FILESA8D90123__
#define __QWFW_HEAD_FILESA8D90123__
#include <QtWebKit/QWebFrame>
#include <QtGui/QWidget>
#include <QtWebKit/QWebView>
#include <QtCore/QString>
#include <QtWebKit/QWebElement>

class QWebPluginFWBase
{
public:
	QWebPluginFWBase(QWidget * widget){m_widget = widget;};

protected:
	void EventProcCall(QString sEvent){
		QList<QString> sProcs = m_mapEventProc.values(sEvent);
		int n = sProcs.count();
		for (int i = n ; i > 0 ;i --)
		{
			QString sItem = sProcs.at(i - 1);
			QWidget *pa = m_widget->parentWidget();
			if ("QtWebKitFW" == pa->objectName())
			{
				((QWebView *)pa)->page()->mainFrame()->evaluateJavaScript(sItem);
			}
		}
	};

	QMap<QString,QString> m_mapEventProc;
	QWidget * m_widget;
};


class QWebUiFWBase
{
public:
	QString QueryValue(QString sElementId){
		QWebElement elementTemp = m_MainFrame->findFirstElement(QString("#") + sElementId);
		return elementTemp.attribute("value");
	}
protected:
	QWebFrame * m_MainFrame;
private:
};

#define QWFW_MSGMAP_BEGIN(x)  \
{ \
	m_MainFrame = x; \
	m_MainFrame->addToJavaScriptWindowObject(QString("qob"),this); \
	connect(m_MainFrame,SIGNAL(javaScriptWindowObjectCleared),this,SLOT(OnJavaScriptWindowObjectCleared)); \
}

#define QWFW_MSGMAP(id,msg,proc) { \
	QString sReq = QString("connectEvent('") + QString(id) + QString("','") + QString(msg) + QString("',function a(){qob.") + QString(proc) +QString(";});"); \
	m_MainFrame->evaluateJavaScript(sReq); \
} 

#define QWFW_MSGMAP_END

#define QWFW_MSGRESET	{m_MainFrame->addToJavaScriptWindowObject(QString("qob"),this);}


#endif