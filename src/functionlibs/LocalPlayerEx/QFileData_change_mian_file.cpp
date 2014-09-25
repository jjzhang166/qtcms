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

#define FILE_POSITION(filePos) m_bPlayDirection?filePos++:filePos--

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
	m_bPlayDirection(true),
	m_pcbTimeChg(NULL),
	m_pUser(NULL)
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

void QFileData::setCbTimeChange( pcbTimeChange pro, void* pUser )
{
	if (pro && pUser)
	{
		m_pcbTimeChg = pro;
		m_pUser = pUser;
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
		readFile(m_lstFileList[FILE_POSITION(m_i32StartPos)], pFileBuff, BUFFER_SIZE);
		tagFileHead *fileHead = (tagFileHead *)pFileBuff;
		tagFileFrameHead *fileFrameHead = (tagFileFrameHead*)(pFileBuff + sizeof(tagFileHead));

		while (fileFrameHead < (tagFileFrameHead*)(pFileBuff + fileHead->uiIndex))
		{
			if (!m_wndBuffMap.contains(fileFrameHead->tFrameHead.uiChannel))
			{
				fileFrameHead = (tagFileFrameHead*)(pFileBuff + fileFrameHead->tPerFrameIndex.uiNextFrame);
				continue;
			}

			QMap<uint, CurBuffInfo>::iterator iter = m_wndBuffMap.find(fileFrameHead->tFrameHead.uiChannel); 
			if (iter == m_wndBuffMap.end())
			{
				fileFrameHead = (tagFileFrameHead*)(pFileBuff + fileFrameHead->tPerFrameIndex.uiNextFrame);
				continue;
			}
			switch(fileFrameHead->tFrameHead.uiType)
			{
			case FT_Audio:
				{
					COPY_FRAME_DATA(fileFrameHead, iter, sizeof(tagAudioConfigFrame));
				}
				break;
			case FT_IFrame:
			case FT_PFrame:
			case FT_BFrame:
				{
					COPY_FRAME_DATA(fileFrameHead, iter, sizeof(tagVideoConfigFrame));
				}
				break;
			case FT_AudioConfig:
				{
					memcpy(&iter->curAudioConfig, fileFrameHead->tFrameHead.pBuffer, sizeof(tagAudioConfigFrame));
				}
				break;
			case FT_VideoConfig:
				{
					memcpy(&iter->curVideoConfig, fileFrameHead->tFrameHead.pBuffer, sizeof(tagVideoConfigFrame));
				}
				break;
			default:
				break;
			}
			fileFrameHead = (tagFileFrameHead*)(pFileBuff + fileFrameHead->tPerFrameIndex.uiNextFrame);
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
