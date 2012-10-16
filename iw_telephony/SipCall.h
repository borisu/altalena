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

#pragma once

#include "OfferAnswerSession.h"
#include "Telephony.h"

namespace ivrworx
{

	enum SipCallEvts
	{
		SIP_CALL_INFO_REQ = SIP_MSG_BASE,
		SIP_CALL_INFO_ACK,
		SIP_CALL_INFO_NACK,
		SIP_CALL_NOTIFY_EVT,
		SIP_CALL_SUBSCRIBE_REQ,
		SIP_CALL_SUBSCRIBE_ACK,
		SIP_CALL_SUBSCRIBE_NACK,
		SIP_CALL_UNSUBSCRIBE_REQ,
		SIP_CALL_UNSUBSCRIBE_EVT,
		SIP_CALL_REGISTER_REQ,
		SIP_CALL_REGISTER_ACK,
		SIP_CALL_REGISTER_NACK,
		SIP_CALL_UNREGISTER_REQ,
		SIP_CALL_UNREGISTER_EVT

	};

	class IW_TELEPHONY_API MsgSipCallInfoReq:
		public MsgVoipCallMixin, public MsgRequest, public MsgCallOfferedMixin
	{
	public:
		MsgSipCallInfoReq():
		  MsgRequest(SIP_CALL_INFO_REQ, 
			  NAME(SIP_CALL_INFO_REQ)){}

	};

	class IW_TELEPHONY_API MsgSipCallInfoAck:
		public MsgVoipCallMixin, public MsgResponse, public MsgCallOfferedMixin
	{
	public:
		MsgSipCallInfoAck():
		  MsgResponse(SIP_CALL_INFO_ACK, 
			  NAME(SIP_CALL_INFO_ACK)){}

	};

	class IW_TELEPHONY_API MsgSipCallInfoNack:
		public MsgVoipCallMixin, public MsgResponse, public MsgCallOfferedMixin
	{
	public:
		MsgSipCallInfoNack():
		  MsgResponse(SIP_CALL_INFO_NACK, 
			  NAME(SIP_CALL_INFO_NACK)){}

	};


	class IW_TELEPHONY_API RegistrationMixin : 
		public AuthenticationMixin
	{
	public:
		RegistrationMixin():
		  registration_id(IW_UNDEFINED){};

		HandleId registration_id;

	};

	class IW_TELEPHONY_API MsgSipCallSubscribeReq:
		public MsgVoipCallMixin, public MsgRequest, public AuthenticationMixin
	{
	public:
		MsgSipCallSubscribeReq():
		  MsgRequest(SIP_CALL_SUBSCRIBE_REQ, 
			  NAME(SIP_CALL_SUBSCRIBE_REQ)){}

		   std::list<string> contacts;

		   string dest;

		   string events_package;

		   int subscription_time;

		   int refresh_interval;

		   AbstractOffer offer;

		   LpHandlePtr call_handler_inbound;

	};

	class IW_TELEPHONY_API MsgSipCallSubscribeAck:
		public MsgVoipCallMixin, public MsgResponse, public AuthenticationMixin
	{
	public:
		MsgSipCallSubscribeAck():
		  MsgResponse(SIP_CALL_SUBSCRIBE_ACK, 
			  NAME(SIP_CALL_SUBSCRIBE_ACK)){};

		  HandleId subscription_handle;

	};

	class IW_TELEPHONY_API MsgSipCallSubscribeNack:
		public MsgVoipCallMixin, public MsgResponse, public AuthenticationMixin
	{
	public:
		MsgSipCallSubscribeNack():
		  MsgResponse(SIP_CALL_SUBSCRIBE_NACK, 
			  NAME(SIP_CALL_SUBSCRIBE_NACK)){}

	};


