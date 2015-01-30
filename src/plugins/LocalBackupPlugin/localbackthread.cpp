#include "localbackthread.h"
#include "guid.h"
#include "IDisksSetting.h"
#include <QFile>
#include <QEventLoop>
#include <QTimer>
#include <QTextCodec>
#include <QtCore/qmath.h>
#include <QCoreApplication>
#include <QtXml/QtXml>


#include <QElapsedTimer>
#include <QDebug>

#define qDebug() qDebug()<<__FUNCTION__<<__LINE__

#define SEND_PROGRESS(item, wnd, progress)\
	item.clear();\
	item.insert("nChannel", wnd);\
	item.insert("Progress", progress);\
	emit sendMsg(QString("localFileBackUpProgress"), item);

LocalBackThread::LocalBackThread(QObject *parent)
	: QThread(parent),
	m_pStorage(NULL)
{
	//get usable disk list
	IDisksSetting *pDiskSet = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDiskSetting,(void**)&pDiskSet);
	if (pDiskSet)
	{
		pDiskSet->getEnableDisks(m_sDiskList);//get usable disks
		pDiskSet->Release();
	}

	initFileSystem();
}

LocalBackThread::~LocalBackThread()
{
	if (isRunning()){
		m_bStop = true;
		wait();
	}
	QMap<QString, sqlite3*>::iterator it = m_sqlMap.begin();
	while (it != m_sqlMap.end())
	{
		sqlite3_close(*it++);
	}
	m_sqlMap.clear();
	if (m_pStorage){
		m_pStorage->Release();
		m_pStorage = NULL;
	}
}

sqlite3* LocalBackThread::initDataBase(char *dbPath)
{
	if (!m_sqlMap.contains(QString(dbPath))){
		sqlite3 *pdb = NULL;
		int ret = sqlite3_open(dbPath, &pdb);
		if (ret != SQLITE_OK){
			return NULL;
		}else{
			//check if database has a table
			char **ppDbRet = NULL;
			int nRow = 0, nColumn = 0;
			sqlite3_get_table(pdb, "select count(*) from sqlite_master where type='table';", &ppDbRet, &nRow, &nColumn, NULL);
			if (!ppDbRet){
				qDebug()<<__FUNCTION__<<__LINE__<<"check table error in "<<dbPath;
				sqlite3_close(pdb);
				return NULL;
			}
			if (!strcmp(ppDbRet[nColumn], "0")){
				qDebug()<<__FUNCTION__<<__LINE__<<"no table: search_record in"<<dbPath;
				sqlite3_free_table(ppDbRet);
				sqlite3_close(pdb);
				return NULL;
			}

			sqlite3_free_table(ppDbRet);
			m_sqlMap.insert(QString(dbPath), pdb);
			return pdb;
		}
	}else{
		return m_sqlMap.value(QString(dbPath));
	}
}

void LocalBackThread::setBackupPath( QString sPath )
{
	m_sBackupPath = sPath;
}

int LocalBackThread::startLocalFileBackUp( int nTypes, QString sChannel, QDateTime startTime, QDateTime endTime )
{
	if (!isRunning()){
		m_nTypes = nTypes;
		m_sChls = sChannel;
		m_nStartSec = startTime.toTime_t();
		m_nEndSec = endTime.toTime_t();
		m_bStop = false;
		this->start();
		return 0;
	}
	return 1;
}

void LocalBackThread::stopLocalFileBackUp()
{
	if (isRunning()){
//		qDebug()<<"stop backup";
		m_bStop = true;
	}
}

