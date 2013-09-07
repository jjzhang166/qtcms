#pragma once
#include <QtGui/QGroupBox>
#include <QtWebKit/QWebElement>
#include <qwfw.h>
#include "FfmpegH264Dec.h"
#include "IVideoRender.h"
typedef unsigned long  DWORD;
typedef struct test_nalu_header{
	DWORD flag;
	DWORD size;
	DWORD isider;  //1 ÊÇIÖ¡
}NALU_HEADER_t;

class qffmpeg : public QGroupBox,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	qffmpeg(QWidget * parent = 0);
	~qffmpeg(void);

public slots:
	void Play();
	void Pause();
	void Stop();
	void Open();
	void Init();
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}
public:
	QString filepath;
	CFfmpegH264Dec m_dec;
	IVideoRender * m_CurRender;
};

