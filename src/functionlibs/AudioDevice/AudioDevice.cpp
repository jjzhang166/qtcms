#include "AudioDevice.h"
#include <guid.h>


AudioDevice::AudioDevice() :
m_nRef(0),
m_uiWind(-1),
m_hWaveOut(NULL),
m_bIsRunning(false),
m_bIsEnd(false)
{
	m_nBufferCount = WAVEDEV_BLOCK_COUNT;
	m_pBuffer = AllocateBlocks(WAVEDEV_BLOCK_SIZE, WAVEDEV_BLOCK_COUNT);

 	start();
	m_bIsEnd = false;
}

AudioDevice::~AudioDevice()
{
	m_bIsEnd = true;
	while(m_bIsRunning)
	{
		msleep(10);
	}

	FreeBlocks(m_pBuffer);
}


int AudioDevice::SetMainWnd(int nWnd)
{
	m_uiWind = nWnd;
	return 0;
}
int AudioDevice::SetAudioParam(int nChannel,int nSampleRate,int nSampleWidth)
{
	if (NULL != m_hWaveOut)
	{
		m_waveDev.winWaveOutClose(m_hWaveOut);
		m_hWaveOut = NULL;
	}
	m_waveDev.winWaveOutInitCb(CountChange, (void*)this);
	m_waveDev.winWaveOutOpen(&m_hWaveOut, nChannel, nSampleRate, nSampleWidth);

	return 0;
}
int AudioDevice::SetVolume(unsigned int uiPersent)
{
	m_waveDev.winWaveOutSetVolume(m_hWaveOut, uiPersent);
	return 0;
}
int AudioDevice::PlayBuffer(char *pBuffer,int nSize)
{
	if (NULL == pBuffer || nSize <= 0)
	{
		return 1;
	}
	BufferListNode node;
	node.pBuffer = new char[nSize];
	node.nBufferSize = nSize;
	memcpy(node.pBuffer, pBuffer, nSize);

	m_mutexBufList.lock();
	m_bufferList.append(node);
	m_mutexBufList.unlock();

// 	m_waveDev.winWaveOutRestart(m_hWaveOut);
	return 0;
}
int AudioDevice::Stop()
{
	m_bIsEnd = true;
	while(m_bIsRunning)
	{
		msleep(10);
	}

	m_waveDev.winWaveOutReset(m_hWaveOut);
	m_waveDev.winWaveOutClose(m_hWaveOut);

	return 0;
}

void AudioDevice::run()
{
	BufferList::iterator iter;
	BufferListNode node;
	bool bReadBuf = false;
	int nCurrentBolck = 0;
	m_bIsRunning = true;

	while(!m_bIsEnd)
	{
		bReadBuf = false;
		m_mutexBufList.lock();
		if (!m_bufferList.isEmpty())
		{
			iter = m_bufferList.begin();
			node = *iter;
			m_bufferList.pop_front();
			bReadBuf = true;
		}
		m_mutexBufList.unlock();

		if (bReadBuf)
		{
			WaveInfo *curWaveBlock = NULL;
			int reMain = 0;
			curWaveBlock = &m_pBuffer[nCurrentBolck];
			int nSize = node.nBufferSize;
			char *pData = node.pBuffer;
			while(nSize > 0 && !m_bIsEnd)
			{
				if (curWaveBlock->ulFlags & WHDR_PREPARED)
				{
					m_waveDev.winWaveOutUnprepareHeader(m_hWaveOut, curWaveBlock);
				}
				// 将数据写入当前块
				if (nSize < (int)WAVEDEV_BLOCK_SIZE - curWaveBlock->ulUser)
				{
					memcpy(curWaveBlock->pData + curWaveBlock->ulUser, pData, nSize);
					curWaveBlock->ulUser += nSize;
					// 块未写满，则退出
					break;
				}
				// 块写满
				reMain = WAVEDEV_BLOCK_SIZE - curWaveBlock->ulUser;
				memcpy(curWaveBlock->pData + curWaveBlock->ulUser, pData, reMain);
				nSize -= reMain;
				pData += reMain;
				curWaveBlock->ulBufferLength = WAVEDEV_BLOCK_SIZE;

				// 输出到音频输出设备
				m_waveDev.winWaveOutPrepareHeader(m_hWaveOut, curWaveBlock);
				m_waveDev.winWaveOutWrite(m_hWaveOut, curWaveBlock);

				// 空闲Block数-1
				m_mutexBufCount.lock();
				m_nBufferCount--;
				m_mutexBufCount.unlock();
				while(!m_nBufferCount && !m_bIsEnd)
				{
					msleep(10);
				}
				// 当前块指针偏移
				nCurrentBolck++;
				nCurrentBolck %= WAVEDEV_BLOCK_COUNT;

				// 设置当前块的用户参数(块内数据)
				curWaveBlock = &m_pBuffer[nCurrentBolck];
				curWaveBlock->ulUser = 0;
			}
			delete node.pBuffer;
			node.pBuffer = NULL;
		}
		else
		{
 			msleep(50);
		}
	}
	// 卸除头
	for (int i = 0; i <m_nBufferCount; ++i)
	{
		if (m_pBuffer[i].ulFlags & WHDR_PREPARED)
		{
			m_waveDev.winWaveOutUnprepareHeader(m_hWaveOut, &m_pBuffer[i]);
		}
	}
	// 清楚所有Buffer
	m_mutexBufList.lock();
	while(!m_bufferList.isEmpty())
	{
		iter = m_bufferList.begin();
		node = *iter;
		delete node.pBuffer;
		node.pBuffer = NULL;
		m_bufferList.pop_front();
	}
	m_mutexBufList.unlock();

	m_bIsRunning = false;
}
void AudioDevice::CountChg(void* hWaveOut, const unsigned int &msg)
{
	if (m_hWaveOut == hWaveOut && 0x3BD == msg)
	{
		m_mutexBufCount.lock();
		++m_nBufferCount;
		m_mutexBufCount.unlock();
	}
}

void CountChange(void* hWaveOut, const unsigned int &msg, void* pUser)
{
	if (NULL != hWaveOut && NULL != pUser)
	{
		((AudioDevice*)pUser)->CountChg(hWaveOut, msg);
	}
}

WaveInfo* AudioDevice::AllocateBlocks(int blockSize, int blockCount)
{
	WaveInfo* blocks;
	int totalSize = (sizeof(WaveInfo) + blockSize)*blockCount;
	unsigned char* buffer = new unsigned char[totalSize];

	memset(buffer, 0, totalSize);
	blocks = (WaveInfo*)buffer;
	buffer += sizeof(WaveInfo)*blockCount;
	for (int i = 0; i < blockCount; ++i)
	{
		blocks[i].ulBufferLength = blockSize;
		blocks[i].pData = (char*)buffer;
		buffer += blockSize;
	}

	return blocks;
}

void AudioDevice::FreeBlocks(WaveInfo *pBlock)
{
	if (NULL != pBlock)
	{
		delete pBlock;
		pBlock = NULL;
	}
}

long __stdcall AudioDevice::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IAudioDevice == iid)
	{
		*ppv = static_cast<IAudioDevice *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall AudioDevice::AddRef()
{
	m_csRef.lock();
	m_nRef++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall AudioDevice::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef-- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}