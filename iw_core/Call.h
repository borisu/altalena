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

#include "LightweightProcess.h"
#include "ActiveObject.h"

namespace ivrworx
{

#pragma region Sip_Stack_Events

	enum CallEvts
	{
		MSG_CALL_OFFERED = MSG_USER_DEFINED,
		MSG_CALL_OFFERED_ACK,
		MSG_CALL_OFFERED_NACK,
		MSG_CALL_CONNECTED,
		MSG_CALL_HANG_UP_EVT,
		MSG_CALL_TERMINATED,
		MSG_CALL_DTMF_EVT,

		MSG_MAKE_CALL_REQ,
		MSG_MAKE_CALL_ACK,
		MSG_MAKE_CALL_NACK,
		MSG_HANGUP_CALL_REQ,

		MSG_CALL_BLIND_XFER_REQ,
		MSG_CALL_BLIND_XFER_ACK,
		MSG_CALL_BLIND_XFER_NACK,

	};


	//
	// CALL RELATED
	//
	class MsgMakeCallReq : 
		public MsgRequest
	{

	public:
		MsgMakeCallReq():
		  MsgRequest(MSG_MAKE_CALL_REQ, NAME(MSG_MAKE_CALL_REQ)){};

		  string destination_uri;

		  string on_behalf_of;

		  CnxInfo local_media;

		  LpHandlePtr call_handler_inbound;

	};


	class MsgMakeCallAck: 
		public IwMessage
	{
	public:

		MsgMakeCallAck(): 
		  IwMessage(MSG_MAKE_CALL_ACK, NAME(MSG_MAKE_CALL_ACK)),
			  stack_call_handle(IW_UNDEFINED){};

		  MsgMakeCallAck(int handle):
		  IwMessage(MSG_MAKE_CALL_ACK, NAME(MSG_MAKE_CALL_ACK)),
			  stack_call_handle(handle){};

		  int stack_call_handle;

		  CnxInfo remote_media;
	};


	class MsgMakeCallNack: 
		public IwMessage
	{
	public:
		MsgMakeCallNack():
		  IwMessage(MSG_MAKE_CALL_NACK, NAME(MSG_MAKE_CALL_NACK)){};

		  unsigned long stack_call_handle;
	};


	class MsgHangupCallReq: 
		public MsgRequest
	{
	public:
		MsgHangupCallReq():
		  MsgRequest(MSG_HANGUP_CALL_REQ, NAME(MSG_HANGUP_CALL_REQ)),
			  stack_call_handle(IW_UNDEFINED){};

		  MsgHangupCallReq(int handle):
		  MsgRequest(MSG_HANGUP_CALL_REQ, NAME(MSG_HANGUP_CALL_REQ)),
			  stack_call_handle(handle){};

		  unsigned long stack_call_handle;
	};


	class  MsgStackMixin 
	{
	public:

		MsgStackMixin():
		 stack_call_handle(IW_UNDEFINED){};

		int stack_call_handle;

		CnxInfo local_media;

		MediaFormatsList offered_codecs;

		MediaFormatsList accepted_codecs;

		CnxInfo remote_media;

		string ani;

		string dnis;

		virtual void copy_data_on_response(IN IwMessage *msg)
		{
			MsgStackMixin *req = dynamic_cast<MsgStackMixin*>(msg);

			stack_call_handle	= req->stack_call_handle;
			local_media			= local_media.is_ip_valid()  ? req->local_media : local_media;
			remote_media		= remote_media.is_ip_valid() ? req->remote_media : remote_media;
			offered_codecs      = req->offered_codecs;
			accepted_codecs		= req->accepted_codecs;
			ani					= req->ani;
			dnis				= req->dnis;
		};

	};


	enum OfferType
	{
		OFFER_TYPE_INITIAL_OFFER,
		OFFER_TYPE_KEEP_ALIVE,
		OFFER_TYPE_HOLD,
		OFFER_TYPE_RESUME
	};

	class  MsgCallOfferedMixin 
	{
	public:

		MsgCallOfferedMixin():invite_type(OFFER_TYPE_INITIAL_OFFER),is_indialog(FALSE){};

		OfferType invite_type;

		BOOL is_indialog;

		virtual void copy_data_on_response(IN IwMessage *request)
		{
			MsgCallOfferedMixin *req = dynamic_cast<MsgCallOfferedMixin*>(request);
			is_indialog = req->is_indialog;
			invite_type = req->invite_type;
		}
	};

	class MsgCallOfferedReq:
		public MsgStackMixin, public MsgRequest, public MsgCallOfferedMixin
	{
	public:
		MsgCallOfferedReq():
		  MsgRequest(MSG_CALL_OFFERED, 
			NAME(MSG_CALL_OFFERED)){}

		LpHandlePair call_handler_inbound;

		MediaFormatsList offered_codecs;
	};


