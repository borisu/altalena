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
using namespace ivrworx;

int GenerateNewTxnId();

enum CcuMessageId
{
	CCU_MSG_MARKER,
	CCU_MSG_UNKNOWN,
	CCU_MSG_TIMEOUT,

	CCU_MSG_ACK,
	CCU_MSG_NACK,

	CCU_MSG_PING,
	CCU_MSG_PONG,
	CCU_MSG_PROC_SHUTDOWN_REQ,
	CCU_MSG_PROC_SHUTDOWN_ACK,
	CCU_MSG_PROC_READY,
	CCU_MSG_PROC_FAILURE,

	CCU_MSG_USER_DEFINED = 5000

};


class IpcAdddress
{
private:

	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_FIELD(handle_id);
		SERIALIZE_FIELD(queue_path);
	}

public:
	IpcAdddress()
	{
		handle_id   = IX_UNDEFINED;
	}

	IpcAdddress(const IpcAdddress& other)
	{
		handle_id		= other.handle_id;
		queue_path	= other.queue_path;
	}

	int handle_id;

	wstring queue_path;

};
BOOST_CLASS_EXPORT(IpcAdddress);


class IxMessage :
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
	IxMessage (int message_id, const wstring message_id_str):
		 transaction_id(-1),
		 is_response(FALSE),
		 preferrable_ipc_interface(IX_UNDEFINED)
	{
		::QueryPerformanceCounter(&enter_queue_timestamp);
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

	IxTimeStamp enter_queue_timestamp;

	virtual ~IxMessage()
	{
		
	}

	virtual void copy_data_on_response(IN IxMessage *request)
	{
		transaction_id = request->transaction_id;
		dest = request->source;
		preferrable_ipc_interface = request->preferrable_ipc_interface;
	};

};
BOOST_IS_ABSTRACT(IxMessage);


typedef 
boost::shared_ptr<IxMessage> IxMsgPtr;

#define CCU_NULL_MSG IxMsgPtr((IxMessage*)NULL)

wstring DumpAsXml(IxMsgPtr msg);

class CcuMsgRequest:
	public IxMessage
{
private:
	BOOST_SERIALIZATION_REGION
	{
		// serialize base class information
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(is_response);
	}

public:
	CcuMsgRequest(int message_id, const wstring message_id_str):
	  IxMessage (message_id, message_id_str)
	  {
		  transaction_id = GenerateNewTxnId();
	  };
};
BOOST_IS_ABSTRACT(CcuMsgRequest);

class CcuMsgResponse:
	public IxMessage
{
private:
	BOOST_SERIALIZATION_REGION
	{
		// serialize base class information
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(is_response);
	}

public:
	CcuMsgResponse(int message_id, const wstring message_id_str):
	  IxMessage (message_id, message_id_str)
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


#pragma region Process_Management_Events

class CcuMsgPing : 
	public CcuMsgRequest
{
private:
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
	}
public:

	CcuMsgPing():
	  CcuMsgRequest(CCU_MSG_PING, NAME(CCU_MSG_PING)){};
};
BOOST_CLASS_EXPORT(CcuMsgPing);

class CcuMsgPong : 
	public CcuMsgResponse
{
private:
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgResponse);
	}
public:
	CcuMsgPong():
	  CcuMsgResponse(CCU_MSG_PONG, NAME(CCU_MSG_PONG)){};
};
BOOST_CLASS_EXPORT(CcuMsgPong);

class CcuMsgShutdownReq: 
	public CcuMsgRequest
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
	}
public:

	CcuMsgShutdownReq():
	  CcuMsgRequest(CCU_MSG_PROC_SHUTDOWN_REQ, NAME(CCU_MSG_PROC_SHUTDOWN_REQ)){};

};
BOOST_CLASS_EXPORT(CcuMsgShutdownReq);

class CcuMsgShutdownAck: 
	public CcuMsgResponse
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMsgResponse);
	}
public:
	CcuMsgShutdownAck():
	  CcuMsgResponse(CCU_MSG_PROC_SHUTDOWN_ACK, NAME(CCU_MSG_PROC_SHUTDOWN_ACK)){};
};
BOOST_CLASS_EXPORT(CcuMsgShutdownAck);

class CcuMsgProcReady:
	public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
	}

public:
	CcuMsgProcReady():
	  IxMessage(CCU_MSG_PROC_READY, 
		  NAME(CCU_MSG_PROC_READY)){}
};
BOOST_CLASS_EXPORT(CcuMsgProcReady);

class CcuMsgProcFailure:
	public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
	}
public:
	CcuMsgProcFailure():
	  IxMessage(CCU_MSG_PROC_FAILURE, 
		  NAME(CCU_MSG_PROC_FAILURE)){}
};
BOOST_CLASS_EXPORT(CcuMsgProcFailure);

#pragma endregion
