#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env_common.h"
#include "generic.h"
#include "sock.h"
#include "ticker.h"
#include "app_debug.h"

static int _get_system_information(lpNVP_DEV_INFO info)
{	
	strcpy(info->manufacturer, "JIUAN");
	strcpy(info->devname, "ONVIFDemo");
	strcpy(info->model, "ONVIFD");
	strcpy(info->sn, "OD100");
	strcpy(info->firmware, "V1.0");
	strcpy(info->sw_version, "V1.0");
	strcpy(info->sw_builddate, "2014-7-1 12:00:00");
	strcpy(info->hw_version, "V1.0");
	strcpy(info->hwid, "HW000");

	return 0;
}

static int _get_date_time(lpNVP_SYS_TIME systime)
{
	time_t t_now;
	struct tm *ptm;
	struct tm tm_local, tm_gm;
	
	systime->ntp_enable = false;
	strcpy(systime->ntp_server, "");
	systime->tzone = -800;

	time(&t_now);
	localtime_c(&t_now, &tm_local);
	gmtime_c(&t_now, &tm_gm);

	ptm = &tm_local;
	systime->local_time.date.year = ptm->tm_year;
	systime->local_time.date.month = ptm->tm_mon;
	systime->local_time.date.day = ptm->tm_mday;
	systime->local_time.time.hour = ptm->tm_hour;
	systime->local_time.time.minute = ptm->tm_min;
	systime->local_time.time.second = ptm->tm_sec;
		
	ptm = &tm_gm;
	systime->gm_time.date.year = ptm->tm_year;
	systime->gm_time.date.month = ptm->tm_mon;
	systime->gm_time.date.day = ptm->tm_mday;
	systime->gm_time.time.hour = ptm->tm_hour;
	systime->gm_time.time.minute = ptm->tm_min;
	systime->gm_time.time.second = ptm->tm_sec;
	
	return 0;
}

static int _set_date_time(lpNVP_SYS_TIME systime)
{	
	return -1;
}


static int _get_interface(lpNVP_ETHER_CONFIG ether)
{
	char ethername[16];
	char szip[32], szmask[32], szmac[32];
	char *ptr;
	int port;

	ptr =  getenv("ONVIF_PORT");
	if(ptr)
		port  = atoi(ptr);
	else
		port = 8080;

	ptr =  getenv("DEF_ETHER");
	if (ptr )
		strcpy(ethername, ptr);
	else
		strcpy(ethername, "eth0");
	SOCK_get_ether_ip(ethername, szip, szmask, szmac);
	ether->dhcp = false;
	NVP_IP_INIT_FROM_STRING(ether->ip, szip);
	NVP_IP_INIT_FROM_STRING(ether->netmask, szmask);
	NVP_IP_INIT_FROM_STRING(ether->gateway, "192.168.56.1");
	NVP_IP_INIT_FROM_STRING(ether->dns1, "8.8.8.8");
	NVP_IP_INIT_FROM_STRING(ether->dns2, "8.8.4.4");
	
	NVP_MAC_INIT_FROM_STRING(ether->mac, szmac);

	ether->http_port = port;
	ether->rtsp_port = port;

	return 0;
}

static int _set_interface(lpNVP_ETHER_CONFIG ether)
{
	return -1;
}

static int _get_color(lpNVP_COLOR_CONFIG color, int chn)
{
	color->brightness = 80;
	color->contrast = 80;
	color->hue = 80;
	color->saturation = 50;
	color->sharpness = 50;
	
	return 0;
}


static int _set_color(lpNVP_COLOR_CONFIG color, int chn)
{
	return -1;
}

