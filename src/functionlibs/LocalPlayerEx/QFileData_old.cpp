#include "QFileData.h"
#include <QFile>



#include <QDebug>


#define DEF_EXCEPTION(expCode, wndId, filePath)\
		QVariantMap msg;\
		msg.insert("wndId", wndId);\
		msg.insert("expCode", expCode);\
		msg.insert("filepath", filePath);\
		emit sigThrowException(msg);

#define CURRENT_POSITION(frameHead) m_bPlayDirection?frameHead->tPerFrameIndex.uiNextFrame:frameHead->tPerFrameIndex.uiPreFrame;

#define COPY_FRAME_DATA(pFileFrameHead, iter, size)\
	FrameData frameData(pFileFrameHead->tFrameHead);\
	memcpy(&frameData.AudioConfig, &iter->curAudioConfig, size);\
	m_bPlayDirection ? iter->pBuffList->push_back(frameData) : iter->pBuffList->push_front(frameData);


QFileData::QFileData()
	: QThread(),
	m_uiStartSec(0),
	m_uiEndSec(0),
	m_i32StartPos(0),
	m_bStop(false),
	m_bPlayDirection(true)
{

}

QFileData::~QFileData()
{

}

void QFileData::setParamer( QStringList lstFileList, uint uiStartSec, uint uiEndSec, qint32 i32StartPos )
{
	if (!lstFileList.isEmpty())
	{
		m_lstFileList = lstFileList;
		m_uiStartSec = uiStartSec;
		m_uiEndSec = uiEndSec;
		m_i32StartPos = i32StartPos;
	}
}

void QFileData::setWndIdList(QList<qint32> &wndList)
{
	m_wndList = wndList;
}

void QFileData::setBuffer(qint32 wndId, QList<FrameData> *pquFrameBuffer )
{
	if (pquFrameBuffer && wndId > 0)
	{
		CurBuffInfo curBuffInfo;
		curBuffInfo.pBuffList = pquFrameBuffer;
		m_wndBuffMap.insert(wndId, curBuffInfo);
	}
}

void QFileData::startReadFile()
{
	if (!isRunning())
	{
		m_bStop = false;
		start();
	}
}

void QFileData::stopThread()
{
	if (isRunning())
	{
		m_bStop = true;
		wait();
	}
	m_wndBuffMap.clear();
}

void QFileData::run()
{
	uint skiptime = MAX_SECONDS;
	tagFileHead *pFileHead = NULL;
	char *pFileBuff1 = new char[BUFFER_SIZE];
	char *pFileBuff2 = new char[BUFFER_SIZE];
	char *pFileBuff = pFileBuff1;
	if (!pFileBuff1 || !pFileBuff2)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"alloc memory error!";
		return;
	}

	memset(pFileBuff1, 0, BUFFER_SIZE);
	memset(pFileBuff2, 0, BUFFER_SIZE);

	while(!m_bStop)
	{
		QString filePath = m_lstFileList[m_i32StartPos++];
		if (!readFile(filePath, pFileBuff, BUFFER_SIZE))
		{
			continue;
		}
		tagFileHead *fileHead = (tagFileHead *)pFileBuff;
		QMap<uint, CurBuffInfo>::iterator iter = m_wndBuffMap.begin(); 

		while(!m_bStop)
		{
			//find wndID in file
			while (!checkWndIdExist(fileHead->uiChannels, iter.key()))
			{
				DEF_EXCEPTION(2, m_wndList.indexOf(iter.key()), filePath);
				++iter;
			}
			checkBuffer(iter);//find a buffer which is not full
			//first read file
			if (iter->bIsFirstRead)
			{
				iter->curBuffer = pFileBuff;
				iter->curPos = fileHead->tIFrameIndex.uiFirstIFrame[iter.key()];
				iter->bIsFirstRead = false;
			}
			tagFileFrameHead *pFileFrameHead = (tagFileFrameHead*)(iter->curBuffer + iter->curPos);

			//find right position in file
			if (pFileFrameHead->tFrameHead.uiGentime >= m_uiStartSec)
			{
				skiptime = qMin(skiptime, pFileFrameHead->tFrameHead.uiGentime - m_uiStartSec);
			}
			else
			{
				while (!m_bStop && pFileFrameHead->tFrameHead.uiGentime < m_uiStartSec)
				{
					iter->lastPos = iter->curPos;
					iter->curPos = CURRENT_POSITION(pFileFrameHead);
					pFileFrameHead = (tagFileFrameHead *)(iter->curBuffer + iter->curPos);
				}
			}

			while (!m_bStop && iter->pBuffList->size() != MAX_FRAME_NUM)
			{
				//copy data to FrameData
				switch (pFileFrameHead->tFrameHead.uiType)
				{
				case FT_Audio:
					{
						COPY_FRAME_DATA(pFileFrameHead, iter, sizeof(tagAudioConfigFrame));
					}
					break;
				case FT_IFrame:
				case FT_PFrame:
				case FT_BFrame:
					{
						COPY_FRAME_DATA(pFileFrameHead, iter, sizeof(tagVideoConfigFrame));
					}
					break;
				case FT_AudioConfig:
					{
						memcpy(&iter->curAudioConfig, pFileFrameHead->tFrameHead.pBuffer, sizeof(tagAudioConfigFrame));
					}
					break;
				case FT_VideoConfig:
					{
						memcpy(&iter->curVideoConfig, pFileFrameHead->tFrameHead.pBuffer, sizeof(tagVideoConfigFrame));
					}
					break;
				default:
					break;
				}
				iter->lastPos = iter->curPos;
				iter->curPos = CURRENT_POSITION(pFileFrameHead);
				//across two files
				if (iter->lastPos > iter->curPos || iter->curPos)
				{
					char *curBuffer = (iter->curBuffer == pFileBuff1)?pFileBuff2:pFileBuff1;
					readFile(m_lstFileList[m_i32StartPos++], curBuffer, BUFFER_SIZE);
					iter->curBuffer = curBuffer;
				}
				pFileFrameHead = (tagFileFrameHead *)(iter->curBuffer + iter->curPos);
			}
		}
	}

	delete[] pFileBuff1;
	delete[] pFileBuff2;

}

bool QFileData::readFile( QString filePath, char* buffer, qint32 buffSize )
{
	memset(buffer, 0, buffSize);
	//check file exist
	if (!QFile::exists(filePath))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<filePath<<" don't exist!";
		return false;
	}
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"can't open "<<filePath;
		return false;
	}
	//read file to buffer
	qint64 fileSize = file.size();
	if (file.read(buffer, fileSize) != fileSize)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"read file to buffer error!";
		file.close();
		return false;
	}
	file.close();
	return true;
}

bool QFileData::checkWndIdExist( uint *pChlArr, uint wndId )
{
	if (wndId >= WNDMAX_SIZE/2)
	{
		++pChlArr;
		wndId %= WNDMAX_SIZE/2;
	}
	return ((*pChlArr)>>wndId) & 1;
}

void QFileData::checkBuffer( QMap<uint, CurBuffInfo>::iterator &iter )
{
	while(!m_bStop && iter->pBuffList->size() >= MAX_FRAME_NUM)
	{
		++iter;
		if (iter == m_wndBuffMap.end())
		{
			msleep(10);//wait for buffer read
			iter = m_wndBuffMap.begin();
		}
	}
}
