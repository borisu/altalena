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
								IN CnxInfo sip_stack_data,
								IN ICallHandlerCreatorPtr handler_creator)
{

	return 
		new ProcSipStack(
			stack_pair, 
			sip_stack_data,
			handler_creator);

}