static int _get_image_option(lpNVP_IMAGE_OPTIONS image, int chn)
{
	// FIX me
	image->brightness.min = 0;
	image->brightness.max = 255;
	image->saturation.min = 0;
	image->saturation.max = 255;
	image->sharpness.min = 0;
	image->sharpness.max = 255;
	image->contrast.min = 0;
	image->contrast.max = 255;
	image->hue.min = 0;
	image->hue.max = 255;

	image->ircut_mode.nr = 3;
	image->ircut_mode.list[0] = 0;
	image->ircut_mode.list[1] = 1;
	image->ircut_mode.list[2] = 2;

	return 0;
}

static int _get_image(lpNVP_IMAGE_CONFIG image, int chn)
{
	image->color.brightness = 50;
	image->color.contrast = 50;
	image->color.hue = 50;
	image->color.saturation = 50;
	image->color.sharpness = 50;

	image->ircut.control_mode = 0;
	image->ircut.ircut_mode = 0;

	image->wdr.enabled = false;
	image->wdr.WDRStrength = 0;

	image->manual_sharp.enabled = false;
	image->manual_sharp.sharpnessLevel = 0;

	image->d3d.enabled = false;
	image->d3d.denoise3dStrength = 0;

	_get_color(&image->color, chn);

	_get_image_option(&image->option, chn);
	
	return 0;
}

static int _set_image(lpNVP_IMAGE_CONFIG image, int chn)
{
	return -1;
}

static int _get_video_source(lpNVP_V_SOURCE src, int chn)
{
	src->resolution.width = 1920;
	src->resolution.height = 1080;	

	src->fps = 30;

	_get_image(&src->image, chn);
	
	return 0;
}

static int _set_video_source(lpNVP_V_SOURCE src, int chn)
{	
	return -1;
}


static int _get_video_input_conf(lpNVP_VIN_CONFIG vin, int chn)
{
	vin->rect.nX = 0;
	vin->rect.nY = 0;
	vin->rect.width = 1920;
	vin->rect.height = 1080;	

	vin->rotate.enabled = false;
	
	vin->rotate.degree = 0;
	return 0;
}

static int _set_video_input_conf(lpNVP_VIN_CONFIG vin, int chn)
{	
	return -1;
}

static int _get_video_encode_option(lpNVP_VENC_OPTIONS venc, int chn)
{
	// FIM Me
	venc->enc_fps.min = 3;
	venc->enc_fps.max = 30;

	venc->enc_gov.min = 1;
	venc->enc_gov.max = 30;

	venc->enc_interval.min = 1;
	venc->enc_interval.max = 1;
	
	venc->enc_quality.min = 0;
	venc->enc_quality.max = 4;

	if (venc->index == 0) {		
		venc->enc_bps.min  = 512;
		venc->enc_bps.max = 5000;

		venc->resolution_nr = 5;
		NVP_SET_SIZE(&venc->resolution[0], 640, 480);
		NVP_SET_SIZE(&venc->resolution[1], 720, 480);
		NVP_SET_SIZE(&venc->resolution[2], 720, 576);
		NVP_SET_SIZE(&venc->resolution[3], 1024, 768);
		NVP_SET_SIZE(&venc->resolution[4], 1280, 720);
	} else if (venc->index == 1) {
		venc->enc_bps.min  = 32;
		venc->enc_bps.max = 1500;

		venc->resolution_nr = 5;
		NVP_SET_SIZE(&venc->resolution[0], 320, 240);
		NVP_SET_SIZE(&venc->resolution[1], 352, 240);
		NVP_SET_SIZE(&venc->resolution[2], 352, 288);
		NVP_SET_SIZE(&venc->resolution[3], 640, 360);
		NVP_SET_SIZE(&venc->resolution[4], 640, 480);
	} else {
		venc->enc_bps.min  = 32;
		venc->enc_bps.max = 512;

		venc->resolution_nr = 7;
		NVP_SET_SIZE(&venc->resolution[0], 160, 90);
		NVP_SET_SIZE(&venc->resolution[1], 160, 120);
		NVP_SET_SIZE(&venc->resolution[2], 176, 144);
		NVP_SET_SIZE(&venc->resolution[3], 320, 180);
		NVP_SET_SIZE(&venc->resolution[4], 320, 240);
		NVP_SET_SIZE(&venc->resolution[5], 352, 240);
		NVP_SET_SIZE(&venc->resolution[6], 352, 288);
	}

	venc->enc_profile_nr = 1;
	venc->enc_profile[0] = NVP_H264_PROFILE_BASELINE;

	return 0;	
}

