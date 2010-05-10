#pragma once
#include "LightweightProcess.h"

#define OPAL_H323_STACK_Q 234

namespace ivrworx
{

struct OpalOverlapped:
	public OVERLAPPED
{
	OpalOverlapped(){}
};

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

	bool ProcessApplicationMessages();

	bool ProcessOpalMessages();

	virtual void UponMakeCallReq(IN IwMessagePtr req);

	virtual void UponMakeCallAckReq(IN IwMessagePtr req);

	virtual void UponHangupCallReq(IN IwMessagePtr req);

	virtual void UponShutDownReq(IN IwMessagePtr req);

	virtual void UponCallOfferedAck(IN IwMessagePtr req);

	virtual void UponCallOfferedNack(IN IwMessagePtr req);


private:

	Configuration &_conf;

	IocpInterruptorPtr _iocpPtr;

};

}
