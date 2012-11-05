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

#pragma push_macro("SendMessage")
#undef SendMessage


#define CALL_RESET_STATE(X) ResetState(X,#X)

namespace ivrworx
{
SipMediaCall::SipMediaCall(IN ScopedForking &forking):
	GenericOfferAnswerSession(forking),
	_stackRegistrationHandle(IW_UNDEFINED),
	_stackSubscribeHandle(IW_UNDEFINED),
	_notifyChannel(new LpHandle()),
	_infosChannel(new LpHandle())
{
	FUNCTRACKER;
}

SipMediaCall::SipMediaCall(IN ScopedForking &forking, IN HandleId stack_handle_id):
	GenericOfferAnswerSession(forking, stack_handle_id),
	_stackRegistrationHandle(IW_UNDEFINED),
	_stackSubscribeHandle(IW_UNDEFINED),
	_notifyChannel(new LpHandle()),
	_infosChannel(new LpHandle())
{
	FUNCTRACKER;
}

SipMediaCall::SipMediaCall(IN ScopedForking &forking,
		   IN shared_ptr<MsgCallOfferedReq> offered_msg):
	GenericOfferAnswerSession(forking, offered_msg->source.handle_id, offered_msg),
	_stackRegistrationHandle(IW_UNDEFINED),
	_stackSubscribeHandle(IW_UNDEFINED)
{
	FUNCTRACKER;

}

SipMediaCall::~SipMediaCall()
{
	FUNCTRACKER;

}

ApiErrorCode
SipMediaCall::WaitForNotify(OUT AbstractOffer &remoteOffer)
{
	FUNCTRACKER;

	// just proxy the event
	int handle_index= IW_UNDEFINED;
	IwMessagePtr response = NULL_MSG;

	ApiErrorCode res = GetCurrRunningContext()->WaitForTxnResponse(
		assign::list_of(_notifyChannel)(_hangupChannel),
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

	shared_ptr<MsgSipCallNotifyEvt> notify = 
		dynamic_pointer_cast<MsgSipCallNotifyEvt>(response);
	remoteOffer = notify->remoteOffer;

	return API_SUCCESS;

}

ApiErrorCode
SipMediaCall::WaitForInfo(OUT AbstractOffer &remoteOffer)
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
SipMediaCall::MakeCall(IN const string   &destination_uri, 
							  IN const AbstractOffer &offer,
							  IN const Credentials &credentials,
							  IN OUT MapOfAny &key_value_map,
							  IN csp::Time	  ring_timeout)
{
	
	
	if (_stackRegistrationHandle  != IW_UNDEFINED)
		key_value_map["registration_id"] =  _stackRegistrationHandle;

	return GenericOfferAnswerSession::MakeCall(destination_uri, offer, credentials, key_value_map, ring_timeout);

}

ApiErrorCode
SipMediaCall::StopRegistration()
{
	FUNCTRACKER;

	LogDebug("SipMediaCall::StopRegistration");

	if (_stackRegistrationHandle == IW_UNDEFINED)
	{
		return API_WRONG_STATE;
	}

	MsgSipCallUnRegisterReq *msg = new MsgSipCallUnRegisterReq();
	msg->registration_id  = _stackRegistrationHandle;

	_stackRegistrationHandle = IW_UNDEFINED;
	

	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = API_SUCCESS;

	res = GetCurrRunningContext()->SendMessage(
		_serviceHandleId,
		IwMessagePtr(msg));

	return res;
}

ApiErrorCode
SipMediaCall::StartRegistration(IN const ContactsList &contacts, 
								IN const string		  &registrar,
								IN const Credentials  &credentials, 
								IN csp::Time timeout)
{
	FUNCTRACKER;

	LogDebug("SipMediaCall::StartRegistration username:" << credentials.username);

	if (_stackRegistrationHandle != IW_UNDEFINED)
	{
		return API_WRONG_STATE;
	}

	MsgSipCallRegisterReq *msg = new MsgSipCallRegisterReq();
	msg->contacts  = contacts;
	msg->registrar = registrar;
	msg->credentials  = credentials;
	

	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = API_SUCCESS;
	
	res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_serviceHandleId,
		IwMessagePtr(msg),
		response,
		timeout,
		"SIP Register TXN");

	if (res != API_SUCCESS)
	{
		LogWarn("SipMediaCall::StartRegistration - failure:" << res);
		return res;
	}

	switch (response->message_id)
	{
		case SIP_CALL_REGISTER_ACK:
			{
				shared_ptr<MsgSipCallRegisterAck> ack = 
					dynamic_pointer_cast<MsgSipCallRegisterAck>(response);

				_stackRegistrationHandle = ack->registration_id;
				break;
			}
	default:
		{
			return API_SERVER_FAILURE;
		}
	}

	return res;
}


 
void 
SipMediaCall::CleanNotifyBuffer()
{
	FUNCTRACKER;

	LogDebug("SipMediaCall::CleanNotifyBuffer iwh:" << _iwCallHandle);

	// the trick we are using
	// to clean the buffer is just replace the handle
	// with  new one
	_notifyChannel->Poison();
	_notifyChannel = LpHandlePtr(new LpHandle());

}

void 
SipMediaCall::CleanInfoBuffer()
{
	FUNCTRACKER;

	LogDebug("SipMediaCall::CleanNotifyBuffer iwh:" << _iwCallHandle);

	// the trick we are using
	// to clean the buffer is just replace the handle
	// with  new one
	_infosChannel->Poison();
	_infosChannel = LpHandlePtr(new LpHandle());

}

ApiErrorCode
SipMediaCall::Subscribe(IN const string			&eventserver,
						IN const ContactsList	&contacts, 
						IN const Credentials	&credentials, 
						IN const AbstractOffer	&offer,
						IN const string			&eventsPackage,
						IN int					refreshInterval,
						IN int					subscriptionTime,
						IN csp::Time			timeout)
{
	FUNCTRACKER;

	LogDebug("SipMediaCall::Subscribe username:" << credentials.username);

	if (_stackSubscribeHandle != IW_UNDEFINED)
	{
		return API_WRONG_STATE;
	}

	MsgSipCallSubscribeReq *msg = new MsgSipCallSubscribeReq();
	msg->contacts		= contacts;
	msg->dest			= eventserver;
	msg->credentials	= credentials;
	msg->events_package = eventsPackage;
	msg->offer			= offer;
	msg->stack_call_handle	= _iwCallHandle;
	msg->subscription_time	= subscriptionTime;
	msg->refresh_interval	= refreshInterval;
	msg->call_handler_inbound	= _handlerPair.inbound;


	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = API_SUCCESS;

	res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_serviceHandleId,
		IwMessagePtr(msg),
		response,
		timeout,
		"SIP Subscribe TXN");

