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
#include "SipCall.h"
#include "ResipCommon.h"


#define CALL_RESET_STATE(X) ResetState(X,#X)

namespace ivrworx
{

SipMediaCall::SipMediaCall(IN ScopedForking &forking, IN HandleId handle_id):
	MediaCallSession(forking, handle_id)
{
	FUNCTRACKER;
}

SipMediaCall::SipMediaCall(IN ScopedForking &forking,
		   IN shared_ptr<MsgCallOfferedReq> offered_msg):
	 MediaCallSession(forking, offered_msg->source.handle_id, offered_msg)
{
	FUNCTRACKER;

}

SipMediaCall::~SipMediaCall()
{

}

ApiErrorCode
SipMediaCall::SendInfo(const string &body, const string &type)
{
	FUNCTRACKER;

	LogDebug("SipMediaCall::SendInfo - body:" << body  << ", type:"  << type);

	MsgSipCallInfoReq *msg = new MsgSipCallInfoReq();
	msg->stack_call_handle = _iwCallHandle;
	msg->offer  = body;
	msg->offer_type = type;
	
	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		_stackHandleId,
		IwMessagePtr(msg),
		response,
		Seconds(10),
		"SIP SendInfo TXN");

	if (res != API_SUCCESS)
	{
		LogWarn("SipMediaCall::SendInfo - Error allocating RTSP session" << res);
		return res;
	}

	switch (response->message_id)
	{
	case SIP_CALL_INFO_ACK:
		{

			LogDebug("SipMediaCall::SendInfo - OK :");

			break;

		}
	case SIP_CALL_INFO_NACK:
		{
			LogWarn("Error sending options .");
			res = API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}

	return res;

}

void 
SipMediaCall::UponActiveObjectEvent(IwMessagePtr ptr)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case SIP_CALL_INFO_REQ:
		{

			if (_infosChannel)
			{
				_infosChannel->Send(ptr);
			}
			break;
		}
	default:
		{
			ActiveObject::UponActiveObjectEvent(ptr);
		}
	}

	
}

}