void LocalBackThread::run()
{
	qDebug()<<"backup start ===================";

	//get file list for backup
	QStringList fileList;
	if (!getFileList(fileList)){
		return;
	}
	char *buffer = new char[MAX_BUFF_SIZE];
	int fileIndex = 0;
	bool bStop = false;
	FileHead *pFileHead = NULL;
	FileFrameHead *pFrameHead = NULL;
	FdIterator iter = m_chlFdMap.end();
	int steps = EM_INIT;
	QVariantMap item;
	item.insert("types", 1);
	emit sendMsg(QString("localFileBackUpState"), item);
	while (!bStop){
		if (m_bStop){
			qDebug()<<"set step = STOP";
			steps = EM_STOP;
		}
		switch (steps)
		{
		case EM_INIT:
			{
				//read file over
				if (fileIndex == fileList.size()){
					FdIterator it = m_chlFdMap.begin();
					while (it != m_chlFdMap.end()){
						SEND_PROGRESS(item, it.key(), 100);
						++it;
					}
					steps = EM_STOP;
					break;
				}
				qDebug()<<"start read file "<<fileList[fileIndex];

//				QElapsedTimer timer;
//				qint64 pos1 = timer.elapsed();

				memset(buffer, 0, MAX_BUFF_SIZE);
				QString fileName = fileList[fileIndex];
				//check file exist
				if (!QFile::exists(fileName)){
					qDebug()<<fileName<<" don't exist!";
					fileIndex++;
					steps = EM_INIT;
					break;
				}
				//lock file in database
				lockFile(fileName, true);

//				qint64 pos2 = timer.elapsed();

				//read file
				QFile file(fileName);
				if (!file.open(QIODevice::ReadOnly)){
					qDebug()<<"open file "<<fileName<<" error!";
					fileIndex++;
					steps = EM_INIT;
					lockFile(fileName, false);
					break;
				}

//				qint64 pos3 = timer.elapsed();

				//read file to buffer
				file.read(buffer, MAX_BUFF_SIZE);
				file.close();
				fileIndex++;

//				qint64 pos4 = timer.elapsed();

				//unlock file in database
				lockFile(fileName, false);

//				qint64 pos5 = timer.elapsed();
//				qDebug()<<"lock:"<<pos2 - pos1<<"open:"<<pos3 - pos2<<"read:"<<pos4 - pos3<<"unlock:"<<pos5 - pos4<<"total:"<<pos5 - pos1;

				qDebug()<<"read file "<<fileName<<" success!";

				//fill first frame
				pFileHead = (FileHead *)buffer;
				pFrameHead = (FileFrameHead *)(buffer + sizeof(FileHead));

				steps = EM_READ_BUFF;
			}
			break;
		case EM_READ_BUFF:
			{
				//check whether reach the file tail
				if ((char*)pFrameHead >= buffer + pFileHead->uiIndex){
					steps = EM_INIT;
					break;
				}
				//check whether the frame is usable
				if (pFrameHead->tFrameHead.uiGentime < m_nStartSec 
					|| !(pFrameHead->tFrameHead.uiRecType & m_nTypes)
					|| !m_chlFdMap.contains(pFrameHead->tFrameHead.uiChannel)){
					pFrameHead = (FileFrameHead *)((char*)pFrameHead + sizeof(FileFrameHead) - sizeof(pFrameHead->tFrameHead.pBuffer) + pFrameHead->tFrameHead.uiLength);
					steps = EM_READ_BUFF;
					break;
				}
				iter = m_chlFdMap.find(pFrameHead->tFrameHead.uiChannel);
				//check whether reach the end time
				if (pFrameHead->tFrameHead.uiGentime > m_nEndSec){
					SEND_PROGRESS(item, iter.key(), 100);

					qDebug()<<"wnd:"<<iter.key()<<" has finished, persent 100%";

					iter->finished = true;
					steps = EM_PACK;
					break;
				}
				//filter non-I-Frame when file doesn't create
				if (EM_NO_CREATED == iter->fileStatus && FT_IFrame != pFrameHead->tFrameHead.uiType){
					pFrameHead = (FileFrameHead *)((char*)pFrameHead + sizeof(FileFrameHead) - sizeof(pFrameHead->tFrameHead.pBuffer) + pFrameHead->tFrameHead.uiLength);
					steps = EM_READ_BUFF;
					break;
				}
				//record video width and height
				if (FT_VideoConfig == pFrameHead->tFrameHead.uiType){
					VideoConfigFrame *pVideoCfg = (VideoConfigFrame*)&pFrameHead->tFrameHead.pBuffer;
					iter->width = pVideoCfg->uiWidth;
					iter->height = pVideoCfg->uiHeight;
					pFrameHead = (FileFrameHead *)((char*)pFrameHead + sizeof(FileFrameHead) - sizeof(pFrameHead->tFrameHead.pBuffer) + pFrameHead->tFrameHead.uiLength);
					steps = EM_READ_BUFF;
					break;
				}
				//record audio samplewidth, samplerate and channel
				if (FT_AudioConfig == pFrameHead->tFrameHead.uiType){
					AudioConfigFrame *pAudioCfg = (AudioConfigFrame*)&pFrameHead->tFrameHead.pBuffer;
					iter->samplebit = pAudioCfg->uiSamplebit;
					iter->samplerate = pAudioCfg->uiSamplerate;
					iter->channel = pAudioCfg->uiChannels;
					pFrameHead = (FileFrameHead *)((char*)pFrameHead + sizeof(FileFrameHead) - sizeof(pFrameHead->tFrameHead.pBuffer) + pFrameHead->tFrameHead.uiLength);
					steps = EM_READ_BUFF;
					break;
				}

				//find first I-Frame
				if (EM_NO_CREATED == iter->fileStatus && FT_IFrame == pFrameHead->tFrameHead.uiType){
					steps = EM_CREATE_FILE;
					break;
				}
				steps = EM_CHECK_SIZE;
			}
			break;
		case EM_CREATE_FILE:
			{
				QString startTimeStr = QDateTime::fromTime_t(m_nStartSec).toString("yyyyMMddhhmmss");
				QString endTimeStr = QDateTime::fromTime_t(m_nEndSec).toString("yyyyMMddhhmmss");
				QString temp;
				temp = temp.sprintf("/WND%02d_%s_%s_PART%d.avi", pFrameHead->tFrameHead.uiChannel, startTimeStr.toLatin1().data(), endTimeStr.toLatin1().data(), iter->fileNum);
				QTextCodec * codec = QTextCodec::codecForLocale();
				QByteArray fileName = codec->fromUnicode(m_sBackupPath + temp);
				iter->fd = AVI_open_output_file(fileName.data());
				if (!iter->fd){
					qDebug()<<"open file "<<fileName<<" error!";
					steps = EM_STOP;
					break;
				}
				iter->fileStatus++;
				iter->keyPts = pFrameHead->tFrameHead.uiPts;
				if (iter->firstFile){
					iter->firstFile = false;
					iter->startGMT = pFrameHead->tFrameHead.uiGentime;
				}
				AVI_set_video(iter->fd, iter->width, iter->height, 25, "X264");
				steps = EM_CHECK_SIZE;
			}
			break;
		case EM_CHECK_SIZE:
			{
				if (256*1024*1024 < AVI_bytes_written(iter->fd) + pFrameHead->tFrameHead.uiLength){
					iter->fileStatus = EM_WAIT_FOR_PACK;
				}
				if (!checkDisk(m_sBackupPath)){
					qDebug()<<"you disk space < 2GB, there is no enough space to backup!";
					steps = EM_STOP;
					break;
				}
				steps = EM_WRITE_FRAME;
			}
			break;
		case EM_WRITE_FRAME:
			{
				//trun to pack
				if (EM_WAIT_FOR_PACK == iter->fileStatus && FT_IFrame == pFrameHead->tFrameHead.uiType){
					steps = EM_PACK;
					break;
				}
				//write frame
				if (FT_Audio == pFrameHead->tFrameHead.uiType){
					if (!iter->audioBeSet){
						iter->audioBeSet = true;
						AVI_set_audio(iter->fd, iter->channel, iter->samplerate, iter->samplebit, WAVE_FORMAT_ALAW, 64);
					}
					AVI_write_audio(iter->fd, (char*)&pFrameHead->tFrameHead.pBuffer, pFrameHead->tFrameHead.uiLength);
				}else{
					AVI_write_frame(iter->fd, (char*)&pFrameHead->tFrameHead.pBuffer, pFrameHead->tFrameHead.uiLength, (FT_IFrame == pFrameHead->tFrameHead.uiType));
					//count frame rate
					iter->frameCounts++;
					if (pFrameHead->tFrameHead.uiPts - iter->keyPts >= 1000){
						iter->keyPts = pFrameHead->tFrameHead.uiPts;
						iter->ptsCountMap[iter->frameCounts]++;
						iter->frameCounts = 0;
					}
					//count progress
					int curProgress = (pFrameHead->tFrameHead.uiGentime - iter->startGMT)*100/(m_nEndSec - iter->startGMT);
					if (iter->progress < curProgress){
						iter->progress = curProgress;
						SEND_PROGRESS(item, iter.key(), curProgress);
						sleepEx(10);

// 						qDebug()<<"wnd:"<<iter.key()<<" progress: "<<curProgress;
					}
				}
				//get next frame
				pFrameHead = (FileFrameHead *)((char*)pFrameHead + sizeof(FileFrameHead) - sizeof(pFrameHead->tFrameHead.pBuffer) + pFrameHead->tFrameHead.uiLength);
				steps = EM_READ_BUFF;
			}
			break;
		case EM_PACK:
			{
				//close file
				AVI_set_video(iter->fd, iter->width, iter->height, countPts(iter->ptsCountMap), "X264");
				AVI_close(iter->fd);
				//clear old data
				iter->fd = NULL;
				iter->fileNum++;
				iter->fileStatus = EM_NO_CREATED;
				iter->audioBeSet = false;
				if (iter->finished){
					m_chlFdMap.remove(iter.key());
					steps = EM_READ_BUFF;
				}else{
					steps = EM_CREATE_FILE;
				}
			}
			break;
		case EM_STOP:
			{
				//pack all file
				FdIterator it = m_chlFdMap.begin();
				while (it != m_chlFdMap.end()){
					if (!it->fd){
						++it;
						continue;
					}
					AVI_set_video(it->fd, it->width, it->height, countPts(it->ptsCountMap), "X264");
					AVI_close(it->fd);
					++it;
				}
				bStop = true;
			}
			break;
		}
	}
	item.clear();
	item.insert("types", 2);
	emit sendMsg(QString("localFileBackUpState"), item);

	qDebug()<<"backup end ==================";

	delete[] buffer;
	buffer = NULL;
	m_chlFdMap.clear();
}

