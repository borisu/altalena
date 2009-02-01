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
#include "LpHandle.h"
#include "CcuLogger.h"



#define CCU_MAX_NUM_OF_ARGUMENTS 10

using namespace boost;

wstring 
IxGetCurrLpName();

int
IxGetCurLpId();

#pragma  region Interruptors
SemaphoreInterruptor::SemaphoreInterruptor():
_handle(NULL)
{

	_handle =  ::CreateSemaphore(
		NULL,
		0,
		CCU_MAX_MESSAGES_IN_QUEUE,
		NULL
		);

	if (_handle == NULL)
	{
		LogSysError("Cannot ::CreateSemaphore");
		throw;
	}
	
	
}

SemaphoreInterruptor::~SemaphoreInterruptor()
{
	BOOL res=::CloseHandle(_handle);
	_handle = NULL;

}

void
SemaphoreInterruptor::SignalDataOut()
{
	
	
}

HANDLE
SemaphoreInterruptor::Handle()
{
	return _handle;
}


void
SemaphoreInterruptor::SignalDataIn()
{
	BOOL res = ::ReleaseSemaphore(_handle, 1, NULL);
	if (res == FALSE)
	{
		LogSysError("Cannot ::ReleaseSemaphore");
		throw;
	}

}


IocpInterruptor::IocpInterruptor(IN HANDLE iocpHandle, IN DWORD dwCompletionKey)
:_iocpHandle(iocpHandle),
_dwCompletionKey(dwCompletionKey)
{
	

}

IocpInterruptor::~IocpInterruptor()
{

}

void
IocpInterruptor::SignalDataIn()
{
	
	ULONG dwNumberOfBytesTransferred = 0;
	

	BOOL res = ::PostQueuedCompletionStatus(
		_iocpHandle,				//A handle to an I/O completion port to which the I/O completion packet is to be posted.
		dwNumberOfBytesTransferred,	//The value to be returned through the lpNumberOfBytesTransferred parameter of the GetQueuedCompletionStatus function.
		_dwCompletionKey,			//The value to be returned through the lpCompletionKey parameter of the GetQueuedCompletionStatus function.
		NULL						//The value to be returned through the lpOverlapped parameter of the GetQueuedCompletionStatus function.
		);


	if (res == FALSE)
	{
		LogSysError("::PostQueuedCompletionStatus");
		throw;
	}
}

void
IocpInterruptor::SignalDataOut()
{

}

#pragma endregion Interruptors


#pragma region Direction

IxHandleDirection 
LpHandle::Direction() const 
{ 
	return _direction; 
}

void 
LpHandle::Direction(IxHandleDirection val) 
{ 
	_direction = val; 
}

#pragma endregion Direction


LpHandle::LpHandle():
_bufferFactory(CCU_MAX_MESSAGES_IN_QUEUE),
_channel(_bufferFactory),
_firstChanceHandler(NULL),
_direction(CCU_MSG_DIRECTION_UNDEFINED)
{

}

bool 
LpHandle::InboundPending()
{
	return _channel.reader().pending();
}

void 
LpHandle::HandleInterruptor(InterruptorPtr interruptor)
{
	_interruptor = interruptor;
}

void 
LpHandle::Poison()
{
	_channel.reader().poison();
	_channel.writer().poison();
}


CcuApiErrorCode 
LpHandle::Send(IN CcuMessage *message)
{
	return Send(CcuMsgPtr(message));
}

CcuApiErrorCode 
LpHandle::Send(IN CcuMsgPtr message)
{
	FUNCTRACKER;
	
	if (message->source.proc_id == CCU_UNDEFINED)
	{
		message->source.proc_id = IxGetCurLpId();
	}
	
	try 
	{
		_channel.writer() << message;
		if (_interruptor != NULL)
		{
			_interruptor->SignalDataIn();
		}
	} 
	catch(PoisonException p)
	{
		LogWarn(this << " poisoned.");
		return CCU_API_FAILURE;
	}

	LogDebug("SND (" << this << ") msg=[" << message->message_id_str << "] from proc=[" << IxGetCurrLpName() << "], rsp dst=[" << message->source.proc_id  << "] txn=[" << message->transaction_id << "]");
	return CCU_API_SUCCESS;
}

