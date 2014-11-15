#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env_common.h"
#include "generic.h"
#include "sock.h"
#include "ticker.h"
#include "app_debug.h"

#include "environment.h"

#ifdef _IPC_TESTER

static int _get_system_information(lpNVP_DEV_INFO info)
{	
	int maj, min, rev;
	char ver_ext[32];
	
	strcpy(info->manufacturer, "JIUAN");
	
	g_pstSysEnv->GetDevName(g_pstSysEnv,info->devname);
	g_pstSysEnv->GetDevModel(g_pstSysEnv, info->model);
	strcpy(info->sn, "00000000");
	g_pstSysEnv->GetSWVersion(g_pstSysEnv, &maj, &min, &rev, ver_ext);	
	sprintf(info->firmware, "%d.%d.%d%s", maj, min, rev, ver_ext);
	sprintf(info->sw_version, "%d.%d.%d%s", maj, min, rev, ver_ext);
	g_pstSysEnv->GetHWVersion(g_pstSysEnv, &maj, &min, &rev, ver_ext);	
	sprintf(info->hw_version, "%d.%d.%d%s", maj, min, rev, ver_ext);
	strcpy(info->sw_builddate, "2014-7-1 12:00:00");
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
	localtime_r(&t_now, &tm_local);
	gmtime_r(&t_now, &tm_gm);

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
	char szip[32], szmask[32], szmac[32], szgateway[32];
	char *ptr;
	int port;

	 ether->http_port = (uint32_t)g_pstSysEnv->GetSpookPort(g_pstSysEnv);
	 ether->rtsp_port = ether->http_port;

	 g_pstSysEnv->GetStaticIp(g_pstSysEnv, szip);
	 g_pstSysEnv->GetNetmask(g_pstSysEnv, szmask);
	 g_pstSysEnv->GetGateway(g_pstSysEnv, szgateway);
	 g_pstSysEnv->GetHWAddr(g_pstSysEnv, szmac);
	 
	ether->dhcp = false;
	NVP_IP_INIT_FROM_STRING(ether->ip, szip);
	NVP_IP_INIT_FROM_STRING(ether->netmask, szmask);
	NVP_IP_INIT_FROM_STRING(ether->gateway, szgateway);
	NVP_IP_INIT_FROM_STRING(ether->dns1, "8.8.8.8");
	NVP_IP_INIT_FROM_STRING(ether->dns2, "8.8.4.4");
	
	NVP_MAC_INIT_FROM_STRING(ether->mac, szmac);

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
	int n;
	int protocal = 0;
	SysenvIPCPrivData priv;
	int fps_min = 1, fps_max  = 30;
	int bps_min = 32, bps_max = 5000;
	int resolution_nr = 1;
	int width[16] = {320, };
	int height[16] = {240, };

	protocal = g_pstSysEnv->GetIPCamProtocolname(g_pstSysEnv, chn);
	g_pstSysEnv->GetIPCamPrivData(g_pstSysEnv, chn, venc->index, &priv, sizeof(priv));

	if (protocal == IPCAM_FILE) 
	{
		resolution_nr = 1;
		width[0] = priv.file.width;
		height[0] = priv.file.height;
		bps_min = bps_max = priv.file.bps;
		fps_min = fps_max = priv.file.fps;
	}
	

	// FIM Me
	venc->enc_fps.min = fps_min;
	venc->enc_fps.max = fps_max;

	venc->enc_gov.min = 1;
	venc->enc_gov.max = fps_max;

	venc->enc_interval.min = 1;
	venc->enc_interval.max = 1;
	
	venc->enc_quality.min = 0;
	venc->enc_quality.max = 4;

	
	venc->enc_bps.min  = bps_min;
	venc->enc_bps.max = bps_max;
	
	venc->resolution_nr = resolution_nr;
	for ( n = 0; n < resolution_nr; n++) {
		NVP_SET_SIZE(&venc->resolution[n], width[n], height[n]);
	}

	venc->enc_profile_nr = 1;
	venc->enc_profile[0] = NVP_H264_PROFILE_BASELINE;

	return 0;	
}

