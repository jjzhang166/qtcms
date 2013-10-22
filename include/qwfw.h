#ifndef __QWFW_HEAD_FILESA8D90123__
#define __QWFW_HEAD_FILESA8D90123__
#include <QtWebKit/QWebFrame>
#include <QtGui/QWidget>
#include <QtWebKit/QWebView>
#include <QtCore/QString>
#include <QtWebKit/QWebElement>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>

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

#define QWFW_MSGMAP_BEGIN(x)  \
{ \
	m_MainFrame = x; \
	m_MainFrame->addToJavaScriptWindowObject(QString("qob"),this); \
	qDebug("Insert qob"); \
	connect(m_MainFrame,SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(OnJavaScriptWindowObjectCleared())); \
}

#define QWFW_MSGMAP(id,msg,proc) { \
	QString sReq = QString("connectEvent('") + QString(id) + QString("','") + QString(msg) + QString("',function a(){qob.") + QString(proc) +QString(";});"); \
	m_MainFrame->evaluateJavaScript(sReq); \
} 

#define QWFW_MSGMAP_END

#define QWFW_MSGRESET	{m_MainFrame->addToJavaScriptWindowObject(QString("qob"),this);}

class QWebUiFWBase : public QObject
	{
	Q_OBJECT
public:
	QVariant QueryValue(QString sElementId){
		QWebElement elementTemp = m_MainFrame->findFirstElement(QString("#") + sElementId);
		return elementTemp.evaluateJavaScript("document.getElementById('" + sElementId + "').value");
	}
protected:
	QWebFrame * m_MainFrame;
	QMap<QString,QString> m_mapEventProc;
private:
public slots:
	void OnJavaScriptWindowObjectCleared(){QWFW_MSGRESET;};
	void AttachEvent(QString sEvent,QString sProc){
		m_mapEventProc.insertMulti(sEvent,sProc);
		qDebug("Call AttachEvent %s %s",sEvent.toAscii().data(),sProc.toAscii().data() );
	};
protected:
	void EventProcCall(QString sEvent,QVariantMap eventParam){
		QList<QString> sProcs = m_mapEventProc.values(sEvent);
		int n = sProcs.count();
		for (int i = n ; i > 0 ;i --)
		{
			QString sItem = sProcs.at(i - 1);
			QString sScripte;
			sScripte += "{var e={";
			QVariantMap::const_iterator itParameters;
			for (itParameters = eventParam.begin();itParameters != eventParam.end(); itParameters ++)
			{
				QString sKey = itParameters.key();
				QString sValue = itParameters.value().toString();
				sScripte += sKey;
				sScripte += ":'";
				sScripte += sValue;
				sScripte += "'";
				if (itParameters + 1 != eventParam.end())
				{
					sScripte += ",";
				}
			}
			sScripte += "};";
			sScripte += sItem.replace(QRegExp("\\((.*)\\)"),"(e)");
			sScripte += ";}";
			m_MainFrame->evaluateJavaScript(sScripte);
		}
	};
};

#define DEF_EVENT_PARAM(v) QVariantMap v;

#define EP_ADD_PARAM(v,name,value) v.insert(name,value);



#endif