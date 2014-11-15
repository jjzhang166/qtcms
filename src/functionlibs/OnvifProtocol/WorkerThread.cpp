#include "WorkerThread.h"

#include <QDebug>

#define  CHECK_NVP_CONTEXT(context)							  	  \
	if (!context)                    							  \
	{															  \
		qDebug()<<__FUNCTION__<<__LINE__<<"rtsp_context is null"; \
		emit sigResultReady(1);									  \
		return 1;												  \
	}
	

#define  CHECK_RTSP_CONTEXT(context)							  \
	if (!context)                    							  \
	{															  \
		qDebug()<<__FUNCTION__<<__LINE__<<"rtsp_context is null"; \
		emit sigResultReady(1);									  \
		return 1;												  \
	}

#define CHECK_RESULT(ret)										 \
	if (ret)													 \
	{															 \
		qDebug()<<__FUNCTION__<<__LINE__<<"ret = "<<ret;		 \
		emit sigResultReady(1);									 \
		return 1;												 \
	}															 \
	else														 \
	{															 \
		qDebug()<<__FUNCTION__<<__LINE__<<"ret = 0";;			 \
		emit sigResultReady(0);									 \
		return 0;												 \
	}															 \

WorkerThread::WorkerThread()
	: QObject(),
	m_enStatus(CONNECT_STATUS_DISCONNECTED),
	m_rtspContext(NULL),
	m_nvpContext(NULL),
	m_nvpVerify(NULL)
{

}

WorkerThread::~WorkerThread()
{
	if (m_nvpContext)
	{
		NVP_ONVIF_delete(m_nvpContext);
		m_nvpContext = NULL;
	}
	if (m_rtspContext)
	{
		MINIRTSP_delete(m_rtspContext);
		m_rtspContext = NULL;
	}
}

int WorkerThread::ConnectToDevice()
{
	qDebug()<<__FUNCTION__<<__LINE__<<"start =======";

	m_nvpContext = NVP_ONVIF_new();
	if (!m_nvpContext)
	{
		emit sigResultReady(1);
		m_enStatus = CONNECT_STATUS_DISCONNECTED;
		qDebug()<<__FUNCTION__<<__LINE__<<"create nvp context error";
		return 1;
	}
	m_nvpArguments.thiz = (void *)m_nvpContext;
	strncpy(m_nvpArguments.ip, m_tDeviceInfo.sIpAddr.toLatin1().data(), m_tDeviceInfo.sIpAddr.size() + 1);
	m_nvpArguments.port = m_tDeviceInfo.vPorts["media"].toInt();
	strncpy(m_nvpArguments.username, m_tDeviceInfo.sUsername.toLatin1().data(), m_tDeviceInfo.sUsername.size() + 1);
	strncpy(m_nvpArguments.password, m_tDeviceInfo.sPassword.toLatin1().data(), m_tDeviceInfo.sPassword.size() + 1);
	m_nvpArguments.chn = 0;
	m_nvpStreamUrl.main_index = 0;
	m_nvpStreamUrl.sub_index = 1;

	//get rtsp url both main and sub stream
	m_nvpContext->GetRtspUri(&m_nvpArguments, &m_nvpStreamUrl);
	//create rtsp context, default for sub stream
	m_rtspContext = MINIRTSP_client_new(m_nvpStreamUrl.sub_uri, MINIRTSP_TRANSPORT_OVER_RTSP, m_tDeviceInfo.sUsername.toLatin1().data(), m_tDeviceInfo.sPassword.toLatin1().data(), true, true);
	if (!m_rtspContext)
	{
		emit sigResultReady(1);
		m_enStatus = CONNECT_STATUS_DISCONNECTED;
		NVP_ONVIF_delete(m_nvpContext);
		qDebug()<<__FUNCTION__<<__LINE__<<"create rtsp context fault";

		return 1;
	}
	//register event callback function
	MINIRTSP_set_event_hook(m_rtspContext, eventHook, this);

	int ret = MINIRTSP_connect(m_rtspContext);
	if (ret)
	{
		emit sigResultReady(1);
		m_enStatus = CONNECT_STATUS_DISCONNECTED;
		qDebug()<<__FUNCTION__<<__LINE__<<"connect error";

		return 1;
	}
	else
	{
		emit sigResultReady(0);
		m_enStatus = CONNECT_STATUS_CONNECTED;
		qDebug()<<__FUNCTION__<<__LINE__<<"connect success!!!";

		return 0;
	}
}

