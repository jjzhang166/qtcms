#include "IVideoRender.h"
#include "SDLRender.h"
#include "DDrawRender.h"

IVideoRender::IVideoRender(void)
{
}


IVideoRender::~IVideoRender(void)
{
}

IVideoRender * IVideoRender::CreateRender( RenderType type )
{
	IVideoRender * pRet = 0;
	switch(type)
	{
	case IVideoRender::RT_SDL: pRet = new CSDLRender();break;
	case IVideoRender::RT_DDRAW: pRet = new CDDrawRender();break;
	default: break;
	}
	return pRet;
}