#include "StdAfx.h"
#include "ProcStackStub.h"
#include "CcuLogger.h"
#include "Call.h"


ProcStackStub::ProcStackStub(LpHandlePair pair):
LightweightProcess(pair, __FUNCTIONW__),
handle_counter(0)
{
}

ProcStackStub::~ProcStackStub(void)
{
}

void
ProcStackStub::real_run()
{


	BOOL shutdownFlag = FALSE;
	while (!shutdownFlag)
	{
		IxApiErrorCode res;
		IwMessagePtr msg = GetInboundMessage(Seconds(0),res);
		if (CCU_FAILURE(res))
		{
			throw;
		}

		LogInfo(L" Processing msg=[" << msg->message_id_str <<"]");

		if (msg.get() == NULL)
			break;

		switch (msg->message_id)
		{
		case MSG_MAKE_CALL_REQ:
			{
				UponMakeCall(msg);
				break;
			}
		case CCU_MSG_PROC_SHUTDOWN_REQ:
			{
				
				shutdownFlag = TRUE;
				SendResponse(msg, new CcuMsgShutdownAck());
				break;
			}
		case MSG_HANGUP_CALL_REQ:
			{
				UponHangupCall(msg);
				break;

			}
		case MSG_CALL_OFFERED_ACK:
			{
				UponCallOfferedAck(msg);
				break;
			}
		default:
			{ 
				if (HandleOOBMessage(msg) == FALSE)
				{
					LogInfo(L" Received unknown message " << msg->message_id_str)
				}

			}
		}
	}
}

void
ProcStackStub::UponCallOfferedAck(IwMessagePtr msg)
{

}


void
ProcStackStub::UponMakeCall(IwMessagePtr ptr)
{
	MsgMakeCallReq *req  = 
		boost::shared_dynamic_cast<MsgMakeCallReq>(ptr).get();

	MsgMakeCallAck *ack =
		new MsgMakeCallAck();

	ack->stack_call_handle = handle_counter ++;

	ack->remote_media = CnxInfo(DUMMY_STACK_REMOTE_IP,(int)DUMMY_STACK_REMOTE_PORT);

	SendResponse(ptr,ack);

}

void
ProcStackStub::UponHangupCall(IwMessagePtr msg)
{

}