#pragma once
#include <QtCore/QThread>
#include "IVideoDecoder.h"
#include "IVideoRender.h"

class H264Player :
	public QThread
{
	Q_OBJECT
public:
	H264Player(void);
	~H264Player(void);

	int loadRender( QString renderName );
	int loadDecoder( QString decoderName );
	bool InitSearver(QWidget* pwnd);
	bool OpenFile(QString filename);
	void Play();
	void Pause();
	void Stop();
protected:
	void run();

private:
	IVideoDecoder* m_pDecoder;
	IVideoRender* m_pRender;

	FILE* m_pfile;
	bool playing;
};

