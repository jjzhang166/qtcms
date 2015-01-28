#include "playbackthread.h"
#include "guid.h"
#include <QApplication>
#include <QFile>
#include <QtXml/QtXml>

#define qDebug() qDebug()<<__FUNCTION__<<__LINE__

#define VLOG(info, status)\
	qDebug()<<(info);\
	m_curOperate = status;\
	break;

PlayBackThread::PlayBackThread()
	: QThread(),
	m_bStop(false),
	m_bInitFlag(false),
	m_fileKey(0),
	m_fileTotal(0),
	m_playTypes(0),
	m_nChannels(0),
	m_channelWithAudio(-1),
	m_nSpeedRate(0),
	m_playback(NULL),
	m_susWnd(NULL)
{

}

PlayBackThread::~PlayBackThread()
{
	m_bStop = true;
	if (isRunning()){
		wait();
	}
}

int PlayBackThread::setDeviceHostInfo( const QString & sAddress,unsigned int uiPort,const QString &eseeID )
{
	m_devInfo.m_sAddress = sAddress;
	m_devInfo.m_uiPort = uiPort;
	m_devInfo.m_sEseeId = eseeID;
	return 0;
}

int PlayBackThread::setDeviceVendor( const QString & vendor )
{
	m_devInfo.m_sVendor = vendor;
	return 0;
}

void PlayBackThread::setUserVerifyInfo( const QString & sUsername,const QString & sPassword )
{
	m_devInfo.m_sUsername = sUsername;
	m_devInfo.m_sPassword = sPassword;
}

QString PlayBackThread::GetNowPlayedTime()
{
	QString strSec = "0";
	if (m_playMap.isEmpty())
		return strSec;
	PlayIter it = m_playMap.begin();
	int seconds = it->playManager->getPlayTime();
	seconds -= m_playStart.toTime_t();
	if (seconds > 0){
		strSec.setNum(seconds);
	}
	return strSec;
}

int PlayBackThread::startSearchRecFile( int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime )
{
	m_schInfo.nSearchChls = nChannel;
	m_schInfo.nSearchTypes = nTypes;
	m_schInfo.startTime = startTime;
	m_schInfo.endTime  = endTime;
	m_curOperate = EM_SEARCH;
	if (!isRunning()){
		start();
	}
	return 0;
}

int PlayBackThread::AddChannelIntoPlayGroup( uint uiWndId,int uiChannelId )
{
	if (getStreamInfo(uiChannelId)){
		qDebug()<<"fill data error";
		return 1;
	}
	if (uiWndId > m_wndList.size() || m_devInfo.m_uiChannelId > 32){
		qDebug()<<"parameter error";
		return 1;
	}
	if (m_playMap.size() >= 4){
		qDebug()<<"group is full";
		return 1;
	}
	//insert new item into the map
	int nChannel = m_devInfo.m_uiChannelId;
	QWidget *wnd = (QWidget*)m_wndList[uiWndId];
	WndPlay wndPlay;
	if (1 != ((m_nChannels>>(nChannel))&1))
	{
		m_nChannels |= 1<<(nChannel);

		wndPlay.bufferManager = new BufferManager();
		wndPlay.playManager = new PlayManager();

		removeRepeatWnd(wnd);
		wndPlay.wnd = wnd;

		QObject::connect(wndPlay.bufferManager, SIGNAL(action(QString, BufferManager*)), this, SLOT(action(QString, BufferManager*)));
		QObject::connect(wndPlay.bufferManager, SIGNAL(bufferStatus(int)), m_wndList[uiWndId], SLOT(setProgress(int)));

		m_playMap.insert(nChannel, wndPlay);
	}
	else
	{
		m_playMap[nChannel].wnd = wnd;
	}
	return 0;
}

int PlayBackThread::GroupPlay( int nTypes,const QDateTime & start,const QDateTime & end )
{
	m_playTypes = nTypes;
	m_playStart = start;
	m_playEnd = end;
	for (int index = 0; index < m_wndList.size(); ++index){
		m_wndList[index]->setProgress(0);
	}
	m_curOperate = EM_PLAY;
	return 0;
}

