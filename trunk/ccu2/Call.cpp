/*
*	The Altalena Project File
*	Copyright (C) 2009  Boris Ouretskey
*
*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "StdAfx.h"
#include "Call.h"
#include "CcuLogger.h"


using namespace ivrworx;

Call::Call(IN LpHandlePair stack_pair, 
		   IN LightweightProcess &parent_process):
_stackPair(stack_pair),
_stackCallHandle(CCU_UNDEFINED),
_parentProcess(parent_process),
_hangupDetected(FALSE),
_handlerPair(HANDLE_PAIR)
{
	_callState = IX_CALL_NONE;
	Init();
}

Call::Call(
	 IN LpHandlePair stack_pair, 
	 IN int stack_handle,
	 IN CnxInfo offered_media,
	 IN LightweightProcess &parent_process):
 _stackPair(stack_pair),
 _stackCallHandle(stack_handle),
 _remoteMedia(offered_media),
 _parentProcess(parent_process),
 _handlerPair(HANDLE_PAIR)
{
	_callState = IX_CALL_OFFERED;

	LogDebug("Creating call session - ix stack handle=[" << _stackCallHandle << "].");

	Init();
}

#pragma TODO ("Crashes on dtor")
void
Call::Init()
{

// 	_forking.forkInThisThread(
// 		new ProcVoidFuncRunner<Call>(
// 		_handlerPair,
// 		bind<void>(&Call::call_handler_run, _1),
// 		this,
// 		L"Call Handler"));

}

Call::~Call(void)
{
	HagupCall();

	_parentProcess.Shutdown(Seconds(5), _handlerPair);

	_hangupDetected = TRUE;
}

CcuApiErrorCode
Call::WaitForDtmf(IN wstring &dtmf_digit, IN Time timeout)
{
	CcuApiErrorCode res = CCU_API_SUCCESS;
	CcuMsgPtr ptr = _dtmfChannel.Wait(timeout,res);

	if (CCU_FAILURE(res))
	{
		return res;
	}

	shared_ptr<CcuMsgCallDtmfEvt> dtmfEvt = 
		dynamic_pointer_cast<CcuMsgCallDtmfEvt> (ptr);

	dtmf_digit = dtmfEvt->dtmf_digit;

	return res;


}

CcuApiErrorCode
Call::RejectCall()
{
	FUNCTRACKER;

	CcuMsgCallOfferedNack *msg = new CcuMsgCallOfferedNack();

	msg->stack_call_handle = _stackCallHandle;

	_stackPair.inbound->Send(msg);

	return CCU_API_SUCCESS;

}

CcuApiErrorCode
Call::HagupCall()
{
	
	FUNCTRACKER;

	LogDebug("Hanging up the call - ix stack handle=[" << _stackCallHandle << "].");

	if (_stackCallHandle == CCU_UNDEFINED || 
		_callState == IX_CALL_NONE		  || 
		_callState == IX_CALL_TERMINATED)
	{
		return CCU_API_SUCCESS;
	}

	CcuMsgHangupCallReq *msg = new CcuMsgHangupCallReq(_stackCallHandle);

	_stackPair.inbound->Send(msg);

	_stackCallHandle = CCU_UNDEFINED;

	_hangupDetected = TRUE;

	return CCU_API_SUCCESS;
}

CcuApiErrorCode
Call::AcceptCall(IN CnxInfo local_media)
{
	
	FUNCTRACKER;

	LogDebug("Accepting call - ix stack handle=[" << _stackCallHandle << "].");

	CcuMsgPtr response = CCU_NULL_MSG;
	
	CcuMsgCalOfferedlAck *ack = new CcuMsgCalOfferedlAck();
	ack->stack_call_handle = _stackCallHandle;
	ack->local_media = local_media;


	EventsSet map;
	map.insert(CCU_MSG_CALL_CONNECTED);

	CcuApiErrorCode res = _parentProcess.DoRequestResponseTransaction(
		_stackPair.inbound,
		CcuMsgPtr(ack),
		map,
		response,
		MilliSeconds(_parentProcess.TransactionTimeout()),
		L"Accept Call TXN");

	if (CCU_FAILURE(res))
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_CALL_CONNECTED:
		{
			shared_ptr<CcuMsgNewCallConnected> make_call_sucess = 
				dynamic_pointer_cast<CcuMsgNewCallConnected>(response);

			_stackCallHandle = make_call_sucess->stack_call_handle;

			break;
		}
	default:
		{
			throw;
		}
	}

	_callState = IX_CALL_CONNECTED;

	return res;
}

int 
Call::StackCallHandle() const 
{ 
	return _stackCallHandle; 
}



void
Call::call_handler_run()
{

	BOOL shutdown_flag = FALSE;
	while (!shutdown_flag)
	{
		CcuApiErrorCode res = CCU_API_SUCCESS;
		CcuMsgPtr message = _handlerPair.inbound->Wait(Seconds(10),res);

		if (res == CCU_API_TIMEOUT)
		{
			if (_hangupDetected)
			{
				return;
			}

			continue;
		}

		switch (message->message_id)
		{
		case CCU_MSG_CALL_DTMF_EVT:
			{
				_dtmfChannel.Send(message);
				break;
			}
		case CCU_MSG_PROC_SHUTDOWN_REQ:
			{
				_parentProcess.SendResponse(message,new CcuMsgShutdownAck());
				shutdown_flag = true;
				break;
			}
		default:
			{
				BOOL res = _parentProcess.HandleOOBMessage(message);
				if (res == FALSE)
				{
					LogCrit("OOB message");
					throw;
				}
			} // default
		}
	}
	

}

CcuApiErrorCode
Call::MakeCall(IN wstring destination_uri, 
			   IN CnxInfo local_media)
{
	FUNCTRACKER;

	_localMedia = local_media;

	CcuMsgPtr response = CCU_NULL_MSG;

	CcuMsgMakeCallReq *msg = new CcuMsgMakeCallReq();
	msg->local_media = local_media;
	msg->destination_uri = destination_uri;
	msg->call_handler_inbound = _handlerPair.inbound;

	EventsSet map;
	map.insert(CCU_MSG_MAKE_CALL_ACK);
	map.insert(CCU_MSG_MAKE_CALL_NACK);

	CcuApiErrorCode res = _parentProcess.DoRequestResponseTransaction(
		_stackPair.inbound,
		CcuMsgPtr(msg),
		map,
		response,
		Seconds(60),
		L"Make Call TXN");

	if (res != CCU_API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_MAKE_CALL_ACK:
		{
			shared_ptr<CcuMsgMakeCallAck> make_call_sucess = 
				dynamic_pointer_cast<CcuMsgMakeCallAck>(response);

			_stackCallHandle = make_call_sucess->stack_call_handle;

			_remoteMedia = make_call_sucess->remote_media;

			break;
		}
	case CCU_MSG_MAKE_CALL_NACK:
		{
			res = CCU_API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}
	return res;
}

CnxInfo 
Call::RemoteMedia() const 
{ 
	return _remoteMedia; 
}

void 
Call::RemoteMedia(CnxInfo &val) 
{ 
	_remoteMedia = val; 
}


CnxInfo 
Call::LocalMedia() const 
{ 
	return _localMedia; 
}

void 
Call::LocalMedia(CnxInfo &val) 
{ 
	_localMedia = val; 
}