static int _get_video_encode(lpNVP_VENC_CONFIG venc, int chn)
{
	venc->width = 1280;
	venc->height =  720;
	
	venc->enc_bps = 3000;
	venc->enc_fps = 25;
	venc->enc_gov = 25;
	venc->enc_interval = 1;
	venc->quant_mode = NVP_QUANT_CBR;
	
	venc->enc_type = NVP_VENC_H264;
	// FIX me
	venc->enc_profile = NVP_H264_PROFILE_BASELINE;
	if (venc->index == 0)
		venc->user_count = 4;
	else
		venc->user_count = 6;
	
	venc->option.index = venc->index;
	strcpy(venc->option.token, venc->token);
	strcpy(venc->option.enc_token, venc->enc_token);
	_get_video_encode_option(&venc->option, chn);

	return 0;
}

static int _set_video_encode(lpNVP_VENC_CONFIG venc, int chn)
{
	return -1;
}

static int _get_audio_input(lpNVP_AIN_CONFIG ain, int chn)
{
	return 0;
}

static int _set_audio_input(lpNVP_AIN_CONFIG ain, int chn)
{
	return -1;
}


static int _get_audio_encode(lpNVP_AENC_CONFIG aenc, int chn)
{
	//FIX me
	aenc->channel = 1;
	aenc->enc_type = NVP_AENC_G711;
	aenc->sample_size = 8;
	aenc->sample_rate = 8000;
	
	aenc->user_count = 2;
	return 0;
}

static int _set_audio_encode(lpNVP_AENC_CONFIG aenc, int chn)
{
	return -1;
}

static int _get_motion_detection(lpNVP_MD_CONFIG md, int chn)
{
	md->type = NVP_MD_TYPE_GRID;

	md->grid.columnGranularity = 22;
	md->grid.rowGranularity = 15;
	md->grid.sensitivity = 80;
	memset(md->grid.granularity, 0xff, sizeof(md->grid.granularity));
	//FIM me
	md->grid.threshold = 5;
	// FIX me
	md->delay_off_alarm = 300;
	md->delay_on_alarm = 200;
	
	return 0;
}

static int _set_motion_detection(lpNVP_MD_CONFIG md, int chn)
{	
	return 0;
}

static int _get_video_analytic(lpNVP_VAN_CONFIG van, int chn)
{
	return 0;
}

static int _set_video_analytic(lpNVP_VAN_CONFIG van, int chn)
{
	return 0;
}


static int _get_ptz(lpNVP_PTZ_CONFIG ptz, int chn)
{
	return 0;
}

static int _set_ptz(lpNVP_PTZ_CONFIG ptz, int chn)
{
	return 0;
}

static int _get_profile(lpNVP_PROFILE_CHN profile)
{
	int i;
	// FIX me
	profile->profile_nr = 2;
	profile->venc_nr = 2;
	profile->aenc_nr = 1;
	//
	for (i = 0; i < profile->venc_nr; i++) {
		_get_video_encode(&profile->venc[i], profile->index);
	}
	for (i = 0; i < profile->aenc_nr; i++) {
		_get_audio_encode(&profile->aenc[i], profile->index);
	}
	_get_video_source(&profile->v_source, profile->index);
	for (i = 0; i < profile->vin_conf_nr; i++) {
		_get_video_input_conf(&profile->vin[i], profile->index);
	}
	_get_audio_input(&profile->ain, profile->index);
	_get_ptz(&profile->ptz, profile->index);
	_get_video_analytic(&profile->van, profile->index);
	_get_motion_detection(&profile->md, profile->index);

	return 0;
}

