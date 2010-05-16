#include "StdAfx.h"
#include "RtpProxyFactory.h"
#include "ProcRtpProxy.h"

namespace ivrworx
{



RtpProxyFactory::RtpProxyFactory(void)
{
}

RtpProxyFactory::~RtpProxyFactory(void)
{
}


LightweightProcess *
RtpProxyFactory::Create(LpHandlePair pair, Configuration &conf)
{
	return new ProcRtpProxy(pair,conf);
}

}