int PlayBackThread::GroupPause()
{
	m_curOperate = EM_PAUSE;
	PlayIter iter = m_playMap.begin();
	while (iter != m_playMap.end())
	{
		iter->playManager->pause(true);
		++iter;
	}
	return 0;
}

int PlayBackThread::GroupContinue()
{
	m_curOperate = EM_CONTINUE;
	PlayIter iter = m_playMap.begin();
	while (iter != m_playMap.end())
	{
		iter->playManager->pause(false);
		++iter;
	}
	return 0;
}

int PlayBackThread::GroupStop()
{
 	m_curOperate = EM_STOP;

	PlayIter iter = m_playMap.begin();
	while (iter != m_playMap.end()){
		iter->playManager->stop();
		iter->bufferManager->emptyBuff();
		delete iter->playManager;
		iter->playManager = NULL;
		delete iter->bufferManager;
		iter->bufferManager = NULL;
		++iter;
	}
	//clear outdate data
	m_nChannels = 0;
	m_playTypes = 0;
	m_playMap.clear();

	return 0;
}

int PlayBackThread::AudioEnabled( bool bEnable )
{
	if (m_playMap.isEmpty())
		return 1;
	PlayIter it = m_playMap.begin();
	if (NULL != it->playManager){
		it->playManager->AudioSwitch(bEnable);
	}

	if (!bEnable&&m_channelWithAudio!=-1){	
		it = m_playMap.find(m_channelWithAudio);
		m_channelWithAudio = -1;
	}
	return 0;
}

int PlayBackThread::SetVolume( const unsigned int &uiPersent, QWidget* pWnd )
{
	if ((int)uiPersent < 0)
		return 1;
	PlayIter it = m_playMap.begin();
	if (NULL == it->playManager)
		return 1;

	if (0xAECBCA == uiPersent){
		while(it != m_playMap.end())
		{
			if (it->wnd == pWnd)
				break;
			++it;
		}
		m_channelWithAudio = it.key();
		it->playManager->setCurAudioWnd(it->playManager);
	}else{
		it->playManager->setVolume(*const_cast<unsigned int*>(&uiPersent));
	}
	return 0;
}

int PlayBackThread::GroupSpeedFast()
{
	if (m_playMap.isEmpty())
		return 1;

	PlayIter it = m_playMap.begin();
	while (it != m_playMap.end()){
		m_nSpeedRate = 0;
		it->playManager->setPlaySpeed(1, 1);
		++it;
	}
	return 0;
}

int PlayBackThread::GroupSpeedSlow()
{
	if (m_playMap.isEmpty())
		return 1;

	m_nSpeedRate++;
	PlayIter it = m_playMap.begin();
	while (it != m_playMap.end()){
		m_nSpeedRate = 0;
		it->playManager->setPlaySpeed(2, m_nSpeedRate);
		++it;
	}
	return 0;
}

int PlayBackThread::GroupSpeedNormal()
{
	if (m_playMap.isEmpty())
		return 1;

	PlayIter it = m_playMap.begin();
	while (it != m_playMap.end()){
		m_nSpeedRate = 0;
		it->playManager->setPlaySpeed(0, 1);
		++it;
	}
	return 0;
}

