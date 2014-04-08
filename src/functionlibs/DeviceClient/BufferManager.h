#ifndef _BUFFERMANAGER_HEAD_FILE_H_
#define _BUFFERMANAGER_HEAD_FILE_H_

#include "deviceclient_global.h"
#include <QThread>
#include <QQueue>
#include <QMutex>

class BufferManager : public QThread
{
	Q_OBJECT
public:
	BufferManager(void);
	~BufferManager(void);

// 	int recordAudioStream(QVariantMap &evMap);
	int recordStream(QVariantMap &evMap);
	int readStream(RecordStreamFrame &streamInfo);
	void audioSwitch(bool mode);
	bool getAudioStatus();
	int getVedioBufferSize();
	int emptyBuff();
	void removeItem(RecordStreamFrame*);
signals:
	void action(QString option, BufferManager* pBuffer);
	void bufferStatus(int persent, BufferManager* pBuffer);
public slots:

private:
	QQueue<RecordStreamFrame> m_StreamBuffer;
// 	QQueue<RecordAudioStream> m_audioStreamBuffer;
	bool m_bVedioBufferIsFull;
	bool m_bStopAudio;
	bool m_bStopBuff;

	QMutex m_mutex;
};

#endif
