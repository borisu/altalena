#include "StdAfx.h"
#include "ProcImsStub.h"
#include "Ims.h"

ProcImsStub::ProcImsStub(LpHandlePair pair)
:LightweightProcess(pair,IMS_Q,__FUNCTIONW__),
_currHandle(CCU_UNDEFINED)
{

}

ProcImsStub::~ProcImsStub(void)
{
}

void
ProcImsStub::real_run()
{
	I_AM_READY;

	BOOL shutdownFlag = FALSE;
	while (shutdownFlag  == FALSE)
	{
		CcuApiErrorCode err_code = CCU_API_SUCCESS;
		CcuMsgPtr ptr =  _inbound->Wait(Seconds(60), err_code);


		if (err_code == CCU_API_TIMEOUT)
		{
			LogDebug("IMS >>Keep Alive<<");
			continue;
		}

		switch (ptr->message_id)
		{
		case CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST:
			{
				AllocatePlaybackSession(ptr);
				break;
			}
		case CCU_MSG_START_PLAYBACK_REQUEST:
			{
				StartPlayback(ptr);
				break;
			}
		case CCU_MSG_PROC_SHUTDOWN_REQ:
			{
				shutdownFlag = TRUE;
				SendResponse(ptr,new CcuMsgShutdownAck());

				continue;
			}
		default:
			{
				BOOL  oob_res = HandleOOBMessage(ptr);

				if (oob_res == FALSE)
				{
					LogWarn("Received OOB message id=[" << ptr->message_id << "]");
					_ASSERT	(false);
				} // if
			}// default
		}// switch
	}//while


}

void 
ProcImsStub::AllocatePlaybackSession(CcuMsgPtr msg)
{
	FUNCTRACKER;

	shared_ptr<CcuMsgAllocateImsSessionReq> req  =
		dynamic_pointer_cast<CcuMsgAllocateImsSessionReq> (msg);

	int handle = ++_currHandle;

	CcuMsgAllocateImsSessionAck *ack = 
		new CcuMsgAllocateImsSessionAck();

	ack->playback_handle = handle;

	ack->ims_media = CnxInfo(IMS_DUMMY_IP,IMS_DUMMY_PORT);

	SendResponse(req,ack);

}

void 
ProcImsStub::StartPlayback(CcuMsgPtr msg)
{

	FUNCTRACKER;

	shared_ptr<CcuMsgStartPlayReq> req  =
		dynamic_pointer_cast<CcuMsgStartPlayReq> (msg);

	SendResponse(req, new CcuMsgImsPlayStopped());

}

void 
ProcImsStub::StopPlayback(CcuMsgPtr msg)
{
	FUNCTRACKER;

	
}