	class MsgCalOfferedAck:
		public MsgStackMixin, public IwMessage, public MsgCallOfferedMixin
	{
	public:
		MsgCalOfferedAck():IwMessage(MSG_CALL_OFFERED_ACK, 
			NAME(MSG_CALL_OFFERED_ACK)){}

		virtual void copy_data_on_response(IN IwMessage *request)
		{
			MsgCallOfferedMixin::copy_data_on_response(request);
			MsgStackMixin::copy_data_on_response(request);
			IwMessage::copy_data_on_response(request);
		}
	};


	class MsgCallOfferedNack:
		public MsgStackMixin, public IwMessage, public MsgCallOfferedMixin
	{
	public:
		MsgCallOfferedNack():
		  IwMessage(MSG_CALL_OFFERED_NACK, NAME(MSG_CALL_OFFERED_NACK))
			  ,code(API_FAILURE){}

		  virtual void copy_data_on_response(IN IwMessage *request)
		  {
			  MsgCallOfferedMixin::copy_data_on_response(request);
			  MsgStackMixin::copy_data_on_response(request);
			  IwMessage::copy_data_on_response(request);
		  }

		  ApiErrorCode code;

	};


	class MsgNewCallConnected:
		public MsgStackMixin, public IwMessage, public MsgCallOfferedMixin
	{
	public:
		MsgNewCallConnected():IwMessage(MSG_CALL_CONNECTED, 
			NAME(MSG_CALL_CONNECTED)){}

		virtual void copy_data_on_response(IN IwMessage *request)
		{
			MsgCallOfferedMixin::copy_data_on_response(request);
			MsgStackMixin::copy_data_on_response(request);
			IwMessage::copy_data_on_response(request);
		}

	};

	class MsgCallHangupEvt:
		public MsgStackMixin,public IwMessage
	{
	public:
		MsgCallHangupEvt():IwMessage(MSG_CALL_HANG_UP_EVT, 
			NAME(MSG_CALL_HANG_UP_EVT)){}

	};


	class MsgCallBlindXferReq:
		public MsgStackMixin,public IwMessage
	{
	public:
		MsgCallBlindXferReq():IwMessage(MSG_CALL_BLIND_XFER_REQ, 
			NAME(MSG_CALL_BLIND_XFER_REQ)){}

		string destination_uri;

	};

	class MsgCallBlindXferAck:
		public MsgStackMixin,public IwMessage
	{
	public:
		MsgCallBlindXferAck():IwMessage(MSG_CALL_BLIND_XFER_ACK, 
			NAME(MSG_CALL_BLIND_XFER_ACK)){}

	};

	class MsgCallBlindXferNack:
		public MsgStackMixin,public IwMessage
	{
	public:
		MsgCallBlindXferNack():IwMessage(MSG_CALL_BLIND_XFER_NACK, 
			NAME(MSG_CALL_BLIND_XFER_NACK)){}

	};



#pragma endregion Sip_Stack_Events


	enum CallState
	{
		CALL_STATE_UKNOWN,
		CALL_STATE_INITIAL_OFFERED,
		CALL_STATE_IN_DIALOG_OFFERED,
		CALL_STATE_CONNECTED,
		CALL_STATE_HELD,
		CALL_STATE_TERMINATED
	};


	class Call : 
		public ActiveObject
	{
	public:

		Call(IN LpHandlePair _stackPair);

		Call(IN LpHandlePair _stackPair,
			IN ScopedForking &forking,
			IN shared_ptr<MsgCallOfferedReq> offered_msg);

		virtual ~Call(void);

		void EnableMediaFormat(IN const MediaFormat& codec);

		void UponCallTerminated(IwMessagePtr ptr);

		ApiErrorCode NegotiateMediaFormats(
			IN const MediaFormatsList &offered_medias, 
			OUT MediaFormatsList &accepted_media,
			OUT MediaFormat &accepted_speech_format);

		ApiErrorCode AcceptInitialOffer(
			IN const CnxInfo &local_media, 
			IN const MediaFormatsList &accepted_codec,
			IN const MediaFormat &speech_codec);

		ApiErrorCode RejectCall();

		ApiErrorCode MakeCall(
			IN const string &destination_uri, 
			IN const CnxInfo &local_media);

		const string& Dnis();

		const string& Ani();

		ApiErrorCode HagupCall();

		ApiErrorCode BlindXfer(IN const string &destination_uri);

		CnxInfo RemoteMedia() const;
		void RemoteMedia(CnxInfo &val);

		CnxInfo LocalMedia() const;
		void LocalMedia(CnxInfo &val);

		int StackCallHandle() const;

		void UponActiveObjectEvent(IwMessagePtr ptr);

	protected:

		void ResetState(CallState state, const char *state_name);

		LpHandlePair _handlerPair;

		LpHandlePair _stackPair;

		int _stackCallHandle;

		CnxInfo _remoteOfferedMedia;

		CnxInfo _remoteMedia;

		CnxInfo _localMedia;

		BOOL _hangupDetected;

		LpHandlePtr _dtmfChannel;

		CallState _callState;

		MediaFormatsMap _supportedMediaFormatsList;

		MediaFormat _acceptedSpeechFormat;

		MediaFormatsList _acceptedMediaFormats;

		string _dnis;

		string _ani;

	};

	typedef 
	shared_ptr<Call> CallPtr;

}