static int _set_profile(lpNVP_PROFILE_CHN profile)
{
	int i;
	int ret = 0;
	//
	for (i = 0; i < profile->venc_nr; i++) {
		if (_set_video_encode(&profile->venc[i], profile->index) < 0)
			ret = -1;
	}
	for (i = 0; i < profile->aenc_nr; i++) {
		if (_set_audio_encode(&profile->aenc[i], profile->index) < 0)
			ret = -1;
	}
	if (_set_video_source(&profile->v_source, profile->index) < 0)
		ret = -1;
	for (i = 0; i < profile->vin_conf_nr; i++) {
		if (_set_video_input_conf(&profile->vin[i], profile->index) < 0)
			ret = -1;
	}
	if (_set_audio_input(&profile->ain, profile->index) < 0)
		ret = -1;
	if (_set_ptz(&profile->ptz, profile->index) < 0)
		ret = -1;
	if (_set_video_analytic(&profile->van, profile->index) < 0)
		ret = -1;
	if (_set_motion_detection(&profile->md, profile->index) < 0)
		ret = -1;

	return ret;
}

static int _get_profiles(lpNVP_PROFILE profiles)
{
	int i;

	profiles->chn = NVP_MAX_CH;
	//
	for ( i = 0; i < profiles->chn; i++) {
		_get_profile(&profiles->profile[i]);
	}
	return 0;
}

static int _set_profiles(lpNVP_PROFILE profiles)
{
	int i;
	int ret = 0;
	//
	for ( i = 0; i < profiles->chn; i++) {
		if(_set_profile(&profiles->profile[i]) < 0)
			ret = -1;
	}
	return ret;
}


static int _get_all(lpNVP_ENV env)
{
	_get_system_information(&env->devinfo);
	_get_date_time(&env->systime);
	_get_interface(&env->ether);
	_get_profiles(&env->profiles);

	return 0;
}

static int _set_all(lpNVP_ENV env)
{
	int ret = 0;

	if (_set_date_time(&env->systime) < 0)
		ret = -1;
	if (_set_interface(&env->ether) < 0)
		ret = -1;
	if (_set_profiles(&env->profiles) < 0)
		ret = -1;

	return ret;	
}

/********************************************************************************
* system command interfaces
*********************************************************************************/
static void _cmd_system_boot(long l, void *r)
{
	TICKER_del_task(_cmd_system_boot);
	APP_TRACE("system reboot now...");
	exit(0);
}

static int _cmd_ptz(lpNVP_CMD cmd, const char *module, int keyid)
{
	const char *ptz_cmd_name[] = 
	{
		"PTZ_CMD_UP",
		"PTZ_CMD_DOWN",
		"PTZ_CMD_LEFT",
		"PTZ_CMD_RIGHT",
		"PTZ_CMD_LEFT_UP",
		"PTZ_CMD_RIGHT_UP",
		"PTZ_CMD_LEFT_DOWN",
		"PTZ_CMD_RIGHT_DOWN",
		"PTZ_CMD_AUTOPAN",
		"PTZ_CMD_IRIS_OPEN",
		"PTZ_CMD_IRIS_CLOSE",
		"PTZ_CMD_ZOOM_IN",
		"PTZ_CMD_ZOOM_OUT",
		"PTZ_CMD_FOCUS_FAR",
		"PTZ_CMD_FOCUS_NEAR",
		"PTZ_CMD_STOP",
		"PTZ_CMD_WIPPER_ON",
		"PTZ_CMD_WIPPER_OFF",
		"PTZ_CMD_LIGHT_ON",
		"PTZ_CMD_LIGHT_OFF",
		"PTZ_CMD_POWER_ON",
		"PTZ_CMD_POWER_OFF",
		"PTZ_CMD_GOTO_PRESET",
		"PTZ_CMD_SET_PRESET",
		"PTZ_CMD_CLEAR_PRESET",
		"PTZ_CMD_TOUR",
	};

	APP_TRACE("%s(%d)", ptz_cmd_name[cmd->ptz.cmd], cmd->ptz.cmd);
	switch(cmd->ptz.cmd) 
	{
		case NVP_PTZ_CMD_LEFT:
			break;
		case NVP_PTZ_CMD_RIGHT:
			break;
		case NVP_PTZ_CMD_UP:
			break;
		case NVP_PTZ_CMD_DOWN:
			break;
		case NVP_PTZ_CMD_ZOOM_IN:
			break;
		case NVP_PTZ_CMD_ZOOM_OUT:
			break;
		case NVP_PTZ_CMD_SET_PRESET:
			break;
		case NVP_PTZ_CMD_GOTO_PRESET:
			break;
		case NVP_PTZ_CMD_CLEAR_PRESET:
			break;
		case NVP_PTZ_CMD_STOP:
			break;
		default:
			break;
	}

	return 0;
}