bool LocalBackThread::getFileList( QStringList &list )
{
	qint32 insertPos = -1;
	QList<TimePath> tpList;
	QString sqlCommand;
	createSQLCmd(sqlCommand);
	QStringList diskList = m_sDiskList.split(":", QString::SkipEmptyParts);
	foreach(QString disk, diskList)
	{
		//get database path
		QString sDbName = disk + ":/recEx/record.db";
		if (!QFile::exists(sDbName)){
			qDebug()<<sDbName<<" didn't exists!";
			continue;
		}
		//create sqlite3 interface
		sqlite3 *pdb = NULL;
		int ret = sqlite3_open(sDbName.toLatin1().data(), &pdb);
		if (SQLITE_OK != ret){
			qDebug()<<"get database point error!";
			continue;
		}
		char **ppDbRet = NULL, *pErrMsg = NULL;
		int nRow = 0, nColumn = 0, index = 0;
		QStringList excludeFilelst;
		//find locked file or damaged file
		ret = execCommand(pdb, "select sFilePath from RecordFileStatus where (nLock=1 or nDamage=1);", &ppDbRet, &nRow, &nColumn, &pErrMsg);
		if (SQLITE_OK == ret){
			index = nColumn;
			for (int i = 0; i < nRow; ++i)
				excludeFilelst<<QString(ppDbRet[index++]);
		}else{
			qDebug()<<"search RecordFileStatus failed. error: "<<pErrMsg;
		}
		//search file path
		ret = execCommand(pdb, sqlCommand.toLatin1().data(), &ppDbRet, &nRow, &nColumn, &pErrMsg);
		if (SQLITE_OK == ret)
		{
			index = nColumn;
			for (int i = 0; i < nRow; ++i)
			{
				uint start = atoi(ppDbRet[index++]);
				QString path = QString(ppDbRet[index++]);
				//exclude file from file list
				if (excludeFilelst.contains(path))
					continue;
				appendTimePath(tpList, start, path, insertPos);
			}
		}else{
			qDebug()<<"search record failed. error: "<<pErrMsg;
		}
		sqlite3_free_table(ppDbRet);
	}
	if (tpList.isEmpty()){
		return false;
	}

	//deduplication
	QString lastPath = tpList.at(0).path;
	QString tempPath;
	list.append(lastPath);
	for (int index = 1; index < tpList.size(); ++index){
		tempPath = tpList.at(index).path;
		if (lastPath == tempPath){
			continue;
		}else{
			list.append(tempPath);
			lastPath = tempPath;
		}
	}
	return true;
}

