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



#pragma region Sip_Stack_Events

enum CallEvts
{
	CCU_MSG_CALL_OFFERED = CCU_MSG_USER_DEFINED,
	CCU_MSG_CALL_OFFERED_ACK,
	CCU_MSG_CALL_OFFERED_NACK,
	CCU_MSG_CALL_CONNECTED,
	CCU_MSG_CALL_HANG_UP_EVT,
	CCU_MSG_CALL_TERMINATED,

	CCU_MSG_MAKE_CALL_REQ,
	CCU_MSG_MAKE_CALL_ACK,
	CCU_MSG_MAKE_CALL_NACK,
	CCU_MSG_HANGUP_CALL_REQ,
};

//
// CALL RELATED
//
class CcuMsgMakeCallReq : 
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(destination_uri);
		SERIALIZE_FIELD(on_behalf_of);
		SERIALIZE_FIELD(local_media);
	}
public:
	CcuMsgMakeCallReq():
	  CcuMessage(CCU_MSG_MAKE_CALL_REQ, NAME(CCU_MSG_MAKE_CALL_REQ)){};

	  wstring destination_uri;

	  wstring on_behalf_of;

	  CcuMediaData local_media;

#pragma TODO ("TODO:This should support IPC IpcAddress instead of handle")

	  LpHandlePtr call_handler_inbound;

};
BOOST_CLASS_EXPORT(CcuMsgMakeCallReq);

class CcuMsgMakeCallAck: 
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(stack_call_handle);
		SERIALIZE_FIELD(remote_media);
	}
public:

	CcuMsgMakeCallAck(): 
	  CcuMessage(CCU_MSG_MAKE_CALL_ACK, NAME(CCU_MSG_MAKE_CALL_ACK)),
		  stack_call_handle(CCU_UNDEFINED){};

	  CcuMsgMakeCallAck(int handle):
	  CcuMessage(CCU_MSG_MAKE_CALL_ACK, NAME(CCU_MSG_MAKE_CALL_ACK)),
		  stack_call_handle(handle){};

	  int stack_call_handle;

	  CcuMediaData remote_media;
};
BOOST_CLASS_EXPORT(CcuMsgMakeCallAck);

class CcuMsgMakeCallNack: 
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(stack_call_handle);
	}
public:
	CcuMsgMakeCallNack():
	  CcuMessage(CCU_MSG_MAKE_CALL_NACK, NAME(CCU_MSG_MAKE_CALL_NACK)){};

	  unsigned long stack_call_handle;
};
BOOST_CLASS_EXPORT(CcuMsgMakeCallNack);

class CcuMsgHangupCallReq: 
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(stack_call_handle);
	}
public:
	CcuMsgHangupCallReq():
	  CcuMessage(CCU_MSG_HANGUP_CALL_REQ, NAME(CCU_MSG_HANGUP_CALL_REQ)),
		  stack_call_handle(CCU_UNDEFINED){};

	  CcuMsgHangupCallReq(int handle):
	  CcuMessage(CCU_MSG_HANGUP_CALL_REQ, NAME(CCU_MSG_HANGUP_CALL_REQ)),
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

	CcuMediaData local_media;

	CcuMediaData remote_media;

	virtual void copy_data_on_response(IN CcuMessage *msg)
	{
		CcuMsgStackMixin *req = dynamic_cast<CcuMsgStackMixin*>(msg);

		stack_call_handle	= req->stack_call_handle;
		local_media			= local_media.is_valid()  ? req->local_media : local_media;
		remote_media		= remote_media.is_valid() ? req->remote_media : remote_media;

	};

};
BOOST_IS_ABSTRACT(CcuMsgStackMixin);

class CcuMsgCallOffered:
	public CcuMsgStackMixin, public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgCallOffered():CcuMessage(CCU_MSG_CALL_OFFERED, 
		NAME(CCU_MSG_CALL_OFFERED)){}
};
BOOST_CLASS_EXPORT(CcuMsgCallOffered);

class CcuMsgCalOfferedlAck:
	public CcuMsgStackMixin,public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgCalOfferedlAck():CcuMessage(CCU_MSG_CALL_OFFERED_ACK, 
		NAME(CCU_MSG_CALL_OFFERED_ACK)){}

	virtual void copy_data_on_response(IN CcuMessage *request)
	{
		CcuMsgStackMixin::copy_data_on_response(request);
		CcuMessage::copy_data_on_response(request);
	}
};
BOOST_CLASS_EXPORT(CcuMsgCalOfferedlAck);

class CcuMsgCallOfferedNack:
	public CcuMsgStackMixin, public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgCallOfferedNack():CcuMessage(CCU_MSG_CALL_OFFERED_NACK, 
		NAME(CCU_MSG_CALL_OFFERED_NACK)){}

};
BOOST_CLASS_EXPORT(CcuMsgCallOfferedNack);

class CcuMsgNewCallConnected:
	public CcuMsgStackMixin,public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(CcuMessage);
	}

public:
	CcuMsgNewCallConnected():CcuMessage(CCU_MSG_CALL_CONNECTED, 
		NAME(CCU_MSG_CALL_CONNECTED)){}

};
BOOST_CLASS_EXPORT(CcuMsgNewCallConnected);

class CcuMsgCallHangupEvt:
	public CcuMsgStackMixin,public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgStackMixin);
		SERIALIZE_BASE_CLASS(CcuMessage);
	}

public:
	CcuMsgCallHangupEvt():CcuMessage(CCU_MSG_CALL_HANG_UP_EVT, 
		NAME(CCU_MSG_CALL_HANG_UP_EVT)){}

};
BOOST_CLASS_EXPORT(CcuMsgCallHangupEvt);

#pragma endregion Sip_Stack_Events

class Call 
{
public:

	Call(IN LpHandlePair _stackPair, 
		IN LightweightProcess &facade);

	Call(
		IN LpHandlePair _stackPair, 
		IN int stack_handle,
		IN CcuMediaData offered_media,
		IN LightweightProcess &facade);

	virtual ~Call(void);

	CcuApiErrorCode AcceptCall(
		IN CcuMediaData local_data);

	CcuApiErrorCode RejectCall();

	CcuApiErrorCode MakeCall(
		IN wstring destination_uri, 
		IN CcuMediaData local_media);

	
	CcuApiErrorCode HagupCall();

	CcuMediaData RemoteMedia() const;
	void RemoteMedia(CcuMediaData &val);

	CcuMediaData LocalMedia() const;
	void LocalMedia(CcuMediaData &val);


	

protected:

	void real_run();

protected:

	LpHandlePair _stackPair;

	int _stackCallHandle;

	CcuMediaData _remoteMedia;

	CcuMediaData _localMedia;

	LightweightProcess &_parentProcess;

	LpHandlePtr _handlerHandle;
	
};

typedef 
shared_ptr<Call> CallPtr;

class ICallHandlerCreator
{
public:
	virtual LpHandlePair CreateCallHandler(
		IN LpHandlePair stack_pair, 
		IN int stack_handle,
		IN CcuMediaData offered_media) = 0;

};

typedef 
shared_ptr <ICallHandlerCreator> ICallHandlerCreatorPtr;

