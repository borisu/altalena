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
#include "Message.h"
#include "LightweightProcess.h"

namespace ivrworx
{

enum HandleWaiterMessages
{
	MSG_WAITER_SUBMIT = MSG_USER_DEFINED,
	MSG_WAIT_ACK
};


class MsgWaiterSubmitReq: 
	public MsgRequest
{

public:
	MsgWaiterSubmitReq():
	  MsgRequest(MSG_WAITER_SUBMIT, NAME(MSG_WAITER_SUBMIT)),
		  handle_to_wait(NULL),
			timeout(0){};

	HANDLE handle_to_wait;

	DWORD timeout;
};


class MsgWaiterAck: 
	public MsgResponse
{

public:
	MsgWaiterAck():
	  MsgResponse(MSG_WAIT_ACK, NAME(MSG_WAIT_ACK)),
	  error_code(0){};

	  DWORD error_code;

};


class WaiterSession
{
	public:

	ApiErrorCode WaitForSingleObject(IN HANDLE h, IN DWORD timeout, OUT DWORD &res);

};

class ProcHandleWaiter:
	public LightweightProcess
{
public:

	ProcHandleWaiter(LpHandlePair pair);

	virtual void real_run();

	virtual ~ProcHandleWaiter(void);

	BOOL ProcessIwMessage();

private:

	SemaphoreInterruptorPtr _interruptor;

	#define  IW_MAX_NUMBER_OF_SLOTS 100

	// even - event handle
	// odd - associated waitable timer
	HANDLE	    _waitHandles[IW_MAX_NUMBER_OF_SLOTS*2];

	struct WaiterSlot
	{

		WaiterSlot();

		WaiterSlot(const WaiterSlot& x);

		HANDLE handle;

		HANDLE timer_handle;	

		shared_ptr<MsgWaiterSubmitReq> orig_request;

		int quick_access_index;

		DWORD timeout;

	};

	// even and odd points to associated waiter slot
	WaiterSlot *_quickAccessArray[IW_MAX_NUMBER_OF_SLOTS*2];

	int InitHandlesArray();

	


	typedef 
	list<WaiterSlot> WaiterSlotList;

	WaiterSlotList _slots;

};

}
