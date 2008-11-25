#pragma once
#include "LightweightProcess.h"

#define DUMMY_STACK_REMOTE_IP "127.0.0.1"
#define DUMMY_STACK_REMOTE_PORT "127.0.0.1"

class ProcStackStub : 
	public LightweightProcess
{
public:
	ProcStackStub(LpHandlePair pair);
	virtual ~ProcStackStub(void);

	virtual void real_run();

	virtual void UponMakeCall(CcuMsgPtr msg);

	virtual void UponHangupCall(CcuMsgPtr msg);

	virtual void UponCallOfferedAck(CcuMsgPtr msg);

	int handle_counter;
};
