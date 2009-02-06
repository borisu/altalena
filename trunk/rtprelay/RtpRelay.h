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
#include "CcuRtpSession.h"
#include "CcuConfiguration.h"

enum RtpMessages
{
	CCU_MSG_ALLOCATE_NEW_CONNECTION_REQ = CCU_MSG_USER_DEFINED,
	CCU_MSG_ALLOCATE_NEW_CONNECTION_ACK,
	CCU_MSG_ALLOCATE_NEW_CONNECTION_NACK,

	CCU_MSG_MODIFY_CONNECTION_REQ,

	CCU_MSG_BRIDGE_CONNECTIONS_REQ,
	CCU_MSG_BRIDGE_CONNECTIONS_ACK,
	CCU_MSG_BRIDGE_CONNECTIONS_NACK,

	CCU_CLOSE_CONNECTION_REQ,
	CCU_CLOSE_CONNECTION_ACK,


};

class CcuMsgRtpAllocateNewConnectionReq: 
	public CcuMsgRequest
{
private:
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(remote_end);
	}

public:
	CcuMsgRtpAllocateNewConnectionReq():
	  CcuMsgRequest(CCU_MSG_ALLOCATE_NEW_CONNECTION_REQ, NAME(CCU_MSG_ALLOCATE_NEW_CONNECTION_REQ)){};

	  CnxInfo remote_end;
};
BOOST_CLASS_EXPORT(CcuMsgRtpAllocateNewConnectionReq);



class CcuMsgRtpAllocateConnectionAck: 
	public IxMessage
{
private:
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD( connection_media);
		SERIALIZE_FIELD(connection_id);
	}

public:
	CcuMsgRtpAllocateConnectionAck():
	  IxMessage(CCU_MSG_ALLOCATE_NEW_CONNECTION_ACK, NAME(CCU_MSG_ALLOCATE_NEW_CONNECTION_ACK)){};

	  CnxInfo connection_media;

	  int connection_id;
};
BOOST_CLASS_EXPORT(CcuMsgRtpAllocateConnectionAck);


class CcuMsgRtpAllocateConnectionNack: 
	public IxMessage
{
private:
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
	}
public:
	CcuMsgRtpAllocateConnectionNack():
	  IxMessage(CCU_MSG_ALLOCATE_NEW_CONNECTION_NACK, NAME(CCU_MSG_ALLOCATE_NEW_CONNECTION_NACK)){};

};
BOOST_CLASS_EXPORT(CcuMsgRtpAllocateConnectionNack);

class CcuMsgModifyConnectionReq:
	public CcuMsgRequest
{
private:
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(connection_id);
		SERIALIZE_FIELD(remote_media_data);
	}
public:
	CcuMsgModifyConnectionReq():
	  CcuMsgRequest(CCU_MSG_MODIFY_CONNECTION_REQ, NAME(CCU_MSG_MODIFY_CONNECTION_REQ)){};

	  CcuConnectionId connection_id;

	  CnxInfo remote_media_data;
};
BOOST_CLASS_EXPORT(CcuMsgModifyConnectionReq);


class CcuMsgRtpBridgeConnectionsReq: 
	public CcuMsgRequest
{
private:

	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(connection_id1);
		SERIALIZE_FIELD(connection_id2);
	}
public:
	CcuMsgRtpBridgeConnectionsReq():
	  CcuMsgRequest(CCU_MSG_BRIDGE_CONNECTIONS_REQ, NAME(CCU_MSG_BRIDGE_CONNECTIONS_REQ)),
		  connection_id1(IX_UNDEFINED),
		  connection_id2(IX_UNDEFINED){};

	  int connection_id1;

	  int connection_id2;

};
BOOST_CLASS_EXPORT(CcuMsgRtpBridgeConnectionsReq);

class CcuMsgRtpBridgeConnectionsAck: 
	public IxMessage
{
private:
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
	}
public:
	CcuMsgRtpBridgeConnectionsAck():
	  IxMessage(CCU_MSG_BRIDGE_CONNECTIONS_ACK, NAME(CCU_MSG_BRIDGE_CONNECTIONS_ACK)){};

};
BOOST_CLASS_EXPORT(CcuMsgRtpBridgeConnectionsAck);


class CcuMsgRtpBridgeConnectionsNack: 
	public IxMessage
{
private:
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
	}
public:
	CcuMsgRtpBridgeConnectionsNack():
	  IxMessage(CCU_MSG_BRIDGE_CONNECTIONS_NACK, NAME(CCU_MSG_BRIDGE_CONNECTIONS_NACK)){};

};

BOOST_CLASS_EXPORT(CcuMsgRtpBridgeConnectionsNack);

class CcuMsgRtpCloseConnectionReq: 
	public CcuMsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(connection_id);
	}
public:
	CcuMsgRtpCloseConnectionReq():
	  CcuMsgRequest(CCU_CLOSE_CONNECTION_REQ, NAME(CCU_CLOSE_CONNECTION_REQ)){};

	  CcuConnectionId connection_id;

};
BOOST_CLASS_EXPORT(CcuMsgRtpCloseConnectionReq);



class RtpRelayFactory
{
public:
	RtpRelayFactory(void);

	static LightweightProcess* CreateProcRtpRelay(LpHandlePair pair, CnxInfo data);

	static LightweightProcess* CreateProcRtpRelay(IN LpHandlePair pair, IN CcuConfiguration *conf);

	virtual ~RtpRelayFactory(void);

};