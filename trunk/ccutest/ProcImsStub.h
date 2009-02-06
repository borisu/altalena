#pragma once
#include "LightweightProcess.h"
#include "Ccu.h"

#define IMS_DUMMY_IP	"127.0.0.1"
#define IMS_DUMMY_PORT	665

class ProcImsStub:
	public LightweightProcess
{
public:

	ProcImsStub(LpHandlePair pair);

	virtual ~ProcImsStub(void);

	virtual void real_run();

	virtual void AllocatePlaybackSession(IxMsgPtr msg);

	virtual void StartPlayback(IxMsgPtr msg);

	virtual void StopPlayback(IxMsgPtr msg);

	//virtual void UponPlaybackStopped(CcuMsgPtr msg);

private:

	CcuHandleId _currHandle;

	
};