static int _get_video_encode(lpNVP_VENC_CONFIG venc, int chn)
{
	int protocal = 0;
	SysenvIPCPrivData priv;
	int width = 0, height = 0;
	int fps = 0;
	int bps = 0;

	protocal = g_pstSysEnv->GetIPCamProtocolname(g_pstSysEnv, chn);
	g_pstSysEnv->GetIPCamPrivData(g_pstSysEnv, chn, venc->index, &priv, sizeof(priv));
	venc->user_count = 96;
	venc->enc_type = NVP_VENC_H264;
	venc->enc_interval = 1;
	venc->quant_mode = NVP_QUANT_CBR;
	venc->enc_profile = NVP_H264_PROFILE_BASELINE;

	if (protocal == IPCAM_FILE) 
	{
		width = priv.file.width;
		height = priv.file.height;
		bps = priv.file.bps;
		fps = priv.file.fps;
	}

	
	venc->width = width;
	venc->height = height;
	venc->enc_bps = bps;
	venc->enc_fps = fps;

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
	profile->venc_nr = profile->profile_nr;
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
static void _nvp_env_init(lpNVP_ENV env)
{
#define ONVIF_SET_NT(ptr, value)			snprintf(ptr, sizeof(ptr), "%s", value)
#define ONVIF_SET_NT_CHN(ptr, value,chn)	snprintf(ptr, sizeof(ptr), "%s%d", value, chn)
#define ONVIF_SET_NT_ID(ptr, value,chn, id)	snprintf(ptr, sizeof(ptr), "%s%d_%d", value, chn, id)
	int i, n;

	if (env == NULL)
		return;

	memset(&env->devinfo, 0, sizeof(stNVP_DEV_INFO));
	memset(&env->systime, 0, sizeof(stNVP_DATE_TIME));
	memset(&env->ether, 0, sizeof(stNVP_ETHER_CONFIG));
	memset(&env->profiles, 0, sizeof(stNVP_PROFILE));

	env->profiles.chn =  NVP_CH_NR;
	for ( n = 0; n < NVP_MAX_CH; n++)
	{
		env->profiles.profile[n].index = n;
		env->profiles.profile[n].profile_nr = NVP_VENC_NR;
		for ( i = 0; i < NVP_MAX_VENC; i++) {
			ONVIF_SET_NT_ID(env->profiles.profile[n].name[i], "Profile",n,  i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].token[i], "ProfileToken", n,i);
			
			ONVIF_SET_NT_ID(env->profiles.profile[n].ain_name[i], "AIN", n,i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].ain_token[i], "AINToken", n,i);
		}

		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.name, "VIN", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.token, "VINToken", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.image.name, "IMG", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.image.token, "IMGToken", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.image.src_token, "VINToken", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.image.color.src_token, "VINToken", n);
		
		env->profiles.profile[n].venc_nr = NVP_VENC_NR;
		for ( i = 0; i < NVP_MAX_VENC; i++) {
			env->profiles.profile[n].venc[i].index = i;
			ONVIF_SET_NT_ID(env->profiles.profile[n].venc[i].name, "Profile", n, i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].venc[i].token, "ProfileToken", n, i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].venc[i].enc_name, "VENC", n, i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].venc[i].enc_token, "VENCToken", n, i);
		}

		env->profiles.profile[n].vin_conf_nr = NVP_VIN_IN_A_SOURCE;
		for ( i = 0; i < NVP_MAX_VIN_IN_A_SOURCE; i++) {
			env->profiles.profile[n].venc[i].index = i;
			ONVIF_SET_NT_ID(env->profiles.profile[n].vin[i].name, "VIN", n, i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].vin[i].token, "VINToken", n, i);
		}

		ONVIF_SET_NT_CHN(env->profiles.profile[n].ain.name, "AIN", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].ain.token, "AINToken", n);

		env->profiles.profile[n].aenc_nr = NVP_AENC_NR;
		for ( i = 0; i < NVP_MAX_AENC; i++) {
			env->profiles.profile[n].aenc[i].index = i;
			ONVIF_SET_NT_ID(env->profiles.profile[n].aenc[i].name, "AENC", n, i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].aenc[i].token, "AENCToken", n, i);
		}

		ONVIF_SET_NT_CHN(env->profiles.profile[n].van.name, "VAN", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].van.token, "VANToken", n);

		ONVIF_SET_NT_CHN(env->profiles.profile[n].md.rule_name, "MDRule", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].md.module_name, "MDModule", n);

		ONVIF_SET_NT_CHN(env->profiles.profile[n].ptz.name, "PTZ", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].ptz.token, "PTZToken", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].ptz.node_name, "PTZNode", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].ptz.node_token, "PTZNodeToken", n);
		env->profiles.profile[n].ptz.preset_nr = NVP_MAX_PTZ_PRESET;
		for ( i  = 0; i < NVP_MAX_PTZ_PRESET; i++) {
			env->profiles.profile[n].ptz.preset[i].index = i;
			memset(env->profiles.profile[n].ptz.preset[i].name, 0, sizeof(env->profiles.profile[n].ptz.name));
			sprintf(env->profiles.profile[n].ptz.preset[i].token, "PresetToken%d", i + 1);
			env->profiles.profile[n].ptz.preset[i].in_use = false;
		}
		env->profiles.profile[n].ptz.default_pan_speed = 0.5;
		env->profiles.profile[n].ptz.default_tilt_speed = 0.5;
		env->profiles.profile[n].ptz.default_zoom_speed = 0.5;
		env->profiles.profile[n].ptz.tour_nr = 0;
		for ( i  = 0; i < NVP_MAX_PTZ_TOUR; i++) {
			env->profiles.profile[n].ptz.tour[i].index = i;
			memset(env->profiles.profile[n].ptz.tour[i].name, 0, sizeof(env->profiles.profile[n].ptz.tour[i].name));
			sprintf(env->profiles.profile[n].ptz.tour[i].token, "TourToken%d", i + 1);
		}
	}

	
	ONVIF_SET_NT(env->ether.name, "eth0");
	ONVIF_SET_NT(env->ether.token, "Eth0Token");

}

int NVP_env_load(lpNVP_ENV env, const char *module, int keyid)
{
	char temp[512];
	char *ptr = NULL, *pbuf;
	char *saveptr = NULL;
	int ret;
	int chn, id;

	_nvp_env_init(env);

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
}

int NVP_env_save(lpNVP_ENV env, const char *module, int keyid)
{
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
}

int NVP_env_cmd(lpNVP_CMD cmd, const char *module, int keyid)
{
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
}

#endif
