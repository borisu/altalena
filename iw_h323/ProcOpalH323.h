#pragma once
#include "LightweightProcess.h"


namespace ivrworx
{

class ProcOpalH323 :
	public LightweightProcess
{
public:
	ProcOpalH323(Configuration &conf, LpHandlePair pair);
	virtual ~ProcOpalH323(void);

	void real_run();

private:

	Configuration &_conf;

	H323EndPoint *_h323EP;

	
};

}
