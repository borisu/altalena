#include "StdAfx.h"
#include "SipStackFactory.h"
#include  "ProcSipStack.h"

SipStackFactory::SipStackFactory(void)
{
}

SipStackFactory::~SipStackFactory(void)
{
}

LightweightProcess*
SipStackFactory::CreateSipStack(IN LpHandlePair stack_pair, 
								IN CcuConfiguration &conf)
{

	return 
		new ProcSipStack(
			stack_pair, 
			conf);

}
