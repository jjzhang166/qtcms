#ifndef QFILEDATA_H
#define QFILEDATA_H
#include <LocalPlayerEx_global.h>
#include <QThread>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QVariantMap>



class QFileData : public QThread
{
	Q_OBJECT

public:
	QFileData();
	~QFileData();
	void setParamer(QStringList lstFileList, uint uiStartSec, uint uiEndSec, qint32 i32StartPos);
	void setWndIdList(QList<qint32> &wndList);
	void setBuffer(qint32 wndId, QList<FrameData> *pquFrameBuffer);
// 	void setCbTimeChange(pcbTimeChange pro, void* pUser);
	void startReadFile();
	void stopThread();
// 	void clearBuffer();

	typedef struct _tagCurBuffInof{
		_tagCurBuffInof():curBuffer(NULL),pBuffList(NULL),lastPos(0),curPos(0)/*,lastGMT(0),curGMT(0)*/,bIsFirstRead(true),bIsPlaying(false){}
		bool bIsFirstRead;
		bool bIsPlaying;
		char *curBuffer;
		QList<FrameData> *pBuffList;
		uint lastPos;
		uint curPos;
// 		uint lastGMT;
// 		uint curGMT;
		union{
			tagVideoConfigFrame curVideoConfig;
			tagAudioConfigFrame curAudioConfig;
		};
	}CurBuffInfo;
signals:
	void sigThrowException(QVariantMap &msg);
// 	void sigSkipTime(uint seconds);
	void sigStartPlay(uint wndId);
	void sigStopPlay();
protected:
	void run();
private:
	bool readFile(QStringList &filePathList, qint32 &startPos, char* buffer, qint32 buffSize);
	bool checkWndIdExist(uint *pChlArr, uint wndId);
// 	void checkBuffer(QMap<uint, CurBuffInfo>::iterator &iter);
	qint32 getMinBufferSize();
	qint32 getMaxBufferSize();
private:
	QStringList m_lstFileList;
	uint m_uiStartSec;				//start seconds for playing
	uint m_uiEndSec;				//end seconds for playing
	qint32 m_i32StartPos;			//start position in file list
	bool m_bStop;
	bool m_bPlayDirection;			//true:play false:playback

	char* m_pFileBuff1;
	char* m_pFileBuff2;

	pcbTimeChange m_pcbTimeChg;
	void* m_pUser;

	QList<qint32> m_wndList;
	QMap<uint, CurBuffInfo> m_wndBuffMap;
};

#endif // QFILEDATA_H
