#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include "OnvifProtocol_global.h"
#include <QStringList>
extern "C"{
#include "nvp_define.h"
#include "minirtsp.h"
};

#pragma comment(lib, "rtsplibpro.lib")
void authorityEventHook(int nEvent, unsigned int lparam, unsigned int rparam, void *custom /* top-level-param */);
void eventHook(int eventType, int lParam, void *rParam, void *customCtx);
void dataHook(void *pdata, unsigned int dataSize, unsigned int timestamp, int dataType, void *customCtx);
void nvpMDEventHook(int nEvent, unsigned int lparam, unsigned int rparam, void *custom);
class WorkerThread : public QObject
{
	Q_OBJECT

public:
	WorkerThread();
	~WorkerThread();
	ConnectStatus getCurrentStatus();
	void setDeviceInfo(const DeviceInfo& devInfo);
	void recEventHook(int eventType,void *rParam);
	void recAuthorityEventHook(int eventType,void *rParam);
	void recNvpMDEventHook(int eventType, void *rParam);
	void recFrameData(void* pdata, unsigned int size, unsigned int timestamp, int datatype);
	void registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);
public slots:
	int ConnectToDevice(int *ret);
	int Authority(int *ret);
	int Disconnect(int *ret);
	int GetLiveStream(int chl, int streamId, int *ret);
	int PauseStream(bool pause, int *ret);
	int StopStream(int *ret);
	int GetStreamCount(int *count);
	int GetStreamInfo(int nStreamId, QVariantMap& info, int *ret);
	void setEventMap(const QMultiMap<QString,tagOnvifProInfo> &tEventMap);
	int PtzCtrl(NVP_PTZ_CMD cmd, int chl, int speed, bool bopen, int *ret);
	void MotionDetection(bool bEnable, int *result);
private:
	QMultiMap<QString,tagOnvifProInfo> m_tEventMap;
	ConnectStatus m_enStatus;
	DeviceInfo m_tDeviceInfo;
	lpMINIRTSP m_rtspContext;
	lpMINIRTSP m_nvpVerify;
	lpNVP_INTERFACE m_nvpContext;
	stNVP_ARGS m_nvpArguments;
	stNVP_RTSP_STREAM m_nvpStreamUrl;
	stNVP_VENC_CONFIGS m_nvpStreamInfo;
	stNVP_SUBSCRIBE m_nvpSubscribe;
	volatile bool m_bIgnoreEvent;
	QStringList m_sEventList;

	volatile bool m_bMotionDetect;
};

#endif // WORKERTHREAD_H
