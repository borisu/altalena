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
	CCU_MSG_IMS_START_PLAY_REQ_ACK,
	CCU_MSG_IMS_START_PLAY_REQ_NACK,
	CCU_MSG_IMS_PLAY_STOPPED,
};


class CcuMsgAllocateImsSessionReq:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(remote_media_data);
		SERIALIZE_FIELD(file_name);
	}
public:
	CcuMsgAllocateImsSessionReq():
	  CcuMessage(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST, 
		  NAME(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST)){};

	  CnxInfo remote_media_data;

	  wstring file_name;
};
BOOST_CLASS_EXPORT(CcuMsgAllocateImsSessionReq)

class CcuMsgAllocateImsSessionAck:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(playback_handle);
		SERIALIZE_FIELD(ims_media);
	}
public:
	CcuMsgAllocateImsSessionAck():
	  CcuMessage(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK, 
		  NAME(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK)),
	  playback_handle(CCU_UNDEFINED){};

	  int playback_handle;

	  CnxInfo ims_media;
};
BOOST_CLASS_EXPORT(CcuMsgAllocateImsSessionAck)

class CcuMsgAllocateImsSessionNack:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgAllocateImsSessionNack():
	  CcuMessage(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK, 
		  NAME(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK)){};

};
BOOST_CLASS_EXPORT(CcuMsgAllocateImsSessionNack)

class CcuMsgStartPlayReq:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(playback_handle);
		SERIALIZE_FIELD(send_provisional);
	}
public:
	CcuMsgStartPlayReq():
	  CcuMessage(CCU_MSG_START_PLAYBACK_REQUEST, NAME(CCU_MSG_START_PLAYBACK_REQUEST)),
	  playback_handle(CCU_UNDEFINED),
	  send_provisional(false){};

	  int playback_handle;

	  bool send_provisional;
};
BOOST_CLASS_EXPORT(CcuMsgStartPlayReq)

class CcuMsgImsStartPlayReqAck:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgImsStartPlayReqAck():
	  CcuMessage(CCU_MSG_IMS_START_PLAY_REQ_ACK, NAME(CCU_MSG_IMS_START_PLAY_REQ_ACK)){};

};
BOOST_CLASS_EXPORT(CcuMsgImsStartPlayReqAck)

class CcuMsgStartPlayReqNack:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(playback_handle);
	}
public:
	CcuMsgStartPlayReqNack():
	  CcuMessage(CCU_MSG_IMS_START_PLAY_REQ_NACK, NAME(CCU_MSG_IMS_START_PLAY_REQ_NACK)),
	  playback_handle(CCU_UNDEFINED){};

	  int playback_handle;
};
BOOST_CLASS_EXPORT(CcuMsgStartPlayReqNack)


class CcuMsgImsPlayStopped:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(playback_handle);
		SERIALIZE_FIELD(error);
	}
public:
	CcuMsgImsPlayStopped():
	  CcuMessage(CCU_MSG_IMS_PLAY_STOPPED, NAME(CCU_MSG_IMS_PLAY_STOPPED)),
	  playback_handle(CCU_UNDEFINED),
	  error(CCU_API_FAILURE){};

	  int playback_handle;

	  CcuApiErrorCode error;
};
BOOST_CLASS_EXPORT(CcuMsgImsPlayStopped)

#pragma endregion Playback_Events




