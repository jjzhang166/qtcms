#include "autoSearchDevice.h"


autoSearchDevice::autoSearchDevice(void)
{
}


autoSearchDevice::~autoSearchDevice(void)
{
}

void autoSearchDevice::startSearch()
{

}

void autoSearchDevice::stopSearch()
{

}

void autoSearchDevice::run()
{
	bool bRunStop=false;
	tagAutoSearchDeviceStep tRunStep=AutoSearchDeviceStep_Start;
	while(bRunStop==false){
		switch(tRunStep){
		case AutoSearchDeviceStep_Start:{
			startVendorSearch();
			tRunStep=AutoSearchDeviceStep_NetworkConfig;
										}
										break;
		case AutoSearchDeviceStep_NetworkConfig:{
			if (getNetworkConfig())
			{
				tRunStep=AutoSearchDeviceStep_Default;
			}else{
				tRunStep=AutoSearchDeviceStep_End;
			}
												}
												break;
		case AutoSearchDeviceStep_Default:{

										  }
										  break;
		case AutoSearchDeviceStep_End:{

									  }
									  break;
		}
	}
}

void autoSearchDevice::startVendorSearch()
{

}

bool autoSearchDevice::getNetworkConfig()
{
	return false;
}
