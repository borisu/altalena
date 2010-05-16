#pragma once
#include "LightweightProcess.h"

namespace  ivrworx
{


class RtpProxyFactory:
	public IProcFactory
{
public:
	RtpProxyFactory(void);
	LightweightProcess *Create(LpHandlePair pair, Configuration &conf);
	virtual ~RtpProxyFactory(void);
};

}
