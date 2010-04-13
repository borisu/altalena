#pragma once
#include "LightweightProcess.h"


namespace ivrworx
{

class ProcOpalH323 :
	public LightweightProcess,
	public PProcess
{
	PCLASSINFO(ProcOpalH323, PProcess)

public:
	

	ProcOpalH323(Configuration &conf, LpHandlePair pair);
	virtual ~ProcOpalH323(void);

	void real_run();

	void Main();


private:

	Configuration &_conf;

	shared_ptr<H323EndPoint> _h323EP;

	
};

}