CcuMsgPtr
LpHandle::Read()
{
	FUNCTRACKER;

	CcuMsgPtr ptr;
	try 
	{
		_channel.reader() >> ptr;
		if (_interruptor != NULL)
		{
			_interruptor->SignalDataOut();
		}
	} 
	catch(PoisonException p)
	{
		LogWarn(this << " poisoned.");
		return CCU_NULL_MSG;
	}

	LogDebug("RCV (" << this << ") msg=[" << ptr->message_id_str << "] to=[" << IxGetCurrLpName() << "], rsp dst=[" << ptr->source.proc_id  << "] txn=[" << ptr->transaction_id << "]");
	return ptr;

}

CcuMsgPtr
LpHandle::WaitForMessagesOnChannel(
						  IN  const  Time &timeout, 
						  IN  const EventsSet &msg_id_map, 
						  IN  CcuChannel &channel,
						  OUT CcuApiErrorCode &res)
{

	FUNCTRACKER;

	LogTrace(L"Waiting on " << this << L" " << csp::GetMilliSeconds(timeout) << " ms.");
	
	CcuMsgPtr resPtr;

	Time time_to_wait = timeout;
	DWORD estimated_end = ::GetTickCount() + GetMilliSeconds(timeout);

	do {
		CcuMsgPtr ptr;

		list<Guard*> list;
		list.push_back(channel.reader().inputGuard());

		// if user passed 0 as timeout and there are messages 
		// pending csp will prefer to return TIMEOUT code.
		// We override this behavior by trying to return
		// pending messages in queue in this case. We're doing
		// it by disabling the RelTimeoutGuard
		if (!(GetMilliSeconds(time_to_wait) == 0 && 
			channel.reader().pending()))
		{
			list.push_back (new RelTimeoutGuard(time_to_wait));
		}

		Alternative alt(list);

		
		int wait_res = alt.priSelect();
		DWORD wait_end = ::GetTickCount();

		if (wait_res == 0)
		{
			channel.reader() >> ptr;
		} else
		{
			//
			//  Timeout
			//
			LogTrace(L"TIMEOUT " << this );
			resPtr = ptr;
			res = CCU_API_TIMEOUT;
			break;
		}

		//
		// Reduce the number of waiting 
		// messages in interruptor.
		//
		if (_interruptor != NULL)
		{
			_interruptor->SignalDataOut();
		}

		LogTrace(L">>Received<< message msg=[" << ptr->message_id_str << "] " << this);

		
		//
		// Check that message is one that 
		// we are waiting for. 0 map indicates we accept anything
		//
		if (msg_id_map.size() == 0 ||
			msg_id_map.find(ptr->message_id) != msg_id_map.end())
		{
						
			LogDebug("RCV (" << this << ") msg=[" << ptr->message_id_str << "] to=[" << IxGetCurrLpName() << "], rsp dst=[" << ptr->source.proc_id  << "] txn=[" << ptr->transaction_id << "]");
			
			resPtr = ptr;
			res = CCU_API_SUCCESS;
			break;
		}

		//
		// OOB message
		//
		LogTrace(L"Message is >>First Chance OOB<< . " << this);
		BOOL oob_res = FALSE;
		if (_firstChanceHandler)
		{
			oob_res = _firstChanceHandler->HandleOOBMessage(ptr);
		}

		if (oob_res == FALSE)
		{
			resPtr = ptr;
			res = CCU_API_OUT_OF_BAND;
			LogTrace(L"First Chance OOB Message >>rejected<<." << this);
			break;
		}

		LogTrace(L"First Chance OOB Message >>handled<<." << this);

		//
		// Even if OOB handler took too much time
		// and timeout already elapsed we have 
		// to give it one more chance
		//
		if (wait_end >= estimated_end)
		{
			time_to_wait = MilliSeconds(0);
		} else 
		{
			time_to_wait = MilliSeconds(estimated_end - wait_end);
		}


	} while (true);

	return resPtr;

}





CcuMsgPtr
LpHandle::Wait(IN Time timeout,
			   IN CcuApiErrorCode &res)
{
	EventsSet msg_id_map;

	return WaitForMessagesOnChannel(
		timeout,
		msg_id_map,
		_channel, 
		res);

}


CcuMsgPtr  
LpHandle::WaitForMessages(IN const  Time timeout, 
						  IN const EventsSet &msg_id_map, 
						  OUT CcuApiErrorCode &res)
{
	return WaitForMessagesOnChannel(
		timeout,
		msg_id_map,
		_channel, 
		res);

}

