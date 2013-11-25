#pragma once
#include <QtGui/QWidget>
#include <qwfw.h>
#include "H264Player.h"

class H264PlayerWindow :public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	H264PlayerWindow(QWidget* parent = 0);
	~H264PlayerWindow(void);

public slots:
	void Play();
	void Pause();
	void Stop();
	void Open();
	void Init();
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}
private:
	H264Player m_h264Player;

};