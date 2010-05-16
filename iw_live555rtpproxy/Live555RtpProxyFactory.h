#pragma once
#include "LightweightProcess.h"

namespace  ivrworx
{


class Live555RtpProxyFactory:
	public IProcFactory
{
public:
	Live555RtpProxyFactory(void);
	LightweightProcess *Create(LpHandlePair pair, Configuration &conf);
	virtual ~Live555RtpProxyFactory(void);
};

}