/********************************************************************************
* external interfaces
*********************************************************************************/

int NVP_env_load(lpNVP_ENV env, const char *module, int keyid)
{
#ifdef WIN32
	return 0;
#else
	char temp[512];
	char *ptr = NULL, *pbuf;
	char *saveptr = NULL;
	int ret;
	int chn, id;

	chn = keyid/100;
	id = keyid%100;
	strncpy(temp, module, 512);
	pbuf = temp;
	while((ptr = strtok_r(pbuf, OM_AND, &saveptr)) != NULL)
	{
		if (OM_MATCH(ptr, OM_ALL)) {
			ret = _get_all(env);
			break;
		}else if (OM_MATCH(ptr, OM_PROFILE)) {
			ret = _get_profile(&env->profiles.profile[chn]);
		}else if (OM_MATCH(ptr, OM_PROFILES)) {
			ret = _get_profiles(&env->profiles);
		}else if (OM_MATCH(ptr, OM_INFO)) {
			ret = _get_system_information(&env->devinfo);
		}else if (OM_MATCH(ptr, OM_DTIME)) {
			ret = _get_date_time(&env->systime);
		}else if (OM_MATCH(ptr, OM_NET)) {
			ret = _get_interface(&env->ether);
		}  else if (OM_MATCH(ptr, OM_VENC)) {
			ret = _get_video_encode(&env->profiles.profile[chn].venc[id], chn);
		}  else if (OM_MATCH(ptr, OM_VSRC)) {
			ret = _get_video_source(&env->profiles.profile[chn].v_source, chn);
		}  else if (OM_MATCH(ptr, OM_VINC)) {
			ret = _get_video_input_conf(&env->profiles.profile[chn].vin[id], chn);
		}  else if (OM_MATCH(ptr, OM_AENC)) {
			ret = _get_audio_encode(&env->profiles.profile[chn].aenc[id], chn);
		}  else if (OM_MATCH(ptr, OM_AIN)) {
			ret = _get_audio_input(&env->profiles.profile[chn].ain, chn);
		}  else if (OM_MATCH(ptr, OM_COLOR)) {
			ret = _get_color(&env->profiles.profile[chn].v_source.image.color, chn);			
		}  else if (OM_MATCH(ptr, OM_IMG)) {
			ret = _get_image(&env->profiles.profile[chn].v_source.image, chn);
		}  else if (OM_MATCH(ptr, OM_MD)) {
			ret = _get_motion_detection(&env->profiles.profile[chn].md, chn);
		}  else if (OM_MATCH(ptr, OM_PTZ)) {
			ret = _get_ptz(&env->profiles.profile[chn].ptz, chn);
		} else {
			APP_TRACE("unknown env module: %s", ptr);
		}
		//
		pbuf = NULL;
	}

	return 0;
#endif
}