void LocalBackThread::createSQLCmd( QString &sql )
{
	QStringList temp;
	QString chlCmd, typeCmd;
	//get sql for wndId
	for (int pos = m_sChls.size() - 1; pos >= 0; --pos){
		if (m_sChls[pos] == QChar('1')){
			temp<<QString("nWndId=%1").arg(m_sChls.size() - 1 - pos);
			m_chlFdMap.insert(m_sChls.size() - 1 - pos, FdStatusInfo());
		}
	}
	chlCmd = temp.join(" or ");
	//get sql for types
	int position = 1;
	int nTypes = m_nTypes;
	temp.clear();
	while (nTypes > 0){
		if (nTypes & 1){
			temp<<QString("nRecordType=%1").arg(position);
		}
		position++;
		nTypes = nTypes>>1;
	}
	typeCmd = temp.join(" or ");
	//create total sql
	sql = QString("select nStartTime, sFilePath from record where (%1) and (%2) and nStartTime < %3 and nEndTime > %4 and nStartTime!=nEndTime order by nStartTime;").arg(chlCmd).arg(typeCmd).arg(m_nEndSec).arg(m_nStartSec);
}

int LocalBackThread::execCommand( sqlite3 *pdb, const char* cmd, char*** pppRet, int* row, int* col, char** pMsg )
{
	int ret = -1;
	int count = 0;
	while ((ret = sqlite3_get_table(pdb, cmd, pppRet, row, col, pMsg)) != SQLITE_OK && count < 50){
		//wait 3 milliseconds
		QEventLoop evLoop;
		QTimer::singleShot(3, &evLoop, SLOT(quit()));
		evLoop.exec();
		++count;
	}
	return ret;
}

