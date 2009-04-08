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

#include "IwBase.h"

using namespace std;
using namespace boost;
using namespace boost::serialization;

namespace ivrworx
{

	int GenerateNewTxnId();

	enum MessageId
	{
		MSG_MARKER,
		MSG_UNKNOWN,
		MSG_TIMEOUT,

		MSG_ACK,
		MSG_NACK,

		MSG_PING,
		MSG_PONG,
		MSG_PROC_SHUTDOWN_REQ,
		MSG_PROC_SHUTDOWN_ACK,
		MSG_PROC_SHUTDOWN_EVT,
		MSG_PROC_READY,
		MSG_PROC_FAILURE,

		MSG_USER_DEFINED = 5000

	};

	struct IpcAdddress
	{

		IpcAdddress();

		IpcAdddress(IN const IpcAdddress& other);

		virtual ~IpcAdddress();

		int handle_id;

		string queue_path;

	};


	class IwMessage :
	public boost::noncopyable // must not be copied but only passed as shared pointer or serialized via network
	{
		protected:

			IwMessage (IN int message_id, const string &message_id_str);

		public:

			int message_id;

			string message_id_str;

			IpcAdddress source;

			IpcAdddress dest;

			int transaction_id;

			int preferrable_ipc_interface;

			BOOL is_response;

			TimeStamp enter_queue_timestamp;

			virtual ~IwMessage();

		virtual void copy_data_on_response(IN IwMessage *request)
		{
			transaction_id = request->transaction_id;
			dest = request->source;
			preferrable_ipc_interface = request->preferrable_ipc_interface;
		};

	};
	
	typedef 
	boost::shared_ptr<IwMessage> IwMessagePtr;

#define NULL_MSG IwMessagePtr((IwMessage*)NULL)


	class MsgRequest:
		public IwMessage
	{
	public:
		MsgRequest(int message_id, const string message_id_str):
		  IwMessage (message_id, message_id_str)
		  {
			  transaction_id = GenerateNewTxnId();
		  };
	};

	class MsgResponse:
		public IwMessage
	{
	public:
		MsgResponse(int message_id, const string message_id_str):
		  IwMessage (message_id, message_id_str)
		  {
			  is_response = TRUE;
		  }
	};


#pragma region Simpletons

	class MsgAck : 
		public MsgResponse
	{
	public:
		MsgAck():
		MsgResponse(MSG_ACK, NAME(MSG_ACK)){};
	};


	class MsgNack : 
		public MsgResponse
	{
	public:
		MsgNack():
		  MsgResponse(MSG_NACK, NAME(MSG_NACK)){};
	};

#pragma endregion Simpletons


#pragma region Process_Management_Events

	class MsgPing : 
		public MsgRequest
	{
	public:

		MsgPing():
		  MsgRequest(MSG_PING, NAME(MSG_PING)){};
	};


	class MsgPong : 
		public MsgResponse
	{
	public:
		MsgPong():
		  MsgResponse(MSG_PONG, NAME(MSG_PONG)){};
	};


	class MsgShutdownReq: 
		public MsgRequest
	{
	public:
		MsgShutdownReq():
		  MsgRequest(MSG_PROC_SHUTDOWN_REQ, NAME(MSG_PROC_SHUTDOWN_REQ)){};

	};

	class MsgShutdownAck: 
		public MsgResponse
	{
	public:
		MsgShutdownAck():
		  MsgResponse(MSG_PROC_SHUTDOWN_ACK, NAME(MSG_PROC_SHUTDOWN_ACK)){};
	};

	class MsgProcReady:
		public IwMessage
	{
	public:
		MsgProcReady():
		  IwMessage(MSG_PROC_READY, 
			  NAME(MSG_PROC_READY)){}
	};


#pragma endregion Process_Management_Events

}