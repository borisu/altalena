#include "StdAfx.h"
#include "SipStackFactory.h"
#include  "ProcSipStack.h"

namespace ivrworx
{
	SipStackFactory::SipStackFactory(void)
	{
	}

	SipStackFactory::~SipStackFactory(void)
	{
	}

	LightweightProcess*
		SipStackFactory::CreateSipStack(IN LpHandlePair stack_pair, 
		IN Configuration &conf)
	{

		return 
			new ProcSipStack(
			stack_pair, 
			conf);

	}


}