int WorkerThread::Authority()
{
	//create rtsp context, default for sub stream
	m_nvpVerify = MINIRTSP_client_new(m_nvpStreamUrl.sub_uri, MINIRTSP_TRANSPORT_OVER_RTSP, m_tDeviceInfo.sUsername.toLatin1().data(), m_tDeviceInfo.sPassword.toLatin1().data(), true, true);
	if (m_nvpVerify)
	{
		//this rtsp context is just for verify username and password
		MINIRTSP_delete(m_nvpVerify);
		m_nvpVerify = NULL;
		emit sigResultReady(0);
		return 0;
	}
	else
	{
		emit sigResultReady(1);
		return 1;
	}
}

int WorkerThread::Disconnect()
{
	m_enStatus = CONNECT_STATUS_DISCONNECTING;

	if (!m_rtspContext)
	{
		emit sigResultReady(0);
		m_enStatus = CONNECT_STATUS_DISCONNECTED;

		return 0;
	}
	int ret = MINIRTSP_disconnect(m_rtspContext);
	if (ret)
	{
		emit sigResultReady(1);
		m_enStatus = CONNECT_STATUS_DISCONNECTING;

		return 1;
	}
	MINIRTSP_delete(m_rtspContext);
	m_rtspContext = NULL;
	NVP_ONVIF_delete(m_nvpContext);
	m_nvpContext = NULL;

	emit sigResultReady(0);
	m_enStatus = CONNECT_STATUS_DISCONNECTED;

	return 0;
}

ConnectStatus WorkerThread::getCurrentStatus()
{
	return m_enStatus;
}

void WorkerThread::setDeviceInfo( const DeviceInfo& devInfo )
{
	m_tDeviceInfo = devInfo;
}

void WorkerThread::setEventMap(const QMultiMap<QString,tagOnvifProInfo> &tEventMap)
{
	m_tEventMap = tEventMap;
}

int WorkerThread::GetLiveStream( int chl, int streamId )
{
	CHECK_RTSP_CONTEXT(m_rtspContext);
	if (MAIN_STREAM == streamId)
	{
		//release old context and switch to main stream
		MINIRTSP_delete(m_rtspContext);
		m_rtspContext = MINIRTSP_client_new(m_nvpStreamUrl.main_uri, MINIRTSP_TRANSPORT_OVER_RTSP, m_tDeviceInfo.sUsername.toLatin1().data(), m_tDeviceInfo.sPassword.toLatin1().data(), true, true);
		if (m_rtspContext)
		{
			//register event callback function
			MINIRTSP_set_event_hook(m_rtspContext, eventHook, this);
		}
		else
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"rtsp context has release, please reconnect and try again";
			emit sigResultReady(1);
			return 1;
		}
	}

	MINIRTSP_set_data_hook(m_rtspContext, (fMINIRTSP_DATA_HOOK)dataHook, this);
	int ret = MINIRTSP_play(m_rtspContext);
	CHECK_RESULT(ret);
}

int WorkerThread::PauseStream()
{
	CHECK_RTSP_CONTEXT(m_rtspContext);
	int ret = MINIRTSP_pause(m_rtspContext);
	CHECK_RESULT(ret);
}

int WorkerThread::StopStream()
{
	CHECK_RTSP_CONTEXT(m_rtspContext);
	int ret = MINIRTSP_disconnect(m_rtspContext);
	CHECK_RESULT(ret);
}

