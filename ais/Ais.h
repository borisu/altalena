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

#include "Ccu.h"
#include "LightweightProcess.h"

#pragma region AIS_Events

enum CcuAgentState
{
	CCU_AGENT_STATE_AVAILABLE,
	CCU_AGENT_STATE_UNAVAILABLE
};


class AgentInfo
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_FIELD(media_address);
		SERIALIZE_FIELD(name);
		SERIALIZE_FIELD(state);
	}
public:

	AgentInfo();

	AgentInfo(const AgentInfo &other);

	CcuMediaData media_address;

	wstring name;

	CcuAgentState state;
};
BOOST_CLASS_EXPORT(AgentInfo)

enum CcuAisEvents
{

	CCU_MSG_AGENT_LOGIN_REQ = CCU_MSG_USER_DEFINED,
	CCU_MSG_AGENT_LOGIN_ACK,
	CCU_MSG_AGENT_LOGIN_NACK,

	CCU_MSG_AGENT_LOGOUT_REQ,

	CCU_MSG_AGENT_ALLOCATE_REQ,
	CCU_MSG_AGENT_ALLOCATE_NACK,
	CCU_MSG_AGENT_ALLOCATE_ACK,

	CCU_MSG_AGENT_QUIT_QUEUE_REQ,
	CCU_MSG_AGENT_FREE_REQ
};

class CcuMsgAgentLogin:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(agent_info);
	}
public:
	CcuMsgAgentLogin():
	  CcuMessage(CCU_MSG_AGENT_LOGIN_REQ, 
		  NAME(CCU_MSG_AGENT_LOGIN_REQ)){};

	  AgentInfo agent_info;

};
BOOST_CLASS_EXPORT(CcuMsgAgentLogin)

class CcuMsgAgentLoginAck:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(agent_info);
		SERIALIZE_FIELD(client_handle);
	}
public:
	CcuMsgAgentLoginAck():
	  CcuMessage(CCU_MSG_AGENT_LOGIN_ACK, 
		  NAME(CCU_MSG_AGENT_LOGIN_ACK)),
	  client_handle(CCU_UNDEFINED){};

	  AgentInfo agent_info;

	  CcuHandleId client_handle;

};
BOOST_CLASS_EXPORT(CcuMsgAgentLoginAck)

class CcuMsgAgentLoginNack:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgAgentLoginNack():
	  CcuMessage(CCU_MSG_AGENT_LOGIN_NACK, 
		  NAME(CCU_MSG_AGENT_LOGIN_NACK)){};

};
BOOST_CLASS_EXPORT(CcuMsgAgentLoginNack);

class CcuMsgAgentLogout:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgAgentLogout():
	  CcuMessage(CCU_MSG_AGENT_LOGOUT_REQ, 
		  NAME(CCU_MSG_AGENT_LOGOUT_REQ)){};

	  wstring agent_name;

};
BOOST_CLASS_EXPORT(CcuMsgAgentLogout);

class CcuMsgAgentAllocateReq:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgAgentAllocateReq():
	  CcuMessage(CCU_MSG_AGENT_ALLOCATE_REQ, 
		  NAME(CCU_MSG_AGENT_ALLOCATE_REQ)),
	  handle(CCU_UNDEFINED){};

	  CcuHandleId handle;

};
BOOST_CLASS_EXPORT(CcuMsgAgentAllocateReq);

class CcuMsgAgentAllocateNack:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgAgentAllocateNack():
	  CcuMessage(CCU_MSG_AGENT_ALLOCATE_NACK, 
		  NAME(CCU_MSG_AGENT_ALLOCATE_NACK)){};

};
BOOST_CLASS_EXPORT(CcuMsgAgentAllocateNack);

class CcuMsgAgentAllocateAck:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgAgentAllocateAck():
	  CcuMessage(CCU_MSG_AGENT_ALLOCATE_ACK, 
		  NAME(CCU_MSG_AGENT_ALLOCATE_ACK)){};

	  AgentInfo info;

};
BOOST_CLASS_EXPORT(CcuMsgAgentAllocateAck);

class CcuMsgAgentFree:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgAgentFree():
	  CcuMessage(CCU_MSG_AGENT_FREE_REQ, 
		  NAME(CCU_MSG_AGENT_FREE_REQ)){};

};

class CcuMsgAgentQuitQueue:
	public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:
	CcuMsgAgentQuitQueue():
	  CcuMessage(CCU_MSG_AGENT_QUIT_QUEUE_REQ, 
		  NAME(CCU_MSG_AGENT_QUIT_QUEUE_REQ)){};

};

#pragma endregion AIS_Events

class AisFactory
{

public:
	AisFactory(void);

	virtual ~AisFactory(void);

	static LightweightProcess *CreateProcAis(LpHandlePair pair);

};
