#ifndef __QWFW_HEAD_FILESA8D90123__
#define __QWFW_HEAD_FILESA8D90123__
#include <QtWebKit/QWebFrame>

class QWebUiFWBase
{
public:
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