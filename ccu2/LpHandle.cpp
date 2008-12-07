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
GetCurrThreadOwner();

int
GetCurrCcuProcId();


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

void
IocpInterruptor::SignalDataIn()
{
	
	ULONG dwNumberOfBytesTransferred = 0;
	

	BOOL res = ::PostQueuedCompletionStatus(
		_iocpHandle,		//A handle to an I/O completion port to which the I/O completion packet is to be posted.
		dwNumberOfBytesTransferred,	//The value to be returned through the lpNumberOfBytesTransferred parameter of the GetQueuedCompletionStatus function.
		_dwCompletionKey,	//The value to be returned through the lpCompletionKey parameter of the GetQueuedCompletionStatus function.
		NULL	//The value to be returned through the lpOverlapped parameter of the GetQueuedCompletionStatus function.
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



CcuHandleDirection 
LpHandle::Direction() const 
{ 
	return _direction; 
}

void 
LpHandle::Direction(CcuHandleDirection val) 
{ 
	_direction = val; 
}


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
	// someone send to owner
	if (Direction() == CCU_MSG_DIRECTION_INBOUND)
	{
		LogDebug("LOCAL SND message=[" << message->message_id_str << "] from=[" << GetCurrThreadOwner() << "] to=[" << OwnerProcName() << "].");
	} else 
	{
		// owner sends to someone
		LogDebug("LOCAL SND message=[" << message->message_id_str << "] from=[" << OwnerProcName() << "].");
	} 

	if (message->source.proc_id == CCU_UNDEFINED)
	{
		message->source.proc_id = GetCurrCcuProcId();
	}
	
	
	_channel.writer() << message;
	if (_interruptor != NULL)
	{
		_interruptor->SignalDataIn();
	}
	
	return CCU_API_SUCCESS;
}

CcuMsgPtr
LpHandle::WaitForMessages(IN  const  Time &timeout, 
						  IN const EventsSet &msg_id_map, 
						  IN CcuChannel &channel,
						  OUT CcuApiErrorCode &res)
{

	LogTrace(L"Waiting on " << this << L" " << csp::GetSeconds(timeout) << " seconds.");
	
	CcuMsgPtr resPtr;

	Time time_to_wait = timeout;
	DWORD estimated_end = ::GetTickCount() + GetMilliSeconds(timeout);

	do {
		CcuMsgPtr ptr;

		list<Guard*> list;
		list.push_back(channel.reader().inputGuard());
#pragma TODO ("Add custom memory manager for RelTimeoutGuard to improve the speed")
		list.push_back (new RelTimeoutGuard(time_to_wait));

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
			LogTrace(L">>Timeout<< waiting for messages from proc=[" << OwnerProcName() << "]");
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
			LogTrace(L"Message >>accepted<<. " << this);
			
			if (Direction() == CCU_MSG_DIRECTION_INBOUND)
			{
				LogDebug("LOCAL RCV message=[" << ptr->message_id_str << "] to=[" << OwnerProcName() <<"]" << endl << DumpAsXml(ptr));
			} else if (Direction() == CCU_MSG_DIRECTION_OUTBOUND){
				LogDebug("LOCAL RCV message=[" << ptr->message_id_str << "] from=[" << OwnerProcName() <<"] to=[" << GetCurrThreadOwner() << "]" << endl << DumpAsXml(ptr));
			} else {
				LogDebug("LOCAL RCV message=[" << ptr->message_id_str << "] UNDEFINED " << endl << DumpAsXml(ptr));
			}

			
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
LpHandle::Wait()
{
	CcuMsgPtr ptr;
	try 
	{
		_channel.reader() >> ptr;
	} catch(PoisonException p)
	{
		LogWarn("Cannot read the channel is >>poisoned<<.");
		return CCU_NULL_MSG;
	}
	

	if (_interruptor != NULL)
	{
		_interruptor->SignalDataOut();
	}

	switch (Direction())
	{
	case CCU_MSG_DIRECTION_INBOUND:
		{
			LogDebug("LOCAL RCV message=[" << ptr->message_id_str << "] to=[" << OwnerProcName() <<"]");
			break;
		}
	case CCU_MSG_DIRECTION_OUTBOUND:
		{
			LogDebug("LOCAL RCV message=[" << ptr->message_id_str << "] from=[" << OwnerProcName() <<"] to=[" << GetCurrThreadOwner() << "]");
			break;
		}
	default:
		{
			LogDebug("LOCAL RCV message=[" << ptr->message_id_str << "] UNDEFINED");
			break;
		}
	}

	return ptr;

}

CcuMsgPtr
LpHandle::Wait(IN Time timeout,
			   IN CcuApiErrorCode &res)
{
	EventsSet msg_id_map;

	return WaitForMessages(
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
	return WaitForMessages(
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

	return WaitForMessages(timeout, msg_id_map, _channel, res);
}


void
LpHandle::OwnerProc(int parentProcId, const wstring &procName)
{
	_ownerProcId = parentProcId;
	_ownerProcName = procName;
}

wstring LpHandle::OwnerProcName() const 
{ 
	return _ownerProcName; 
}

void LpHandle::OwnerProcName(const wstring &val) 
{ 
	_ownerProcName = val; 
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
	return ostream << ((lpHandlePtr == NULL) ? L"NULL" : lpHandlePtr->OwnerProcName());
	
}


CcuApiErrorCode SelectFromChannels(
	IN  HandlesList &map,
	IN  Time timeout, 
	OUT int &index, 
	OUT CcuMsgPtr &event,
	IN  bool peekOnly)
{
	if (map.size() == 0)
	{
		CCU_API_FAILURE;
	}

	CcuMsgPtr res_ptr;
	list<Guard*> list;


	// index Zero is timeout
	list.push_back (new RelTimeoutGuard(timeout));
	for (HandlesList::iterator iter = map.begin(); iter!= map.end(); iter++)
	{
		list.push_back((*iter)->_channel.reader().inputGuard());
	}


	Alternative alt(list);
	int wait_res = alt.priSelect();

	// timeout
	if (wait_res == 0)
	{
		index = -1;
		return CCU_API_TIMEOUT;
	} else
	{
		index = wait_res - 1;
		if (!peekOnly)
		{
		    HandlesList::iterator iter = map.begin();
			advance(iter, index);
			event = (*iter)->Wait();
		}
		
		return CCU_API_SUCCESS;
	}


}
