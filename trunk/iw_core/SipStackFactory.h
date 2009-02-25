#pragma once
#include "Configuration.h"
#include "LightweightProcess.h"
#include "Call.h"

// wrapper class to hide dependencies

namespace ivrworx
{
	class SipStackFactory
	{
	public:
		SipStackFactory(void);
		virtual ~SipStackFactory(void);

		static LightweightProcess *CreateSipStack(
			IN LpHandlePair stack_pair, 
			IN Configuration &conf);
	};


}
