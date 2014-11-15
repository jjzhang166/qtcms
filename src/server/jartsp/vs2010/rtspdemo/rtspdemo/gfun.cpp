#include "stdafx.h"
#include "gfun.h"

LPDIRECTDRAW7			g_pDirectDraw7=NULL;				                       	//用于预览的DirectDraw对象
LPDIRECTDRAWSURFACE7	g_pPrimarySurface=NULL;	
LPDIRECTDRAWCLIPPER	    g_pClipper=NULL;
CRITICAL_SECTION		g_csClipper;


CRITICAL_SECTION g_csDecInit;

int DdrawInit(HWND hWnd)
{   
	HRESULT hr;
	
	
	//创建DDraw对象
	hr = DirectDrawCreateEx(NULL,(LPVOID *)&g_pDirectDraw7,IID_IDirectDraw7,NULL);
	if (FAILED(hr))
	{
		g_pDirectDraw7 = NULL;
		return -1;
	}
	
	//设置协作等级，执行成功，返回DD_OK
	hr = g_pDirectDraw7->SetCooperativeLevel(NULL,DDSCL_NORMAL);
	if (FAILED(hr))
	{
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		return -1;
	}
	
	//创建裁减器
	hr = g_pDirectDraw7->CreateClipper(0,&g_pClipper,NULL);
	if (FAILED(hr))
	{
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		return -1;
	}
	
	InitializeCriticalSection(&g_csClipper);
	
	//创建主页面
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	
	hr = g_pDirectDraw7->CreateSurface(&ddsd,&g_pPrimarySurface,NULL);
	if (FAILED(hr))
	{
		g_pClipper->Release();
		g_pClipper = NULL;
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		g_pPrimarySurface = NULL;
		return -1;
	}
	
	
	
	//加载裁剪器到主页面
	hr = g_pClipper->SetHWnd(NULL,hWnd);
	if (FAILED(hr))
	{
		g_pClipper->Release();
		g_pClipper = NULL;
		g_pPrimarySurface->Release();
		g_pPrimarySurface = NULL;
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		return -1;
	}
	//将剪裁器对象连接到主表面 
	hr = g_pPrimarySurface->SetClipper(g_pClipper);
	if (FAILED(hr))
	{
		g_pClipper->Release();
		g_pClipper = NULL;
		g_pPrimarySurface->Release();
		g_pPrimarySurface = NULL;
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
		return -1;
	}
	
	
	
	return 0;
}

void DdrawDeinit()
{
	if (g_pClipper)
	{
		g_pClipper->Release();
		g_pClipper = NULL;
	}
	if (g_pPrimarySurface)
	{	
		g_pPrimarySurface->Release();
		g_pPrimarySurface = NULL;
	}
	if (g_pDirectDraw7)
	{
		g_pDirectDraw7->Release();
		g_pDirectDraw7 = NULL;
	}

}

CTime time_t_to_ctime(time_t _timet)
{
	TIME_ZONE_INFORMATION tzi;
	GetSystemTime(&tzi.StandardDate);
	GetTimeZoneInformation(&tzi);
	
	CTime time(_timet + (tzi.Bias * 60));
//	CTime time(_timet);	
	return time;
}