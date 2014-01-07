#ifndef RSUBVIEW_H
#define RSUBVIEW_H

#include <QWidget>
#include <QString>
#include <QVariantMap>
#include <IDeviceClient.h>
#include <IEventRegister.h>
#include <IVideoDecoder.h>
#include <QLineEdit>
#include "ui_TitleView.h"
#include <IDeviceClient.h>
#include <IDeviceConnection.h>
#include <IDeviceRemotePlayback.h>


class RSubView :public QWidget
{
	Q_OBJECT

public:
	RSubView(QWidget *parent = 0);
	~RSubView();

	virtual void paintEvent( QPaintEvent * );
	virtual void mouseDoubleClickEvent( QMouseEvent * );
	virtual void mousePressEvent(QMouseEvent *);
public:
	void SetLpClient(IDeviceGroupRemotePlayback *m_GroupPlayback);

signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);
public:    
	Ui::titleview * ui;
	IDeviceClient *m_LpClient;
};


#endif // RSUBVIEW_H
