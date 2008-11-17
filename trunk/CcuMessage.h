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

#include "CcuCommon.h"


using namespace std;
using namespace boost;
using namespace boost::serialization;


enum CcuMessageId
{
	CCU_MSG_MARKER,
	CCU_MSG_UNKNOWN,
	CCU_MSG_TIMEOUT,

	CCU_MSG_ACK,
	CCU_MSG_NACK,

// 	CCU_MSG_ALLOCATE_NEW_CONNECTION,
// 	CCU_MSG_ALLOCATE_NEW_CONNECTION_SUCCESS,
// 	
// 	CCU_MSG_ALLOCATE_NEW_CONNECTION_FAILURE,
// 	CCU_MSG_MODIFY_CONNECTION,
// 
// 	CCU_MSG_BRIDGE_CONNECTIONS,
// 	CCU_MSG_BRIDGE_CONNECTIONS_SUCCESS,
// 	CCU_MSG_BRIDGE_CONNECTIONS_FAILURE,
// 
// 	CCU_CLOSE_CONNECTION,
// 	CCU_CLOSE_CONNECTION_SUCCESS,
// 
// 
// 
// 	CCU_MSG_RTP_SESSIONS_ESTABLISHED,
// 	CCU_MSG_RTP_SESSIONS_FAILURE,


	CCU_MSG_USER_DEFINED = 5000

};

class IpcAdddress
{
private:

	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_FIELD(proc_id);
		SERIALIZE_FIELD(queue_path);
	}

public:
	IpcAdddress()
	{
		proc_id   = CCU_UNDEFINED;
	}

	IpcAdddress(const IpcAdddress& other)
	{
		proc_id		= other.proc_id;
		queue_path	= other.queue_path;
	}

	int proc_id;

	wstring queue_path;

};
BOOST_CLASS_EXPORT(IpcAdddress);


class CcuMessage :
	public boost::noncopyable // must not be copied but only passed as shared pointer or serialized via network
{
private:

	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_FIELD(message_id);
		SERIALIZE_FIELD(message_id_str);
		SERIALIZE_FIELD(source);
		SERIALIZE_FIELD(dest);
		SERIALIZE_FIELD(transaction_id);
		SERIALIZE_FIELD(is_response);
	}

protected:
	CcuMessage (int message_id, const wstring message_id_str):
		 transaction_id(-1),
		 is_response(FALSE),
		 preferrable_ipc_interface(CCU_UNDEFINED)
	{
		this->message_id = message_id;
		this->message_id_str = message_id_str;
	};
public:

	int message_id;

	wstring message_id_str;

	IpcAdddress source;

	IpcAdddress dest;

	int transaction_id;

	int preferrable_ipc_interface;

	BOOL is_response;

	virtual ~CcuMessage()
	{
		
	}

	virtual void copy_data_on_response(IN CcuMessage *request)
	{
		transaction_id = request->transaction_id;
		dest = request->source;
		preferrable_ipc_interface = request->preferrable_ipc_interface;
	};

};
BOOST_IS_ABSTRACT(CcuMessage);


typedef 
boost::shared_ptr<CcuMessage> CcuMsgPtr;

#define CCU_NULL_MSG CcuMsgPtr((CcuMessage*)NULL)

wstring DumpAsXml(CcuMsgPtr msg);

class CcuMsgResponse:
	public CcuMessage
{
private:

	BOOST_SERIALIZATION_REGION
	{
		// serialize base class information
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(is_response);
	}
public:
	CcuMsgResponse(int message_id, const wstring message_id_str):
	  CcuMessage (message_id, message_id_str)
	  {
		  is_response = TRUE;
	  }
};
BOOST_IS_ABSTRACT(CcuMsgResponse);


#pragma region Simpletons

class CcuMsgAck : 
	public CcuMsgResponse
{
public:
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgResponse);
	}
	CcuMsgAck():
	CcuMsgResponse(CCU_MSG_ACK, NAME(CCU_MSG_ACK)){};
};
BOOST_CLASS_EXPORT(CcuMsgAck);

class CcuMsgNack : 
	public CcuMsgResponse
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgResponse);
	}
public:
	CcuMsgNack():
	  CcuMsgResponse(CCU_MSG_NACK, NAME(CCU_MSG_NACK)){};
};
BOOST_CLASS_EXPORT(CcuMsgNack);

#pragma endregion Simpletons

