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

#include "ImsSession.h"
#include "ImsFactory.h"


#pragma region Playback_Events

enum ImsEvents
{
	CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST = CCU_MSG_USER_DEFINED,
	CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK,
	CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK,
	CCU_MSG_START_PLAYBACK_REQUEST,
	CCU_MSG_START_PLAY_REQ_ACK,
	CCU_MSG_IMS_START_PLAY_REQ_NACK,
	CCU_MSG_STOP_PLAYBACK_REQUEST,
	CCU_MSG_STOP_PLAYBACK_REQUEST_ACK,
	CCU_MSG_STOP_PLAYBACK_REQUEST_NACK,
	CCU_MSG_IMS_PLAY_STOPPED,
	CCU_MSG_IMS_TEARDOWN_REQ
};


class CcuMsgAllocateImsSessionReq:
	public MsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
		SERIALIZE_FIELD(remote_media_data);
	}
public:
	CcuMsgAllocateImsSessionReq():
	  MsgRequest(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST, 
		  NAME(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST)){};

	 CnxInfo remote_media_data;

	 MediaFormat codec;

	 LpHandlePair session_handler;

};
BOOST_CLASS_EXPORT(CcuMsgAllocateImsSessionReq)

class CcuMsgAllocateImsSessionAck:
	public IwMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
		SERIALIZE_FIELD(playback_handle);
	}
public:
	CcuMsgAllocateImsSessionAck():
	  IwMessage(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK, 
		  NAME(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK)),
	  playback_handle(IW_UNDEFINED){};

	  int playback_handle;

	  CnxInfo ims_media_data;


	 
};
BOOST_CLASS_EXPORT(CcuMsgAllocateImsSessionAck)

class CcuMsgAllocateImsSessionNack:
	public IwMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
	}
public:
	CcuMsgAllocateImsSessionNack():
	  IwMessage(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK, 
		  NAME(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK)){};

};
BOOST_CLASS_EXPORT(CcuMsgAllocateImsSessionNack)

class CcuMsgStartPlayReq:
	public MsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
		SERIALIZE_FIELD(playback_handle);
		SERIALIZE_FIELD(loop);
	}
public:
	CcuMsgStartPlayReq():
	  MsgRequest(CCU_MSG_START_PLAYBACK_REQUEST, NAME(CCU_MSG_START_PLAYBACK_REQUEST)),
	  playback_handle(IW_UNDEFINED),loop(false),send_provisional(false) {};

	  int playback_handle;

	  wstring file_name;

	  BOOL loop;

	  BOOL send_provisional;

};
BOOST_CLASS_EXPORT(CcuMsgStartPlayReq)

class CcuMsgStartPlayReqAck:
	public IwMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
	}
public:
	CcuMsgStartPlayReqAck():
	  IwMessage(CCU_MSG_START_PLAY_REQ_ACK, NAME(CCU_MSG_START_PLAY_REQ_ACK)){};

};
BOOST_CLASS_EXPORT(CcuMsgStartPlayReqAck)

class CcuMsgStartPlayReqNack:
	public IwMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
		SERIALIZE_FIELD(playback_handle);
	}
public:
	CcuMsgStartPlayReqNack():
	  IwMessage(CCU_MSG_IMS_START_PLAY_REQ_NACK, NAME(CCU_MSG_IMS_START_PLAY_REQ_NACK)),
	  playback_handle(IW_UNDEFINED){};

	  int playback_handle;
};
BOOST_CLASS_EXPORT(CcuMsgStartPlayReqNack)


class CcuMsgImsPlayStopped:
	public IwMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
		SERIALIZE_FIELD(playback_handle);
		SERIALIZE_FIELD(error);
	}
public:
	CcuMsgImsPlayStopped():
	  IwMessage(CCU_MSG_IMS_PLAY_STOPPED, NAME(CCU_MSG_IMS_PLAY_STOPPED)),
	  playback_handle(IW_UNDEFINED),
	  error(CCU_API_FAILURE){};

	  int playback_handle;

	  IxApiErrorCode error;
};
BOOST_CLASS_EXPORT(CcuMsgImsPlayStopped)

class CcuMsgImsTearDownReq:
	public MsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
		SERIALIZE_FIELD(handle);
	}
public:
	CcuMsgImsTearDownReq():
	  MsgRequest(CCU_MSG_IMS_TEARDOWN_REQ, 
		  NAME(CCU_MSG_IMS_TEARDOWN_REQ)){};

	  ImsHandleId handle;

};
BOOST_CLASS_EXPORT(CcuMsgImsTearDownReq)

class CcuMsgStopPlaybackReq:
	public MsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
		SERIALIZE_FIELD(handle);
	}
public:
	CcuMsgStopPlaybackReq():
	  MsgRequest(CCU_MSG_STOP_PLAYBACK_REQUEST, 
		  NAME(CCU_MSG_STOP_PLAYBACK_REQUEST)){};

	  ImsHandleId handle;

};
BOOST_CLASS_EXPORT(CcuMsgStopPlaybackReq)

class CcuMsgStopPlaybackAck:
	public MsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
	}
public:
	CcuMsgStopPlaybackAck():
	  MsgRequest(CCU_MSG_STOP_PLAYBACK_REQUEST_ACK, 
		  NAME(CCU_MSG_STOP_PLAYBACK_REQUEST_ACK)){};

};
BOOST_CLASS_EXPORT(CcuMsgStopPlaybackAck)

class CcuMsgStopPlaybackNack:
	public MsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IwMessage);
	}
public:
	CcuMsgStopPlaybackNack():
	  MsgRequest(CCU_MSG_STOP_PLAYBACK_REQUEST_NACK, 
		  NAME(CCU_MSG_STOP_PLAYBACK_REQUEST_NACK)){};

};
BOOST_CLASS_EXPORT(CcuMsgStopPlaybackNack)

#pragma endregion Playback_Events




