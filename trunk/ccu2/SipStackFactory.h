#pragma once
#include "LightweightProcess.h"
#include "Call.h"

// wrapper class to hide dependencies
class SipStackFactory
{
public:
	SipStackFactory(void);
	virtual ~SipStackFactory(void);

	static LightweightProcess *CreateSipStack(
		IN LpHandlePair stack_pair, 
		IN CnxInfo sip_stack_data);
};
