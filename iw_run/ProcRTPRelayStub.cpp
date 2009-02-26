#include "StdAfx.h"
#include "ProcRTPRelayStub.h"

MockRtpConnection::MockRtpConnection()
{

}

MockRtpConnection::MockRtpConnection(const MockRtpConnection& other)
{
	this->local = other.local;
	this->remote = other.remote;
}

ProcRTPRelayStub::ProcRTPRelayStub(LpHandlePair pair):
LightweightProcess(pair,RTP_RELAY_Q,__FUNCTIONW__),
_rtpHandleCounter(IW_UNDEFINED)
{
}

ProcRTPRelayStub::~ProcRTPRelayStub(void)
{
}

void
ProcRTPRelayStub::real_run()
{
	IxApiErrorCode err_code = CCU_API_SUCCESS;
	BOOL shutdownFlag = FALSE;
	IwMessagePtr shutdown_req = CCU_NULL_MSG;

	while (shutdownFlag  == FALSE)
	{
		IwMessagePtr ptr =  _inbound->Wait(
			Seconds(60),
			err_code);


		if (err_code == CCU_API_TIMEOUT)
		{
			LogDebug("Timeout = Keep Alive");
			continue;
		}


		switch (ptr->message_id)
		{
		case CCU_MSG_ALLOCATE_NEW_CONNECTION_REQ:
			{
				AllocateAudioConnection(ptr);
				break;
			}
		case CCU_MSG_BRIDGE_CONNECTIONS_REQ:
			{
				BridgeConnections(ptr);
				break;
			}
		case CCU_CLOSE_CONNECTION_REQ:
			{
				CloseAudioConnection(ptr);
				break;
			}
		case CCU_MSG_MODIFY_CONNECTION_REQ:
			{
				ModifyConnection(ptr);
				break;
			}
		case CCU_MSG_PROC_SHUTDOWN_REQ:
			{
#pragma TODO ("Handle Shutdown Correctly")
				shutdownFlag = TRUE;
				SendResponse(ptr, 
					new CcuMsgShutdownAck());
				continue;
			}
		default:
			{
				LogWarn("Received OOB message id=[" << ptr->message_id << "]");
				_ASSERT	(false);
			}

		}
	}

}


void
ProcRTPRelayStub::AllocateAudioConnection(IN IwMessagePtr ptr)
{
	shared_ptr<CcuMsgRtpAllocateNewConnectionReq> ac_msg =
		shared_dynamic_cast<CcuMsgRtpAllocateNewConnectionReq>(ptr);

	CcuMsgRtpAllocateConnectionAck *ack = 
		new CcuMsgRtpAllocateConnectionAck();

	MockRtpConnection rtpConnection;
	rtpConnection.local = CnxInfo(DUMMY_RTP_ADDRESS,DUMMY_RTP_PORT);

	
	int  handle = ++_rtpHandleCounter;

	_mockConnections[handle] = rtpConnection;

	ack->connection_media = rtpConnection.local;
	ack->connection_id = handle;

	SendResponse(ptr,ack);

	
}

void
ProcRTPRelayStub::CloseAudioConnection(IN IwMessagePtr ptr)
{
	shared_ptr<CcuMsgRtpCloseConnectionReq> request =
		shared_dynamic_cast<CcuMsgRtpCloseConnectionReq> (ptr);

	_mockConnections.erase(request->connection_id);
	_rtpBridges.erase(request->connection_id);
	
}

void
ProcRTPRelayStub::BridgeConnections(IN IwMessagePtr ptr)
{
	shared_ptr<CcuMsgRtpBridgeConnectionsReq> request =
		shared_dynamic_cast<CcuMsgRtpBridgeConnectionsReq> (ptr);

	if (_mockConnections.find(request->connection_id1) == _mockConnections.end() || 
		_mockConnections.find(request->connection_id2) == _mockConnections.end())
	{

		LogWarn("One of the bridged connection was not found conn1=[" 
			<< request->connection_id1 <<"], conn2=[" << request->connection_id2 << "].");

		SendResponse(ptr, 
			new CcuMsgRtpBridgeConnectionsNack());

		return;
	}
	
	_rtpBridges[request->connection_id1] = request->connection_id2 ;
	_rtpBridges[request->connection_id2] = request->connection_id1 ;

	SendResponse(ptr, 
		new CcuMsgRtpBridgeConnectionsAck());
}

void
ProcRTPRelayStub:: ModifyConnection(IN IwMessagePtr ptr)
{
	shared_ptr<CcuMsgModifyConnectionReq> request =
		shared_dynamic_cast<CcuMsgModifyConnectionReq> (ptr);

	MockRtpConnectionsMap::iterator iter = _mockConnections.find(request->connection_id);
	if ( iter == _mockConnections.end())
	{
		SendResponse(ptr, 
			new CcuMsgNack());
		return;
	}

	(*iter).second.remote = request->remote_media_data;

	SendResponse(ptr, 
		new CcuMsgAck());
}