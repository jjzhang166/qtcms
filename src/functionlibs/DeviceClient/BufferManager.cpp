#include "BufferManager.h"
#include "GlobalSettings.h"
#include <QDebug>

BufferManager::BufferManager(void):
m_bVedioBufferIsFull(false),
m_bStopAudio(false)
{
}


BufferManager::~BufferManager(void)
{
	if (!m_vedioStreamBuffer.isEmpty())
	{
		m_vedioStreamBuffer.clear();
	}
}

int cbRecordStream(QString evName,QVariantMap evMap,void*pUser)
{
	int nRet = 0;
 	QMap<int, WndPlay> *plstSubClient = (QMap<int, WndPlay>*)pUser;

	QMap<int, WndPlay>::iterator iter;
	for (iter = plstSubClient->begin(); iter != plstSubClient->end(); iter++)
	{
		if (iter.key() - 1 == evMap.value("channel"))
		{
			BufferManager *pBuffer = iter->bufferManager;
			if (NULL == pBuffer)
			{
				return 1;
			}
			if (0 == evMap.value("frametype").toInt()  && pBuffer->getAudioStatus())
			{
				nRet = pBuffer->recordAudioStream(evMap);
			}
			else
			{
				nRet = pBuffer->recordVedioStream(evMap);
			}
		}
	}

	return nRet;
}

int BufferManager::recordAudioStream(QVariantMap &evMap)
{
	//´ý¶¨
	return 0;
}

int BufferManager::recordVedioStream(QVariantMap &evMap)
{
	if (evMap.isEmpty())
	{
		return 1;
	}

	if (100 == m_vedioStreamBuffer.size())
	{
		emit action(QString("StartPlay"), this);
	}

	if (1000 <= m_vedioStreamBuffer.size() && !m_bVedioBufferIsFull)
	{
		m_bVedioBufferIsFull = true;
		emit action(QString("Pause"), this);
	}

	RecordVedioStream recVeStream;
	recVeStream.uiLength = evMap.value("length").toUInt();
	recVeStream.cFrameType = evMap.value("frametype").toUInt();
	recVeStream.cChannel = evMap.value("channel").toUInt();
	recVeStream.uiWidth = evMap.value("width").toInt();
	recVeStream.uiHeight = evMap.value("height").toUInt();
	recVeStream.uiFrameRate = evMap.value("framerate").toUInt();
	recVeStream.ui64TSP = evMap.value("tsp").toULongLong();
	recVeStream.uiGenTime = evMap.value("gentime").toUInt();
	recVeStream.sData.append((char*)evMap.value("data").toUInt(), recVeStream.uiLength);

	m_vedioStreamBuffer.enqueue(recVeStream);

	return 0;
}

int BufferManager::readVedioStream(RecordVedioStream &streamInfo)
{
	if (!m_vedioStreamBuffer.isEmpty())
	{
		streamInfo = m_vedioStreamBuffer.dequeue();
		if (200 == m_vedioStreamBuffer.size() && m_bVedioBufferIsFull)
		{
			m_bVedioBufferIsFull = false;
			emit action(QString("Continue"), this);
		}

		return 0;
	}
	else
		return 1;
}


int BufferManager::emptyBuff()
{
	m_vedioStreamBuffer.clear();


	return 0;
}

void BufferManager::audioSwitch(bool mode)
{
	m_bStopAudio = mode;
}

bool BufferManager::getAudioStatus()
{
	return m_bStopAudio;
}

int BufferManager::getVedioBufferSize()
{
	return m_vedioStreamBuffer.size();
}