#pragma once
#include <QThread>
#include <QObject>
#include <QVariantMap>
#include <QWidget>
#include <QDebug>
class PreviewPlay:public QThread
{
	Q_OBJECT
		QThread sMyPreviewPlay;
public:
	PreviewPlay(QThread *parent=0);
	~PreviewPlay(void);
public slots:
	void MyThreadPreviewPlay();
};

