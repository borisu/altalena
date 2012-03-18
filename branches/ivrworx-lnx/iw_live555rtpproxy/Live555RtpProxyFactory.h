#pragma once
#include "LightweightProcess.h"

#ifdef _DLL_IW_LIVE555PROXY // assume this is defined when we build the DLL
#define IW_LIVE555PROXY_API  __declspec( dllexport)
#else
#define IW_LIVE555PROXY_API  __declspec( dllimport)
#endif

namespace  ivrworx
{


class IW_LIVE555PROXY_API Live555RtpProxyFactory:
	public IProcFactory
{
public:
	Live555RtpProxyFactory(void);
	LightweightProcess *Create(LpHandlePair pair, ConfigurationPtr conf);
	virtual ~Live555RtpProxyFactory(void);
};

IW_LIVE555PROXY_API IProcFactory* GetIwFactory();

}