void PlayBackThread::run()
{
	QVariantMap item;
	while (!m_bStop)
	{
		switch (m_curOperate)
		{
		case EM_SEARCH:
			{
				//get playback interface;
				if (!m_playback){
					getPlaybackInterface((void**)&m_playback);
				}
				if (!m_playback){
					QVariantMap item;
					item.insert("parm", 1);
					emit FileSearchFailToUiS(item);
					VLOG("get play back interface error!", EM_DEFAULT);
				}

				//if don't register event, register it
				if (!m_bInitFlag){
					initCallBackFun();
					m_bInitFlag = true;
				}
				//set device info
				IDeviceClient *pDevClient = NULL;
				m_playback->QueryInterface(IID_IDeviceClient, (void**)&pDevClient);
				if (!pDevClient){
					VLOG("get device client interface error!", EM_DEFAULT);
				}
				pDevClient->checkUser(m_devInfo.m_sUsername, m_devInfo.m_sPassword);
				pDevClient->setDeviceHost(m_devInfo.m_sAddress);
				pDevClient->setDeviceId(m_devInfo.m_sEseeId);
				pDevClient->setDevicePorts(m_devInfo.m_uiPort);
				pDevClient->Release();

				//start search
				int ret = m_playback->startSearchRecFile(m_schInfo.nSearchChls, m_schInfo.nSearchTypes, m_schInfo.startTime, m_schInfo.endTime);
				if (ret){
					QVariantMap item;
					item.insert("parm", ret == 1 ? 2 : 1);
					emit FileSearchFailToUiS(item);
					VLOG("search fault!", EM_DEFAULT);
				}
				m_bInitFlag = false;
				m_playback->Release();
				m_playback = NULL;
				m_curOperate = EM_DEFAULT;
			}
			break;
		case EM_PLAY:
			{
				//check interface
				if (!m_playback){
					getPlaybackInterface((void**)&m_playback);
					if (!m_playback){
						VLOG("get play back interface error!", EM_DEFAULT);
					}
				}
				//if don't register event, register it
				if (!m_bInitFlag){
					initCallBackFun();
					m_bInitFlag = true;
				}
				//connect the device
				IDeviceClient *pDevClient = NULL;
				m_playback->QueryInterface(IID_IDeviceClient, (void**)&pDevClient);
				if (!pDevClient){
					VLOG("get device client interface error!", EM_DEFAULT);
				}
				pDevClient->checkUser(m_devInfo.m_sUsername, m_devInfo.m_sPassword);
				pDevClient->setDeviceHost(m_devInfo.m_sAddress);
				pDevClient->setDeviceId(m_devInfo.m_sEseeId);
				pDevClient->setDevicePorts(m_devInfo.m_uiPort);
				int ret = pDevClient->connectToDevice();
				if (ret){
					VLOG("connect device fault!", EM_DEFAULT);
				}
				pDevClient->Release();
				//start play
				ret = m_playback->getPlaybackStreamByTime(m_nChannels, m_playTypes, m_playStart, m_playEnd);
				if (ret){
					VLOG("get play back stream falut!", EM_DEFAULT);
				}
				m_curOperate = EM_DEFAULT;
			}
			break;
		case EM_PAUSE:
			{
				int ret = m_playback->pausePlaybackStream(true);
				if (ret){
					VLOG("play back pause fault!", EM_DEFAULT);
				}
				m_curOperate = EM_DEFAULT;
			}
			break;
		case EM_CONTINUE:
			{
				int ret = m_playback->pausePlaybackStream(false);
				if (ret){
					VLOG("play back continue fault!", EM_DEFAULT);
				}
				m_curOperate = EM_DEFAULT;
			}
			break;
		case EM_STOP:
			{
				//check interface 
				if (!m_playback){
					VLOG("play back interface is NULL", EM_DEFAULT);
				}
				//stop stream
				int ret = m_playback->stopPlaybackStream();
				//disconnect the device
				IDeviceClient *pDevClient = NULL;
				m_playback->QueryInterface(IID_IDeviceClient, (void**)&pDevClient);
				if (!pDevClient){
					VLOG("get device interface fault!", EM_STOP);
				}
				pDevClient->closeAll();
				pDevClient->Release();

				m_curOperate = EM_DEFAULT;
			}
			break;
		default:
			{
				QEventLoop eventloop;
				QTimer::singleShot(10, &eventloop, SLOT(quit()));
				eventloop.exec();
			}
// 			msleep(10);
			break;
		}
	}
}

void PlayBackThread::getPlaybackInterface( void** playbackInterface )
{
	QString sAppPath = QCoreApplication::applicationDirPath();
	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConfFile;
	ConfFile.setContent(file);
	QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	for (int n = 0;n < itemList.count(); n++){
		QDomNode item = itemList.at(n);
		QString sItemName = item.toElement().attribute("vendor");
		if (sItemName == m_devInfo.m_sVendor){
			CLSID playbackTypeClsid = pcomString2GUID(item.toElement().attribute("clsid"));
			pcomCreateInstance(playbackTypeClsid, NULL, IID_IDeviceRemotePlayback, playbackInterface);
			if (NULL != *playbackInterface)
				break;
		}
	}
	file->close();
	delete file;
}

