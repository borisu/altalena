#include "StdAfx.h"
#include "RtspFactory.h"
#include "ProcRtsp.h"

namespace ivrworx
{
	RtspFactory::RtspFactory(void)
	{
	}

	RtspFactory::~RtspFactory(void)
	{
	}

	LightweightProcess * 
	RtspFactory::Create(ivrworx::LpHandlePair pair, ivrworx::Configuration &conf)
	{
		return new ProcRtsp(conf,pair);
	}

};

