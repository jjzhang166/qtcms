#ifndef __AUDIODEVICE_CLASS_HEADER__
#define __AUDIODEVICE_CLASS_HEADER__

class CAudioDevice
{
public:
	CAudioDevice();
	virtual ~CAudioDevice();

public:
	virtual int SetMainWnd(HWND hWnd) = 0;
	virtual int SetAudioParam(int nChannel,int nSampleRate,int nSampleWidth) = 0;
	virtual int SetVolume(int nPersent) = 0;
	virtual int PlayBuffer(char *pBuffer,int nSize) = 0;
	virtual int Stop() = 0;
protected:
private:
};

#endif