void PlayBackThread::FoundFile( QVariantMap evMap )
{
	QVariantMap::const_iterator it;
	QString fileinfo;
	fileinfo.append("{");
	for(it=evMap.begin(); it!=evMap.end(); it++){
		fileinfo.append("\\\"");
		fileinfo.append(it.key());
		fileinfo.append("\\\":\\\"");
		fileinfo.append(it.value().toString());
		fileinfo.append("\\\",");
	}
	fileinfo.append("}");
	fileinfo.replace(",}","}");
	m_fileMap.insert(QString("index_%1").arg(m_fileKey),fileinfo);

	m_fileKey++;
	if (m_fileKey == m_fileTotal){
		emit FoundFileToUiS(m_fileMap);
		m_fileKey = 0;
		m_fileMap.clear();
	}else if (m_fileKey == 100){
		emit FoundFileToUiS(m_fileMap);
		m_fileKey = 0;
		m_fileTotal -= 100;
		m_fileMap.clear();
	}
}

void PlayBackThread::RecFileSearchFinished( QVariantMap evMap )
{
	m_fileTotal = evMap.value("total").toInt();
	m_fileKey = 0;
	m_fileMap.clear();
	emit RecFileSearchFinishedToUiS(evMap);
}

void PlayBackThread::RecFileSearchFail( QVariantMap evMap )
{
	emit FileSearchFailToUiS(evMap);
}

void PlayBackThread::StateChange( QVariantMap evMap )
{
	emit StateChangeToUiS(evMap);
}

int PlayBackThread::getStreamInfo( int chl )
{
	if (chl == m_devInfo.m_uiChannelIdInDataBase)
		return 1;

	IChannelManager *pChannelManager=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void **)&pChannelManager);
	if (pChannelManager!=NULL){
		QVariantMap channelInfo=pChannelManager->GetChannelInfo(chl);
		m_devInfo.m_uiStreamId=channelInfo.value("stream").toInt();
		m_devInfo.m_uiChannelId=channelInfo.value("number").toInt();
		m_devInfo.m_sCameraname=channelInfo.value("name").toString();
		pChannelManager->Release();
		return 0;
	}
	return 1;
}

void PlayBackThread::setPlaybackWnd( RSubView* wnd )
{
	m_wndList.append(wnd);
}

bool PlayBackThread::removeRepeatWnd( QWidget *wndID )
{
	PlayIter it = m_playMap.begin();
	while (it != m_playMap.end()){
		if (it->wnd == wndID){
			uint temp = ~m_nChannels;
			temp |= 1<<(it.key() - 1);
			m_nChannels = ~temp;
			m_playMap.remove(it.key());
			return true;
		}
		++it;
	}
	return false;
}

void PlayBackThread::initCallBackFun()
{
	IEventRegister *pReg = NULL;
	m_playback->QueryInterface(IID_IEventRegister, (void**)&pReg);
	if (!pReg){
		qDebug()<<"register play back event error!";
		return;
	}
	pReg->registerEvent(QString("foundFile"), cbFoundFile, this);
	pReg->registerEvent(QString("recFileSearchFail"), cbRecFileSearchFail, this);
	pReg->registerEvent(QString("CurrentStatus"), cbStateChange, this);
	pReg->registerEvent(QString("recFileSearchFinished"), cbRecFileSearchFinished, this);
	pReg->registerEvent(QString("RecordStream"), cbRecordStream, this);
	pReg->Release();
}

int PlayBackThread::recordFrame( QVariantMap &evMap )
{
	int channel = evMap.value("channel").toInt();
	PlayIter iter = m_playMap.find(channel);
	if (m_playMap.end() == iter)
		return 1;
	iter->bufferManager->recordStream(evMap);
	return 0;
}

void PlayBackThread::action( QString act, BufferManager* pbuff )
{
	qDebug()<<"call "<<__FUNCTION__<<act<<" buff: "<<(int)pbuff;
	if ("StartPlay" == act){
		PlayIter it = m_playMap.begin();
		while (it != m_playMap.end()){
			if (it->bufferManager == pbuff){
				it->playManager->setParamter(pbuff, it->wnd);
				if (it->playManager->isRunning()){
					it->playManager->quit();
				}
				it->playManager->start();
			}
			++it;
		}
	}else if ("Pause" == act){
		m_curOperate = EM_PAUSE;
	}else if ("Continue" == act){
		m_curOperate = EM_CONTINUE;
	}else{
		//nothing
	}
}

