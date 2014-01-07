#ifndef _BUFFERMANAGER_HEAD_FILE_H_
#define _BUFFERMANAGER_HEAD_FILE_H_

#include "deviceclient_global.h"
#include <QThread>
#include <QQueue>


class BufferManager : public QThread
{
	Q_OBJECT
public:
	BufferManager(void);
	~BufferManager(void);

	int recordAudioStream(QVariantMap &evMap);
	int recordVedioStream(QVariantMap &evMap);
	int readVedioStream(RecordVedioStream &streamInfo);
	void audioSwitch(bool mode);
	bool getAudioStatus();
	int getVedioBufferSize();
	int emptyBuff();
signals:
	void action(QString option, BufferManager* pBuffer);
public slots:

private:
	QQueue<RecordVedioStream> m_vedioStreamBuffer;
	QQueue<RecordAudioStream> m_audioStreamBuffer;
	bool m_bVedioBufferIsFull;
	bool m_bStopAudio;
};

#endif
