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
#include "CcuRTPSession.h"
#include "CcuLogger.h"

CcuRtpSession::CcuRtpSession(LightweightProcess &facade):
_connectionId(CCU_UNDEFINED),
_facade(facade)
{
}

CcuRtpSession::~CcuRtpSession(void)
{
	if (_connectionId != CCU_UNDEFINED)
	{
		CloseRTPConnection();
	}

	_connectionId = -1;
}


CcuApiErrorCode
CcuRtpSession::AllocateRTPConnection()
{
	return AllocateRTPConnection(CcuMediaData());
}

CcuApiErrorCode
CcuRtpSession::AllocateRTPConnection(IN CcuMediaData remote_end)
{
	FUNCTRACKER;

	CcuMsgPtr response = CCU_NULL_MSG;

	EventsSet map;
	map.insert(CCU_MSG_ALLOCATE_NEW_CONNECTION_ACK);
	map.insert(CCU_MSG_ALLOCATE_NEW_CONNECTION_NACK);


	CcuMsgRtpAllocateNewConnectionReq *msg = new CcuMsgRtpAllocateNewConnectionReq();
	msg->remote_end = remote_end;

	CcuApiErrorCode res = _facade.DoRequestResponseTransaction(
		RTP_RELAY_Q,
		CcuMsgPtr(msg),
		map,
		response,
		Time(MilliSeconds(_facade.TransactionTimeout())),
		L"Allocate RTP Connection TXN");

	if (res != CCU_API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_ALLOCATE_NEW_CONNECTION_ACK:
		{
			shared_ptr<CcuMsgRtpAllocateConnectionAck> success 
				= shared_dynamic_cast<CcuMsgRtpAllocateConnectionAck>(response);

			_connectionId = success->connection_id;
			_localMediaData  = success->connection_media;

			break;

		}
	case CCU_MSG_ALLOCATE_NEW_CONNECTION_NACK:
		{
			res = CCU_API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}

	return res;


}

CcuApiErrorCode
CcuRtpSession::ModifyRTPConnection(IN CcuMediaData media_data)
{
	FUNCTRACKER;

	CcuMsgPtr response = CCU_NULL_MSG;

	CcuMsgModifyConnectionReq *msg = new CcuMsgModifyConnectionReq();
	msg->remote_media_data = media_data;
	msg->connection_id = _connectionId;

	EventsSet map;
	map.insert(CCU_MSG_MODIFY_CONNECTION_REQ);
	map.insert(CCU_MSG_ACK);
	map.insert(CCU_MSG_NACK);

	CcuApiErrorCode res = _facade.DoRequestResponseTransaction(
		RTP_RELAY_Q,
		CcuMsgPtr(msg),
		map,
		response,
		MilliSeconds(_facade.TransactionTimeout()),
		L"Modify Connection TXN");

	if (res != CCU_API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_NACK:
		{
			res = CCU_API_SERVER_FAILURE;
			break;
		}
	case CCU_MSG_ACK:
		{
			return CCU_API_SUCCESS;
		}
	default:
		{
			throw;
		}
	}

	return res;
}





CcuApiErrorCode
CcuRtpSession::BridgeRTPConnection(IN const CcuRtpSession &other)
{
	FUNCTRACKER;
	CcuMsgPtr response = CCU_NULL_MSG;

	CcuMsgRtpBridgeConnectionsReq *msg = new CcuMsgRtpBridgeConnectionsReq();
	msg->connection_id1 = _connectionId;
	msg->connection_id2 = other._connectionId;

	EventsSet map;
	map.insert(CCU_MSG_BRIDGE_CONNECTIONS_ACK);
	map.insert(CCU_MSG_BRIDGE_CONNECTIONS_NACK);

	CcuApiErrorCode res = _facade.DoRequestResponseTransaction(
		RTP_RELAY_Q,
		CcuMsgPtr(msg),
		map,
		response,
		MilliSeconds(_facade.TransactionTimeout()),
		L"Bridge RTP Connection TXN");

	if (res != CCU_API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_BRIDGE_CONNECTIONS_ACK:
		{
			break;
		}
	case CCU_MSG_BRIDGE_CONNECTIONS_NACK:
		{
			res = CCU_API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}
	return res;

}

CcuApiErrorCode
CcuRtpSession::CloseRTPConnection()
{
	FUNCTRACKER;

	if (_connectionId != CCU_UNDEFINED)
	{
		return CCU_API_SUCCESS;
	}

	CcuMsgPtr response = CCU_NULL_MSG;

	CcuMsgRtpCloseConnectionReq *msg = new CcuMsgRtpCloseConnectionReq();
	msg->connection_id = _connectionId;


	EventsSet map;
	map.insert(CCU_MSG_ACK);
	map.insert(CCU_MSG_NACK);

	//
	_connectionId = CCU_UNDEFINED;

	CcuApiErrorCode res = _facade.DoRequestResponseTransaction(
		RTP_RELAY_Q,
		CcuMsgPtr(msg),
		map,
		response,
		MilliSeconds(_facade.TransactionTimeout()),
		L"Close Connection RTP Connection TXN");

	if (res != CCU_API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_ACK:
		{
			break;
		}
	case CCU_MSG_NACK:
		{
			res = CCU_API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}
	return res;

}


CcuMediaData 
CcuRtpSession::LocalMediaData() const 
{ 
	return _localMediaData; 
}

void 
CcuRtpSession::LocalMediaData(CcuMediaData val) 
{ 
	_localMediaData = val; 
}


