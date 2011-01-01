#pragma once
#include "LightweightProcess.h"

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
	

	ProcOpalH323(ConfigurationPtr conf, LpHandlePair pair);
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

	ConfigurationPtr _conf;

	IocpInterruptorPtr _iocpPtr;

};

}
