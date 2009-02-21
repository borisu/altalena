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

#pragma region Sip_Stack_Events

enum CallEvts
{
	CCU_MSG_CALL_OFFERED = CCU_MSG_USER_DEFINED,
	CCU_MSG_CALL_OFFERED_ACK,
	CCU_MSG_CALL_OFFERED_NACK,
	CCU_MSG_CALL_CONNECTED,
	CCU_MSG_CALL_HANG_UP_EVT,
	CCU_MSG_CALL_TERMINATED,
	CCU_MSG_CALL_DTMF_EVT,

	CCU_MSG_MAKE_CALL_REQ,
	CCU_MSG_MAKE_CALL_ACK,
	CCU_MSG_MAKE_CALL_NACK,
	CCU_MSG_HANGUP_CALL_REQ,
};


//
// CALL RELATED
//
class CcuMsgMakeCallReq : 
	public CcuMsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(destination_uri);
		SERIALIZE_FIELD(on_behalf_of);
		SERIALIZE_FIELD(local_media);
	}
public:
	CcuMsgMakeCallReq():
	  CcuMsgRequest(CCU_MSG_MAKE_CALL_REQ, NAME(CCU_MSG_MAKE_CALL_REQ)){};

	  wstring destination_uri;

	  wstring on_behalf_of;

	  CnxInfo local_media;

#pragma TODO ("TODO:This should support IPC IpcAddress instead of handle")

	  LpHandlePtr call_handler_inbound;

};
BOOST_CLASS_EXPORT(CcuMsgMakeCallReq);

class CcuMsgMakeCallAck: 
	public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(stack_call_handle);
		SERIALIZE_FIELD(remote_media);
	}
public:

	CcuMsgMakeCallAck(): 
	  IxMessage(CCU_MSG_MAKE_CALL_ACK, NAME(CCU_MSG_MAKE_CALL_ACK)),
		  stack_call_handle(IX_UNDEFINED){};

	  CcuMsgMakeCallAck(int handle):
	  IxMessage(CCU_MSG_MAKE_CALL_ACK, NAME(CCU_MSG_MAKE_CALL_ACK)),
		  stack_call_handle(handle){};

	  int stack_call_handle;

	  CnxInfo remote_media;
};
BOOST_CLASS_EXPORT(CcuMsgMakeCallAck);

class CcuMsgMakeCallNack: 
	public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(stack_call_handle);
	}
public:
	CcuMsgMakeCallNack():
	  IxMessage(CCU_MSG_MAKE_CALL_NACK, NAME(CCU_MSG_MAKE_CALL_NACK)){};

	  unsigned long stack_call_handle;
};
BOOST_CLASS_EXPORT(CcuMsgMakeCallNack);

class CcuMsgHangupCallReq: 
	public CcuMsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(stack_call_handle);
	}
public:
	CcuMsgHangupCallReq():
	  CcuMsgRequest(CCU_MSG_HANGUP_CALL_REQ, NAME(CCU_MSG_HANGUP_CALL_REQ)),
		  stack_call_handle(IX_UNDEFINED){};

	  CcuMsgHangupCallReq(int handle):
	  CcuMsgRequest(CCU_MSG_HANGUP_CALL_REQ, NAME(CCU_MSG_HANGUP_CALL_REQ)),
		  stack_call_handle(handle){};

	  unsigned long stack_call_handle;
};
BOOST_CLASS_EXPORT(CcuMsgHangupCallReq);

class  CcuMsgStackMixin 
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_FIELD(stack_call_handle);
		SERIALIZE_FIELD(local_media);
		SERIALIZE_FIELD(remote_media);
	}

public:

	int stack_call_handle;

	CnxInfo local_media;

	MediaFormatsList offered_codecs;

	MediaFormatsList accepted_codecs;

	CnxInfo remote_media;

	virtual void copy_data_on_response(IN IxMessage *msg)
	{
		CcuMsgStackMixin *req = dynamic_cast<CcuMsgStackMixin*>(msg);

		stack_call_handle	= req->stack_call_handle;
		local_media			= local_media.is_ip_valid()  ? req->local_media : local_media;
		remote_media		= remote_media.is_ip_valid() ? req->remote_media : remote_media;
		offered_codecs      = req->offered_codecs;
		accepted_codecs		= req->accepted_codecs;

	};

};
BOOST_IS_ABSTRACT(CcuMsgStackMixin);

class CcuMsgCallOfferedReq:
	public CcuMsgStackMixin, public CcuMsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(IxMessage);
	}
public:
	CcuMsgCallOfferedReq():CcuMsgRequest(CCU_MSG_CALL_OFFERED, 
		NAME(CCU_MSG_CALL_OFFERED)){}

	LpHandlePair call_handler_inbound;

	MediaFormatsList offered_codecs;

};
BOOST_CLASS_EXPORT(CcuMsgCallOfferedReq);

