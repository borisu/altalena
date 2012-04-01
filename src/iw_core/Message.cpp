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
#include "Message.h"

using namespace std;
using namespace boost;

namespace ivrworx
{
	int GenerateNewTxnId()
	{
		static long txn_counter = 0;

		return ::InterlockedExchangeAdd(&txn_counter,1);

	}

	IpcAdddress::IpcAdddress():
	handle_id(IW_UNDEFINED)
	{
		
	}

	IpcAdddress::IpcAdddress(IN const IpcAdddress& other)
	{
		handle_id		= other.handle_id;
		queue_path		= other.queue_path;
	}

	IpcAdddress::~IpcAdddress()
	{

	}

	

	IwMessage::IwMessage (IN int message_id, IN const string &message_id_str):
	transaction_id(-1),
	is_response(FALSE),
	preferrable_ipc_interface(IW_UNDEFINED)
	{
		::QueryPerformanceCounter(&enter_queue_timestamp);
		this->message_id = message_id;
		this->message_id_str = message_id_str;
	};

	IwMessage::~IwMessage()
	{

	}


	
}

