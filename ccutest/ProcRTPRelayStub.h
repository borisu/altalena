#pragma once
#include "LightweightProcess.h"
#include "RtpRelay.h"

#define DUMMY_RTP_ADDRESS "127.0.0.1"
#define DUMMY_RTP_PORT 111

class ProcRTPRelayStub: 
	public LightweightProcess
{
public:
	ProcRTPRelayStub(LpHandlePair pair);

	virtual ~ProcRTPRelayStub(void);

	virtual void real_run();

	virtual void AllocateAudioConnection(IN CcuMsgPtr ptr);

	virtual void CloseAudioConnection(IN CcuMsgPtr ptr);

	virtual void BridgeConnections(IN CcuMsgPtr ptr);

	virtual void ModifyConnection(IN CcuMsgPtr ptr);
};