	if (res != API_SUCCESS)
	{
		LogWarn("ipMediaCall::Subscribe - failure:" << res);
		return res;
	}

	switch (response->message_id)
	{
	case SIP_CALL_SUBSCRIBE_ACK:
		{
			shared_ptr<MsgSipCallSubscribeAck> ack = 
				dynamic_pointer_cast<MsgSipCallSubscribeAck>(response);

			if (_serviceHandleId == IW_UNDEFINED)
				_serviceHandleId= ack->stack_call_handle;

			_stackSubscribeHandle = ack->subscription_handle;

			res = API_SUCCESS;

			break;
		}
	default:
		{
			return API_SERVER_FAILURE;
		}
	}

	return res;
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
			_serviceHandleId,
			IwMessagePtr(msg),
			response,
			Seconds(10),
			"SIP SendInfo TXN");

	} else
	{
		res = GetCurrRunningContext()->SendMessage(
			_serviceHandleId,
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
				_infosChannel->Send(ptr);
			break;
		}
	case SIP_CALL_NOTIFY_EVT:
		{
			if (_notifyChannel)
				_notifyChannel->Send(ptr);
			break;
		}
	default:
		{
			GenericOfferAnswerSession::UponActiveObjectEvent(ptr);
		}
	}

	
}

}

#pragma pop_macro("SendMessage")

