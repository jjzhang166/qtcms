#pragma once
#include <QWidget>
#include <QDebug>
#include <QObject>
#include "widgetforvideo.h"
#include "Qqwidget.h"
#include <QLabel>

class ManageWidget:public QWidget
{
	Q_OBJECT
public:
	ManageWidget(QWidget *parent);
	~ManageWidget();


	QWidget *GetWidgetForVideo();
	QWidget *GetRecordItem();

public :
	virtual void resizeEvent(QResizeEvent *event);
	virtual void timerEvent( QTimerEvent * );
public slots:
	void RecordState(bool flag);
private:

	widgetforvideo *_widgetForVideo;
	Qqwidget *_recordItem;
private:
	void __createWidgetForvideo();
	void __createRecordItem();
};

