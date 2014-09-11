#ifndef __IREMOTE_MOTION_DETECTION_HEAD_FILE__
#define __IREMOTE_MOTION_DETECTION_HEAD_FILE__

#include "libpcom.h"

interface IRemoteMotionDetection : public IPComBase
{
	virtual int startMotionDetection() = 0;
	virtual int stopMotionDetection() = 0;
};

#endif