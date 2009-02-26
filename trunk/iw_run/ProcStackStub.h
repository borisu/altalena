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

	virtual void UponMakeCall(IwMessagePtr msg);

	virtual void UponHangupCall(IwMessagePtr msg);

	virtual void UponCallOfferedAck(IwMessagePtr msg);

	int handle_counter;
};