bool PlayBackThread::getPlayInterface( QWidget* pwnd, void** playInterface )
{
	PlayIter it = m_playMap.begin();
	while (it != m_playMap.end()){
		if (it->wnd == pwnd){
			*playInterface = it->playManager;
			return true;
		}
		++it;
	}
	return false;
}

int PlayBackThread::setInfromation( QString evName, QVariantMap info )
{
	if ("CloseWnd" == evName){
		if (m_zoomWndList.isEmpty()){
			return 1;
		}
		for (int index = 0; index < m_zoomWndList.size(); ++index){
			QWidget *pWnd = (QWidget *)m_zoomWndList[index];
			PlayManager *playMgr = NULL;
			if (!getPlayInterface(pWnd, (void**)&playMgr)){
				continue;
			}
			playMgr->setZoomRect(QRect(1, 1, 1, 1), 0, 0);
			playMgr->setOriginRect(QRect(1, 1, 1, 1));
			playMgr->removeWnd(QString::number((quintptr)pWnd));
		}
		return 0;
	}

	QVariant wnd = info.value("CurWnd");
	QWidget *pWnd = (QWidget*)wnd.toUInt(), *lastWnd = NULL;
	PlayManager  *playMgr = NULL, *lastPlayMgr = NULL;
	if (!getPlayInterface(pWnd, (void**)&playMgr)){
		return 1;
	}

	if ("VedioZoom" == evName){
		m_susWnd = (QWidget *)info.value("SusWnd").toUInt();
		if (m_zoomWndList.contains(pWnd)){
			if (pWnd == m_zoomWndList.last()){
				return 0;
			}
			lastWnd = m_zoomWndList.last();
			getPlayInterface(lastWnd, (void**)&lastPlayMgr);
			lastPlayMgr->setOriginRect(QRect(1, 1, 1, 1));
			lastPlayMgr->removeWnd(QString::number((quintptr)lastWnd));
			playMgr->addWnd(m_susWnd, wnd.toString());

			m_zoomWndList.removeOne(pWnd);
			m_zoomWndList.append(pWnd);
		}else{
			if (!m_zoomWndList.isEmpty()){
				lastWnd = m_zoomWndList.last();
				getPlayInterface(lastWnd, (void**)&lastPlayMgr);
				lastPlayMgr->setOriginRect(QRect(1, 1, 1, 1));
				lastPlayMgr->removeWnd(QString::number((quintptr)lastWnd));
			}
			m_zoomWndList.append(pWnd);
			playMgr->addWnd(m_susWnd, wnd.toString());
		}
		playMgr->setZoomRect(info["ZoRect"].toRect(), info["Width"].toInt(), info["Height"].toInt());
	}else if ("ZoomRect" == evName){
		playMgr->setZoomRect(info["ZoRect"].toRect(), info["Width"].toInt(), info["Height"].toInt());
	}else if ("RectToOrigion" == evName){
		playMgr->setOriginRect(info["ZoRect"].toRect());
	}

	return 0;
}

int cbFoundFile(QString evName,QVariantMap evMap,void *pUser)
{
	if ("foundFile" == evName)
		((PlayBackThread*)pUser)->FoundFile(evMap);
	return 0;
}

int cbRecFileSearchFail(QString evName,QVariantMap evMap,void *pUser)
{
	if ("recFileSearchFail" == evName)
		((PlayBackThread*)pUser)->RecFileSearchFail(evMap);
	return 0;
}

int cbRecFileSearchFinished(QString evName,QVariantMap evMap,void *pUser)
{
	if ("recFileSearchFinished" == evName)
		((PlayBackThread*)pUser)->RecFileSearchFinished(evMap);
	return 0;
}

int cbStateChange(QString evName,QVariantMap evMap,void *pUser)
{
	if ("CurrentStatus" == evName)
		((PlayBackThread*)pUser)->StateChange(evMap);
	return 0;
}

int cbRecordStream(QString evName, QVariantMap evMap, void* pUser)
{
	if ("RecordStream" == evName)
		((PlayBackThread*)pUser)->recordFrame(evMap);
	return 0;
}