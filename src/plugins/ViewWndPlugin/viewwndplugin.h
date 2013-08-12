#ifndef VIEWWNDPLUGIN_H
#define VIEWWNDPLUGIN_H

#include "viewwndplugin_global.h"
#include <QObject>
#include <QtPlugin>
#include <QList>
#include <QtWebKit/QWebPluginFactory>
#include <QUrl>
#include <QtGui/QTextEdit>

class VIEWWNDPLUGIN_EXPORT ViewWndPlugin : public QObject
{
	Q_OBJECT
public:
	ViewWndPlugin();
	~ViewWndPlugin();

public:
	QList<QWebPluginFactory::Plugin> plugins()const;
	QObject * create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues);
	

private:

};

#endif // VIEWWNDPLUGIN_H
