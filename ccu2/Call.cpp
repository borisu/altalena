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



Call::Call(IN LpHandlePair stack_pair, 
		   IN LightweightProcess &parent_process):
_stackPair(stack_pair),
_stackCallHandle(CCU_UNDEFINED),
_parentProcess(parent_process),
_handlerHandle(new LpHandle())
{

}

Call::Call(
	 IN LpHandlePair stack_pair, 
	 IN int stack_handle,
	 IN CcuMediaData offered_media,
	 IN LightweightProcess &parent_process):
 _stackPair(stack_pair),
 _stackCallHandle(stack_handle),
 _remoteMedia(offered_media),
 _parentProcess(parent_process),
 _handlerHandle(new LpHandle())
{
	
}

Call::~Call(void)
{
	HagupCall();
}

void
Call::real_run()
{

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

	if (_stackCallHandle == CCU_UNDEFINED)
	{
		return CCU_API_SUCCESS;
	}

	CcuMsgHangupCallRequest *msg = new CcuMsgHangupCallRequest(_stackCallHandle);

	_stackPair.inbound->Send(msg);

	_stackCallHandle = CCU_UNDEFINED;

	return CCU_API_SUCCESS;
}

CcuApiErrorCode
Call::AcceptCall(IN CcuMediaData local_media)
{
	FUNCTRACKER;

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
			shared_ptr<CcuMsgMakeCallSuccess> make_call_sucess = 
				dynamic_pointer_cast<CcuMsgMakeCallSuccess>(response);

			_stackCallHandle = make_call_sucess->stack_call_handle;

			break;
		}
	default:
		{
			throw;
		}
	}

	return res;
}


CcuApiErrorCode
Call::MakeCall(IN wstring destination_uri, 
			   IN CcuMediaData local_media)
{
	FUNCTRACKER;

	CcuMsgPtr response = CCU_NULL_MSG;

	CcuMsgMakeCallRequest *msg = new CcuMsgMakeCallRequest();
	msg->local_media = local_media;
	msg->destination_uri = destination_uri;
	msg->call_handler_inbound = _handlerHandle;

	EventsSet map;
	map.insert(CCU_MSG_MAKE_CALL_SUCCESS);
	map.insert(CCU_MSG_MAKE_CALL_FAILURE);

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
	case CCU_MSG_MAKE_CALL_SUCCESS:
		{
			shared_ptr<CcuMsgMakeCallSuccess> make_call_sucess = 
				dynamic_pointer_cast<CcuMsgMakeCallSuccess>(response);

			_stackCallHandle = make_call_sucess->stack_call_handle;

			_remoteMedia = make_call_sucess->remote_media;

			break;
		}
	case CCU_MSG_MAKE_CALL_FAILURE:
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

CcuMediaData 
Call::RemoteMedia() const 
{ 
	return _remoteMedia; 
}

void 
Call::RemoteMedia(CcuMediaData val) 
{ 
	_remoteMedia = val; 
}

