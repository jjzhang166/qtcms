#include "BufferManager.h"
#include "GlobalSettings.h"
#include <QDebug>



BufferManager::BufferManager(void):
m_bVedioBufferIsFull(false),
m_bStopAudio(false),
m_bStopBuff(false)
{
}


BufferManager::~BufferManager(void)
{
	if (!m_StreamBuffer.isEmpty())
	{
		emptyBuff();
	}
}

// int BufferManager::recordAudioStream(QVariantMap &evMap)
// {
// 	//´ý¶¨
// 	return 0;
// }

int BufferManager::recordStream(QVariantMap &evMap)
{
	if (evMap.isEmpty())
	{
		return 1;
	}

	if (m_StreamBuffer.size() <= 100 && !m_bStopBuff)
	{
		emit bufferStatus(m_StreamBuffer.size(), this);
	}
	if (100 == m_StreamBuffer.size())
	{
		m_bStopBuff = true;
		emit action(QString("StartPlay"), this);
	}

	if (1000 <= m_StreamBuffer.size() && !m_bVedioBufferIsFull)
	{
		m_bVedioBufferIsFull = true;
		emit action(QString("Pause"), this);
	}

	RecordStreamFrame recStream;
	int frameType = evMap["frametype"].toUInt();
	if (0 == frameType)
	{
		//if (!getAudioStatus())
		//{
		//	return 0;
		//}
		recStream.uiLength = evMap["length"].toUInt();
		recStream.cFrameType = frameType;
		recStream.cChannel = evMap["channel"].toUInt();
		recStream.uiAudioSampleRate = evMap["audioSampleRate"].toUInt();
// 		memcpy(recStream.cAudioFormat, (char*)(evMap["audioFormat"].toUInt()), sizeof(recStream.cAudioFormat));
		recStream.uiAudioDataWidth = evMap["audioDataWidth"].toUInt();
		recStream.ui64TSP = evMap["pts"].toULongLong();
		recStream.uiGenTime = evMap["gentime"].toUInt();

		recStream.pData = new char[recStream.uiLength];
		memcpy(recStream.pData, (char*)evMap["data"].toUInt(), recStream.uiLength);
	}
	else if (1 == frameType || 2 == frameType)
	{
		recStream.uiLength = evMap.value("length").toUInt();
		recStream.cFrameType = evMap.value("frametype").toUInt();
		recStream.cChannel = evMap.value("channel").toUInt();
		recStream.uiWidth = evMap.value("width").toInt();
		recStream.uiHeight = evMap.value("height").toUInt();
		recStream.uiFrameRate = evMap.value("framerate").toUInt();
		recStream.ui64TSP = evMap.value("pts").toULongLong();
		recStream.uiGenTime = evMap.value("gentime").toUInt();
		recStream.pData = new char[recStream.uiLength];
		memcpy(recStream.pData, (char*)evMap["data"].toUInt(), recStream.uiLength);
	}


	m_StreamBuffer.enqueue(recStream);

	return 0;
}

int BufferManager::readStream(RecordStreamFrame &streamInfo)
{
	if (!m_StreamBuffer.isEmpty())
	{
		if (1 == m_StreamBuffer.size())
		{
			QMutex mutex;
			mutex.lock();
			streamInfo = m_StreamBuffer.takeFirst();
// 			m_StreamBuffer.removeFirst();
			mutex.unlock();
		}
		else
		{
			m_mutex.lock();
			streamInfo = m_StreamBuffer.takeFirst();
// 			m_StreamBuffer.removeFirst();
			m_mutex.unlock();
		}

		if (200 == m_StreamBuffer.size() && m_bVedioBufferIsFull)
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
	m_bStopBuff = false;
	RecordStreamFrame streamInfo;
	while(!m_StreamBuffer.isEmpty())
	{
		m_mutex.lock();
		streamInfo = m_StreamBuffer.takeAt(0);
		delete streamInfo.pData;
		streamInfo.pData = NULL;
		m_mutex.unlock();
	}
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
	return m_StreamBuffer.size();
}

void BufferManager::removeItem(RecordStreamFrame* item)
{
	if (NULL != item && NULL != item->pData)
	{
		delete item->pData;
	}
}
