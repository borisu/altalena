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


#define CALL_RESET_STATE(X) ResetState(X,#X)

namespace ivrworx
{


SipMediaCall::SipMediaCall(IN ScopedForking &forking, IN HandleId handle_id):
	GenericOfferAnswerSession(forking, handle_id)
{
	FUNCTRACKER;
}

SipMediaCall::SipMediaCall(IN ScopedForking &forking,
		   IN shared_ptr<MsgCallOfferedReq> offered_msg):
	 GenericOfferAnswerSession(forking, offered_msg->source.handle_id, offered_msg)
{
	FUNCTRACKER;

}

SipMediaCall::~SipMediaCall()
{

}

ApiErrorCode
SipMediaCall::WaitForInfo(IN AbstractOffer &remoteOffer)
{
	FUNCTRACKER;

	// just proxy the event
	int handle_index= IW_UNDEFINED;
	IwMessagePtr response = NULL_MSG;

	ApiErrorCode res = GetCurrRunningContext()->WaitForTxnResponse(
		assign::list_of(_infosChannel)(_hangupChannel),
		handle_index,
		response, 
		Seconds(60));

	if (IW_FAILURE(res))
	{
		return res;
	}

	if (handle_index == 1)
	{
		return API_HANGUP;
	}

	shared_ptr<MsgSipCallInfoReq> ack = 
		dynamic_pointer_cast<MsgSipCallInfoReq>(response);
	remoteOffer = ack->remoteOffer;

	return API_SUCCESS;

}


ApiErrorCode
SipMediaCall::SendInfo(IN const AbstractOffer &offer, OUT AbstractOffer &remoteOffer, IN bool async)
{
	FUNCTRACKER;

	LogDebug("SipMediaCall::SendInfo - body:" << offer.body  << ", type:"  << offer.type);

	MsgSipCallInfoReq *msg = new MsgSipCallInfoReq();
	msg->stack_call_handle = _iwCallHandle;
	msg->localOffer  = offer;

	
	IwMessagePtr response = NULL_MSG;

	ApiErrorCode res = API_SUCCESS;
	if (async)
	{
		res = GetCurrRunningContext()->DoRequestResponseTransaction(
			_stackHandleId,
			IwMessagePtr(msg),
			response,
			Seconds(10),
			"SIP SendInfo TXN");

	} else
	{
		res = GetCurrRunningContext()->SendMessage(
			_stackHandleId,
			IwMessagePtr(msg));
	}
	
	if (res != API_SUCCESS)
	{
		LogWarn("SipMediaCall::SendInfo - sending info option (check that stack is up)" << res);
		return res;
	}

	if (async)
		return res;

	switch (response->message_id)
	{
	case SIP_CALL_INFO_ACK:
		{
			shared_ptr<MsgSipCallInfoAck> ack = 
				dynamic_pointer_cast<MsgSipCallInfoAck>(response);
			remoteOffer = ack->remoteOffer;
			break;
		}
	case SIP_CALL_INFO_NACK:
		{
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
			GenericOfferAnswerSession::UponActiveObjectEvent(ptr);
		}
	}

	
}

}
