#pragma once
#include "LightweightProcess.h"
#include "RtpRelay.h"

#define DUMMY_RTP_ADDRESS "127.0.0.1"
#define DUMMY_RTP_PORT 111


struct MockRtpConnection
{
	MockRtpConnection();

	CnxInfo local;

	CnxInfo remote;

	MockRtpConnection(const MockRtpConnection& other);
};

typedef 
map<int, MockRtpConnection> MockRtpConnectionsMap;

typedef 
map<int, int> RtpBridgesMap;


class ProcRTPRelayStub: 
	public LightweightProcess
{
public:
	ProcRTPRelayStub(LpHandlePair pair);

	virtual ~ProcRTPRelayStub(void);

	virtual void real_run();

	virtual void AllocateAudioConnection(IN IwMessagePtr ptr);

	virtual void CloseAudioConnection(IN IwMessagePtr ptr);

	virtual void BridgeConnections(IN IwMessagePtr ptr);

	virtual void ModifyConnection(IN IwMessagePtr ptr);

	MockRtpConnectionsMap _mockConnections;

	RtpBridgesMap _rtpBridges;

	int _rtpHandleCounter;

};
