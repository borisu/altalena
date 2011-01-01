#include "StdAfx.h"
#include "Live555RtpProxyFactory.h"
#include "ProcLive555RtpProxy.h"

namespace ivrworx
{



Live555RtpProxyFactory::Live555RtpProxyFactory(void)
{
}

Live555RtpProxyFactory::~Live555RtpProxyFactory(void)
{
}


LightweightProcess *
Live555RtpProxyFactory::Create(LpHandlePair pair, ConfigurationPtr conf)
{
	return new ProcLive555RtpProxy(pair,conf);
}

IW_LIVE555PROXY_API IProcFactory* GetIwFactory()
{
	return new Live555RtpProxyFactory();
}

}