int NVP_env_save(lpNVP_ENV env, const char *module, int keyid)
{
#ifdef WIN32
	return 0;
#else
	char temp[512];
	char *ptr = NULL, *pbuf;
	char *saveptr = NULL;
	int ret;
	int f_ret = 0;
	int chn, id;

	chn = keyid / 100;
	id = keyid % 100;
	strncpy(temp, module, 512);
	pbuf = temp;
	while((ptr = strtok_r(pbuf, OM_AND, &saveptr)) != NULL)
	{
		if (OM_MATCH(ptr, OM_ALL)) {
			ret = _set_all(env);
			break;
		}else if (OM_MATCH(ptr, OM_PROFILE)) {
			ret = _set_profile(&env->profiles.profile[chn]);
		}else if (OM_MATCH(ptr, OM_PROFILES)) {
			ret = _set_profiles(&env->profiles);
		}else if (OM_MATCH(ptr, OM_INFO)) {
			//
		}else if (OM_MATCH(ptr, OM_DTIME)) {
			ret = _set_date_time(&env->systime);
		}else if (OM_MATCH(ptr, OM_NET)) {
			ret = _set_interface(&env->ether);
		}  else if (OM_MATCH(ptr, OM_VENC)) {
			ret = _set_video_encode(&env->profiles.profile[chn].venc[id], chn);
		}  else if (OM_MATCH(ptr, OM_VSRC)) {
			ret = _set_video_source(&env->profiles.profile[chn].v_source, chn);
		}  else if (OM_MATCH(ptr, OM_VINC)) {
			ret = _set_video_input_conf(&env->profiles.profile[chn].vin[id], chn);
		}  else if (OM_MATCH(ptr, OM_AENC)) {
			ret = _set_audio_encode(&env->profiles.profile[chn].aenc[id], chn);
		}  else if (OM_MATCH(ptr, OM_AIN)) {
			ret = _set_audio_input(&env->profiles.profile[chn].ain, chn);
		}  else if (OM_MATCH(ptr, OM_COLOR)) {
			ret = _set_color(&env->profiles.profile[chn].v_source.image.color, chn);			
		}  else if (OM_MATCH(ptr, OM_IMG)) {
			ret = _set_image(&env->profiles.profile[chn].v_source.image, chn);
		}  else if (OM_MATCH(ptr, OM_MD)) {
			ret = _set_motion_detection(&env->profiles.profile[chn].md, chn);
		}  else if (OM_MATCH(ptr, OM_PTZ)) {
			ret = _set_ptz(&env->profiles.profile[chn].ptz, chn);
		} else {
			APP_TRACE("unknown env module: %s", ptr);
			f_ret = -1;
		}

		if (ret < 0)
			f_ret = -1;

		//
		pbuf = NULL;
	}

	return f_ret;
#endif
}

int NVP_env_cmd(lpNVP_CMD cmd, const char *module, int keyid)
{
#ifdef WIN32
	return 0;
#else
	char temp[512];
	char *ptr = NULL, *pbuf;
	char *saveptr = NULL;
	int ret;
	int f_ret = 0;
	int chn, id;

	chn = keyid / 100;
	id = keyid % 100;
	strncpy(temp, module, 512);
	pbuf = temp;
	while((ptr = strtok_r(pbuf, OM_AND, &saveptr)) != NULL)
	{
		if (OM_MATCH(ptr, OM_REBOOT)) {
			ret = TICKER_add_task(_cmd_system_boot, 1, false);
		}else if (OM_MATCH(ptr, OM_SYS_RESET)) {
			APP_TRACE("unknown env module: %s", ptr);
		}else if (OM_MATCH(ptr, OM_PTZ)) {
			ret = _cmd_ptz(cmd, module, keyid);
		} else {
			APP_TRACE("unknown env module: %s", ptr);
			f_ret = -1;
		}

		if (ret < 0)
			f_ret = -1;

		//
		pbuf = NULL;
	}

	return f_ret;
#endif
}