CcuMsgPtr
LpHandle::WaitForMessages(Time timeout, 
						  CcuApiErrorCode &res,
						  CcuMessageId marker,
						  ...)
{
	va_list param_list;
	va_start(param_list,marker);

	EventsSet msg_id_map;
	if (marker != CCU_MSG_MARKER)
	{
		LogCrit (L"Dear Developer! Have you used the safe list macro? [no marker at begin]");
		throw;
	}
	
	CcuMessageId param_msg_id = 
		va_arg(param_list,CcuMessageId);

	int counter = 0;
	while (param_msg_id != CCU_MSG_MARKER ) 
	{
		msg_id_map.insert(param_msg_id);
		param_msg_id = 
			va_arg(param_list,CcuMessageId);

		if (counter++ == CCU_MAX_NUM_OF_ARGUMENTS)
		{
			LogCrit(L"Dear Developer! Have you used the safe list macro? [too much args]");
			throw;
		}
	} 
		
	va_end(param_list);

	return WaitForMessagesOnChannel(timeout, msg_id_map, _channel, res);
}


wstring 
LpHandle::HandleName() const 
{ 
	return _name; 
}


void LpHandle::HandleName(const wstring &val) 
{ 
	_name = val; 
}

void
LpHandle::FirstChanceOOBMsgHandler(IFirstChanceOOBMsgHandler *handler)
{
	_firstChanceHandler = handler;
}

#pragma TODO("Handle poisoning in LightweightProcess")
LpHandle::~LpHandle(void)
{
}

LpHandlePair::LpHandlePair()
{

}

LpHandlePair::LpHandlePair(IN LpHandlePtr inbound, IN LpHandlePtr outbound)
{
	this->inbound = inbound;
	this->outbound = outbound;
}

LpHandlePair::LpHandlePair(const LpHandlePair &other)
{
	this->inbound = other.inbound;
	this->outbound = other.outbound;	
}

wostream& operator << (wostream &ostream, const LpHandle *lpHandlePtr)
{
	if (lpHandlePtr == NULL)
	{
		return ostream << L"NULL";

	};

	return ostream 
		<< (lpHandlePtr->Direction() == CCU_MSG_DIRECTION_INBOUND ? L"IN ":L"OUT ") 
		<<  lpHandlePtr->HandleName() 
		<< L"," 
		<< lpHandlePtr->GetObjectUid();
	
}


CcuApiErrorCode 
SelectFromChannels(
	IN  HandlesList &param_handles_list,
	IN  Time timeout, 
	OUT int &res_index, 
	OUT CcuMsgPtr &res_event)
{
	FUNCTRACKER;

	if (param_handles_list.size() == 0)
	{
		return CCU_API_FAILURE;
	}

	
	// First loop is to check that may be there are already 
	// messages	so we won't do heavy operations.
	//
	int index = 0;
	for (HandlesList::iterator iter = param_handles_list.begin(); iter!= param_handles_list.end(); iter++)
	{
		LpHandlePtr ptr = (*iter);
		bool pending	= ptr->InboundPending();

		LogTrace(L"(" << (*iter).get() << L"), pending:" << pending);

		if (pending)
		{
			res_index = index;
			res_event = ptr->Read();
			return CCU_API_SUCCESS;
		}

		index++;
	}

	// ok let's do it hard way
	list<Guard*> list;

	// index Zero is timeout
	index = 0;
	list.push_back (new RelTimeoutGuard(timeout));
	LogTrace("(RelTimeoutGuard) added at index=[" << index <<"]");
	for (HandlesList::iterator iter = param_handles_list.begin(); iter!= param_handles_list.end(); iter++)
	{
		index++;

		LpHandlePtr curr_handle = (*iter);
		list.push_back(curr_handle->_channel.reader().inputGuard());
		LogTrace("(" << curr_handle.get() << ") added at index=[" << index <<"]");
	}

	Alternative alt(list);
	int wait_res = alt.priSelect();
	LogTrace("handle index=[" << wait_res << "] selected.");

	// timeout
	if (wait_res == 0)
	{
		res_index = CCU_UNDEFINED;
		return CCU_API_TIMEOUT;
	} else
	{
		res_index = wait_res - 1;

		HandlesList::iterator iter = param_handles_list.begin();
		std::advance(iter, res_index);
		res_event = (*iter)->Read();

		
		return CCU_API_SUCCESS;
	}


}