	class IW_TELEPHONY_API MsgSipCallRegisterReq:
		public MsgVoipCallMixin, public MsgRequest, public RegistrationMixin
	{
	public:
		MsgSipCallRegisterReq():
		  MsgRequest(SIP_CALL_REGISTER_REQ, 
			  NAME(SIP_CALL_REGISTER_REQ)),
			  max_registration_time(IW_UNDEFINED), 
			  registration_retry_time(IW_UNDEFINED){};

		  std::list<string> contacts;

		  string registrar;

		  int max_registration_time;

		  int registration_retry_time;

	};

	class IW_TELEPHONY_API MsgSipCallRegisterAck:
		public MsgVoipCallMixin, public MsgResponse, public RegistrationMixin
	{
	public:
		MsgSipCallRegisterAck():
		  MsgResponse(SIP_CALL_REGISTER_ACK, 
			  NAME(SIP_CALL_REGISTER_ACK)){}

	};

	class IW_TELEPHONY_API MsgSipCallRegisterNack:
		public MsgVoipCallMixin, public MsgResponse, public RegistrationMixin
	{
	public:
		MsgSipCallRegisterNack():
		  MsgResponse(SIP_CALL_REGISTER_NACK, 
			  NAME(SIP_CALL_REGISTER_NACK)){}

	};

	class IW_TELEPHONY_API MsgSipCallUnRegisterReq:
		public MsgVoipCallMixin, public MsgRequest, public RegistrationMixin
	{
	public:
		MsgSipCallUnRegisterReq():
		  MsgRequest(SIP_CALL_UNREGISTER_REQ, 
			  NAME(SIP_CALL_UNREGISTER_REQ)){};
	 
	};


	class IW_TELEPHONY_API MsgSipCallNotifyEvt:
		public MsgVoipCallMixin, public IwMessage
	{
	public:
		MsgSipCallNotifyEvt():
		  IwMessage(SIP_CALL_NOTIFY_EVT, 
			  NAME(SIP_CALL_NOTIFY_EVT)){};

	};

	class SipMediaCall;

	typedef
	shared_ptr<SipMediaCall> SipMediaCallPtr;

	class IW_TELEPHONY_API SipMediaCall : 
		public GenericOfferAnswerSession
	{
	public:

		SipMediaCall(IN ScopedForking &forking,IN HandleId stack_handle_id);

		SipMediaCall(IN ScopedForking &forking,
			 IN shared_ptr<MsgCallOfferedReq> offered_msg);

		virtual ApiErrorCode SendInfo(
			IN	const AbstractOffer	&offer, 
			OUT AbstractOffer		&response, 
			IN	bool async);

		virtual ApiErrorCode WaitForInfo(OUT AbstractOffer &offer);

		virtual void CleanInfoBuffer();

		virtual ApiErrorCode StartRegistration(
			IN const list<string>	&contacts, 
			IN const string			&registrar,
			IN const Credentials	&credentials, 
			IN csp::Time			timeout);

		virtual ApiErrorCode Subscribe(
			IN const string			&eventserver,
			IN const list<string>	&contacts, 
			IN const Credentials	&credentials, 
			IN const AbstractOffer	&offer,
			IN const string			&eventsPackage,
			IN int					refreshInterval,
			IN int					subscriptionTime,
			IN csp::Time			timeout);

		virtual ApiErrorCode WaitForNotify(
			OUT AbstractOffer		&offer);

		virtual void CleanNotifyBuffer();

		virtual ApiErrorCode MakeCall(
			IN const string			&destination_uri, 
			IN const AbstractOffer	&offer,
			IN const Credentials	&credentials,
			IN OUT MapOfAny			&key_value_map,
			IN csp::Time			ring_timeout);

		virtual ApiErrorCode StopRegistration();

		virtual void UponActiveObjectEvent(IwMessagePtr ptr);

		virtual ~SipMediaCall(void);

	private:

		LpHandlePtr _infosChannel;

		LpHandlePtr _notifyChannel;

		LpHandlePtr _registrationChannel;

		HandleId	_stackRegistrationHandle;

		HandleId	_stackSubscribeHandle;

	};

	


}