void LocalBackThread::appendTimePath( QList<TimePath> &tpList, const uint &start, const QString &path, qint32 &insertPos )
{
	TimePath tp = {start, path};
	qint32 end = tpList.size() - 1;
	if (tpList.isEmpty() 
		|| (insertPos == end && tpList.at(insertPos).start < start) 
		|| (insertPos < end && tpList.at(insertPos).start < start && tpList.at(insertPos + 1).start > start)){
		tpList.insert(++insertPos, tp);
	}else{
		qint32 pos = tpList.size() - 1;
		while (tpList.at(pos).start > start){
			--pos;
			if (!pos){
				tpList.insert(pos, tp);
				insertPos = pos;
				return;
			}
		}
		tpList.insert(pos + 1, tp);
		insertPos = pos + 1;
	}
}

int LocalBackThread::countPts( QMap<int,int> ptsMap )
{
	int count = 0;
	QMap<int, int>::iterator it = ptsMap.begin();
	while (it != ptsMap.end()){
// 		qDebug()<<"rate:"<<it.key()<<" times:"<<*it;
		if (count < *it){
			count = *it;
		}
		++it;
	}
	return ptsMap.key(count);
}

void LocalBackThread::lockFile( QString fileName, bool locked )
{
	QString dbPath = fileName.left(2) + "/recEx/record.db";
	sqlite3 *pdb = initDataBase(dbPath.toLatin1().data());
	if (!pdb){
		qDebug()<<"open database "<<dbPath<<" fail";
		return;
	}

	QRegExp rx("(\\d+)");
	int mi = 3, fileNum = 0, pos = 0;
	while ((pos = rx.indexIn(fileName, pos)) != -1){
		fileNum += rx.cap(1).toInt()*qPow(256, mi--);
		pos += rx.matchedLength();
	}
	QString cmd = QString("update RecordFileStatus set nLock=%1 where nFileNum=%2;").arg(locked).arg(fileNum);
	sqlite3_stmt * stmt = NULL;
	int ret = sqlite3_prepare_v2(pdb, cmd.toLatin1().data(), -1, &stmt, NULL);
	if (SQLITE_OK != ret){
		qDebug()<<"update RecordFileStatus error: ";
	}
}

void LocalBackThread::sleepEx( int msec )
{
	QEventLoop evloop;
	QTimer::singleShot(msec,&evloop,SLOT(quit()));
	evloop.exec();
}


void LocalBackThread::initFileSystem()
{
	// ¡ä¨°?a???????t
	QString sConfFilePath = QCoreApplication::applicationDirPath() + QString("/pcom_config.xml");
	QDomDocument confFile;
	QFile * file = new QFile(sConfFilePath);
	file->open(QIODevice::ReadOnly);
	confFile.setContent(file);

	// ??¨¨?filesystem¨¤¨¤¦Ì?¡Á¨¦?t
	QDomNode clsidNode = confFile.elementsByTagName("CLSID").at(0);
	QDomNodeList items = clsidNode.childNodes();
	for (int i = 0; i < items.count(); i++){
		QDomNode node = items.at(i);
		QString name = node.toElement().attribute("name");
		// filesystem ¨¤¨¤D¨ª¦Ì?¡Á¨¦?t
		if (name.contains(QRegExp("^filesystem\\."))){
			QString sCLSID = node.toElement().attribute("clsid");
			GUID clsid_node = pcomString2GUID(sCLSID);
			// ¨º?¡¤??¡ì3?IStorage?¨®?¨²
			pcomCreateInstance(clsid_node,NULL,IID_IStorage,(void **)&m_pStorage);
			if (NULL == m_pStorage)
				continue;
			break;
		}
	}
	// ¨º¨ª¡¤?¡Á¨º?¡ä¡ä| file
	file->close();
	delete file;
}

bool LocalBackThread::checkDisk( QString sPath )
{
	int freesizem = 1024;
	quint64 FreeByteAvailable=0;
	quint64 TotalNumberOfBytes=0;
	quint64 TotalNumberOfFreeBytes=0;
	if (NULL == m_pStorage)
		return false;

	if (m_pStorage->GetFreeSpace(sPath,FreeByteAvailable,TotalNumberOfBytes,TotalNumberOfFreeBytes)){
		if (FreeByteAvailable>(quint64)freesizem*1024*1024*2)
			return true;
	}
	return false;
}
