#pragma once
#include <QWidget>
#include "qwfw.h"
#include <IConfigManager.h>
class userInfoObject: public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	userInfoObject(QWidget *parent = 0);
	~userInfoObject();
public slots:
	void importUserInfo();
	void outportUserInfo();
private:
	void saveToDefaultPath();
	bool getOutputUserInfoFilePath(QString &sFilePath);
private:
	IConfigManager *m_pConfigManager;
};

