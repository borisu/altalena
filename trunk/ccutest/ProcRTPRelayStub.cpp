#include "StdAfx.h"
#include "ProcRTPRelayStub.h"

ProcRTPRelayStub::ProcRTPRelayStub(LpHandlePair pair):
LightweightProcess(pair,RTP_RELAY_Q,__FUNCTIONW__)
{
}

ProcRTPRelayStub::~ProcRTPRelayStub(void)
{
}

void
ProcRTPRelayStub::real_run()
{
	CcuApiErrorCode err_code = CCU_API_SUCCESS;
	BOOL shutdownFlag = FALSE;
	CcuMsgPtr shutdown_req = CCU_NULL_MSG;

	while (shutdownFlag  == FALSE)
	{
		CcuMsgPtr ptr =  _inbound->WaitForMessages(
			Seconds(60),
			err_code,
			SL(
			CCU_MSG_ALLOCATE_NEW_CONNECTION_REQ,
			CCU_MSG_BRIDGE_CONNECTIONS_REQ,
			CCU_CLOSE_CONNECTION_REQ,
			CCU_MSG_PROC_SHUTDOWN_REQ,
			CCU_MSG_MODIFY_CONNECTION_REQ));


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
ProcRTPRelayStub::AllocateAudioConnection(IN CcuMsgPtr ptr)
{
	shared_ptr<CcuMsgRtpAllocateNewConnectionReq> ac_msg =
		shared_dynamic_cast<CcuMsgRtpAllocateNewConnectionReq>(ptr);

	CcuMsgRtpAllocateConnectionAck *ack = 
		new CcuMsgRtpAllocateConnectionAck();

	ack->connection_media = CcuMediaData(DUMMY_RTP_ADDRESS,DUMMY_RTP_PORT);

	SendResponse(ptr,ack);

	
}

void
ProcRTPRelayStub::CloseAudioConnection(IN CcuMsgPtr ptr)
{
	
}

void
ProcRTPRelayStub::BridgeConnections(IN CcuMsgPtr ptr)
{
	SendResponse(ptr, 
		new CcuMsgRtpBridgeConnectionsAck());
}

void
ProcRTPRelayStub:: ModifyConnection(IN CcuMsgPtr ptr)
{
	SendResponse(ptr, 
		new CcuMsgAck());
}