class CcuMsgCalOfferedlAck:
	public CcuMsgStackMixin,public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(IxMessage);
	}
public:
	CcuMsgCalOfferedlAck():IxMessage(CCU_MSG_CALL_OFFERED_ACK, 
		NAME(CCU_MSG_CALL_OFFERED_ACK)){}

	virtual void copy_data_on_response(IN IxMessage *request)
	{
		CcuMsgStackMixin::copy_data_on_response(request);
		IxMessage::copy_data_on_response(request);
	}
};
BOOST_CLASS_EXPORT(CcuMsgCalOfferedlAck);

class CcuMsgCallOfferedNack:
	public CcuMsgStackMixin, public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(code);
	}
public:
	CcuMsgCallOfferedNack():
	  IxMessage(CCU_MSG_CALL_OFFERED_NACK, NAME(CCU_MSG_CALL_OFFERED_NACK))
		  ,code(CCU_API_FAILURE){}

	  virtual void copy_data_on_response(IN IxMessage *request)
	  {
		  CcuMsgStackMixin::copy_data_on_response(request);
		  IxMessage::copy_data_on_response(request);
	  }

	  IxApiErrorCode code;

};
BOOST_CLASS_EXPORT(CcuMsgCallOfferedNack);

class CcuMsgNewCallConnected:
	public CcuMsgStackMixin,public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(IxMessage);
	}

public:
	CcuMsgNewCallConnected():IxMessage(CCU_MSG_CALL_CONNECTED, 
		NAME(CCU_MSG_CALL_CONNECTED)){}

	virtual void copy_data_on_response(IN IxMessage *request)
	{
		CcuMsgStackMixin::copy_data_on_response(request);
		IxMessage::copy_data_on_response(request);
	}

};
BOOST_CLASS_EXPORT(CcuMsgNewCallConnected);

class CcuMsgCallHangupEvt:
	public CcuMsgStackMixin,public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(IxMessage);
	}

public:
	CcuMsgCallHangupEvt():IxMessage(CCU_MSG_CALL_HANG_UP_EVT, 
		NAME(CCU_MSG_CALL_HANG_UP_EVT)){}

};
BOOST_CLASS_EXPORT(CcuMsgCallHangupEvt);


class CcuMsgCallDtmfEvt:
	public CcuMsgStackMixin,public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(dtmf_digit)
	}

public:
	CcuMsgCallDtmfEvt():IxMessage(CCU_MSG_CALL_DTMF_EVT, 
		NAME(CCU_MSG_CALL_DTMF_EVT)){}

	wstring dtmf_digit;

};
BOOST_CLASS_EXPORT(CcuMsgCallDtmfEvt);

#pragma endregion Sip_Stack_Events

namespace ivrworx
{

	enum CallState
	{
		IX_CALL_NONE,
		IX_CALL_OFFERED,
		IX_CALL_CONNECTED,
		IX_CALL_TERMINATED
	};

	
	class Call : 
		public ActiveObject
	{
	public:

		Call(IN LpHandlePair _stackPair);

		Call(IN LpHandlePair _stackPair,
			IN ScopedForking &forking,
			IN shared_ptr<CcuMsgCallOfferedReq> offered_msg);

		virtual ~Call(void);

		void EnableMediaFormat(IN const MediaFormat& codec);

		IxApiErrorCode NegotiateMediaFormats(
			IN const MediaFormatsList &offered_medias, 
			OUT MediaFormatsList &accepted_media);

		IxApiErrorCode AcceptCall(
			IN const CnxInfo &local_media, 
			IN const MediaFormatsList &accepted_codec);

		IxApiErrorCode RejectCall();

		IxApiErrorCode MakeCall(
			IN wstring destination_uri, 
			IN CnxInfo local_media);

		IxApiErrorCode HagupCall();

		IxApiErrorCode WaitForDtmf(
			IN wstring &dtmf_digit,
			IN Time timeout);

		CnxInfo RemoteMedia() const;
		void RemoteMedia(CnxInfo &val);

		CnxInfo LocalMedia() const;
		void LocalMedia(CnxInfo &val);

		int StackCallHandle() const;

		void UponActiveObjectEvent(IxMsgPtr ptr);

	protected:

		LpHandlePair _handlerPair;

		LpHandlePair _stackPair;

		int _stackCallHandle;
		
		CnxInfo _remoteMedia;

		CnxInfo _localMedia;

		BOOL _hangupDetected;

		LpHandle _dtmfChannel;

		CallState _callState;

		MediaFormatsMap _supportedMediaFormatsList;

		MediaFormat _acceptedSpeechFormat;

	};

	typedef 
		shared_ptr<Call> CallPtr;

}




