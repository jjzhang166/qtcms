#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include "OnvifProtocol_global.h"
extern "C"{
#include "nvp_define.h"
#include "minirtsp.h"
};

#pragma comment(lib, "rtsplibpro.lib")

void eventHook(int eventType, int lParam, void *rParam, void *customCtx);
void dataHook(void *pdata, uint32_t dataSize, uint32_t timestamp, char *dataType, void *customCtx);
class WorkerThread : public QObject
{
	Q_OBJECT

public:
	WorkerThread();
	~WorkerThread();
	ConnectStatus getCurrentStatus();
	void setDeviceInfo(const DeviceInfo& devInfo);
	void recFrameData(void* pdata, uint32_t size, uint32_t timestamp, char* datatype);
public slots:
	int ConnectToDevice();
	int Authority();
	int Disconnect();
	int GetLiveStream(int chl, int streamId);
	int PauseStream();
	int StopStream();
	int GetStreamCount(int &count);
	int GetStreamInfo(int nStreamId, QVariantMap& info);
	void setEventMap(const QMultiMap<QString,tagOnvifProInfo> &tEventMap);
	void PtzCtrl(NVP_PTZ_CMD cmd, int chl, int speed, bool bopen);
signals:
	void sigResultReady(int result);
private:
	QMultiMap<QString,tagOnvifProInfo> m_tEventMap;
	ConnectStatus m_enStatus;
	DeviceInfo m_tDeviceInfo;
	lpMINIRTSP m_rtspContext;
	lpNVP_INTERFACE m_nvpContext;
	stNVP_ARGS m_nvpArguments;
	stNVP_RTSP_STREAM m_nvpStreamUrl;
	stNVP_VENC_CONFIGS m_nvpStreamInfo;
};

#endif // WORKERTHREAD_H