int WorkerThread::GetStreamCount( int *count )
{
	CHECK_NVP_CONTEXT(m_nvpContext);
	int ret = m_nvpContext->GetVideoEncoderConfigs(&m_nvpArguments, &m_nvpStreamInfo);
	*count = m_nvpStreamInfo.nr;
	CHECK_RESULT(ret);
}

int WorkerThread::GetStreamInfo( int nStreamId, QVariantMap& info )
{
	if (nStreamId < 0 || nStreamId >= m_nvpStreamInfo.nr || m_nvpStreamInfo.nr <= 0)
	{
		emit sigResultReady(1);
		return 1;
	}
	info.insert("index", m_nvpStreamInfo.entry[nStreamId].index);
	info.insert("name", QString(m_nvpStreamInfo.entry[nStreamId].name));
	info.insert("enc_type", m_nvpStreamInfo.entry[nStreamId].enc_type);
	info.insert("width", m_nvpStreamInfo.entry[nStreamId].width);
	info.insert("height", m_nvpStreamInfo.entry[nStreamId].height);
	info.insert("fps", m_nvpStreamInfo.entry[nStreamId].enc_fps);
	return 0;
}

void WorkerThread::recFrameData( void* pdata, uint32_t size, uint32_t timestamp, char* datatype )
{
	QVariantMap tStreamInfo;
	stMINIRTSP_DATA_PROPERTY frameInfo;
	MINIRTSP_lookup_data(m_rtspContext, datatype, &frameInfo);

	tStreamInfo.insert("channel", frameInfo.channel);
	tStreamInfo.insert("pts", timestamp);
	tStreamInfo.insert("length", size);
	tStreamInfo.insert("data", (quintptr)pdata);

	if (!strcmp("h264", frameInfo.dataType))
	{
		//vedio
		tStreamInfo.insert("frametype", TYPE_VEDIO);
		tStreamInfo.insert("width", frameInfo.h264.width);
		tStreamInfo.insert("height", frameInfo.h264.height);
		tStreamInfo.insert("vcodec", "H264");
	}
	else
	{
		//audio
		tStreamInfo.insert("frametype", TYPE_AUDIO);
		tStreamInfo.insert("samplerate", frameInfo.g711.sampleRate);
		tStreamInfo.insert("samplewidth", frameInfo.g711.sampleSize);
		tStreamInfo.insert("audiochannel", frameInfo.g711.channel);
		tStreamInfo.insert("acodec", "g711");
	}
	tagOnvifProInfo tProInfo=m_tEventMap.value("LiveStream");
	if (tProInfo.proc)
	{
		tProInfo.proc(QString("LiveStream"),tStreamInfo,tProInfo.pUser);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"LiveStream is not register";
	}
}

void WorkerThread::PtzCtrl( NVP_PTZ_CMD cmd, int chl, int speed, bool bopen )
{
	Q_UNUSED(chl);
	if (!m_nvpContext)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"nvp context is null";
		return;
	}
	stNVP_PTZ_CONTROL ptzCtrl;
	memset(&ptzCtrl, 0, sizeof(stNVP_PTZ_CONTROL));
	ptzCtrl.index = 0;
	ptzCtrl.cmd = cmd;
	ptzCtrl.speed = bopen ? (speed + 1)*0.125 : 0.0f;
	ptzCtrl.step = 1;
	ptzCtrl.repeat = false;
	m_nvpContext->ControlPTZ(&m_nvpArguments, &ptzCtrl);
}

void eventHook( int eventType, int lParam, void *rParam, void *customCtx )
{

}

void dataHook( void *pdata, uint32_t dataSize, uint32_t timestamp, char *dataType, void *customCtx )
{
	((WorkerThread*)customCtx)->recFrameData(pdata, dataSize, timestamp, dataType);
}
