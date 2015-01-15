#ifndef _BUFFERMANAGER_HEAD_FILE_H_
#define _BUFFERMANAGER_HEAD_FILE_H_

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QVariantMap>


typedef struct _tagRecordStreamFrame{
	uint uiLength;
	char cFrameType;
	char cChannel;
	union{
		uint uiAudioSampleRate;
		uint uiWidth;
	};
	union{
		char cAudioFormat[8];
		uint uiHeight;
	};
	union{
		uint uiAudioDataWidth;
		uint uiFrameRate;
	};
	quint64 ui64TSP;
	uint uiGenTime;
	char* pData;
}RecordStreamFrame;

class BufferManager : public QThread
{
	Q_OBJECT
public:
	BufferManager(void);
	~BufferManager(void);

	int recordStream(QVariantMap &evMap);
	int readStream(RecordStreamFrame &streamInfo);
	int getVedioBufferSize();
	int emptyBuff();
	void removeItem(RecordStreamFrame*);
signals:
	void action(QString option, BufferManager* pBuffer);
	void bufferStatus(int persent/*, BufferManager* pBuffer*/);
public slots:

private:
	QQueue<RecordStreamFrame> m_StreamBuffer;
	bool m_bVedioBufferIsFull;
	bool m_bStopBuff;

	QMutex m_mutex;
};

#endif
