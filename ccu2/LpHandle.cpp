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
#include "Profiler.h"



#define CCU_MAX_NUM_OF_ARGUMENTS 10

using namespace boost;

namespace ivrworx 
{

	wstring IxGetCurrLpName();

	int		IxGetCurrLpId();

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
			LogSysError("::CreateSemaphore");
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
			LogSysError("::ReleaseSemaphore");
			throw;
		}

	}


	IocpInterruptor::IocpInterruptor()
	{
		_iocpHandle = ::CreateIoCompletionPort(
			INVALID_HANDLE_VALUE,
			NULL,
			0,
			1);
		if (_iocpHandle == NULL)
		{
			LogSysError("::CreateIoCompletionPort");
			throw;
		}

	}

	IocpInterruptor::~IocpInterruptor()
	{
		::CloseHandle(_iocpHandle);
	}

	HANDLE 
	IocpInterruptor::Handle()
	{
		return _iocpHandle;
	}

	void
	IocpInterruptor::SignalDataIn()
	{

		ULONG dwNumberOfBytesTransferred = 0;


		BOOL res = ::PostQueuedCompletionStatus(
			_iocpHandle,				//A handle to an I/O completion port to which the I/O completion packet is to be posted.
			dwNumberOfBytesTransferred,	//The value to be returned through the lpNumberOfBytesTransferred parameter of the GetQueuedCompletionStatus function.
			IOCP_UNIQUE_COMPLETION_KEY,	//The value to be returned through the lpCompletionKey parameter of the GetQueuedCompletionStatus function.
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
	_direction(CCU_MSG_DIRECTION_UNDEFINED),
	_threadId(IX_UNDEFINED),
	_fiberId(NULL)
	{

	}

	bool 
	LpHandle::InboundPending()
	{
		CheckReader();

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


	IxApiErrorCode 
	LpHandle::Send(IN IxMessage *message)
	{
		return Send(IxMsgPtr(message));
	}

	IxApiErrorCode 
	LpHandle::Send(IN IxMsgPtr message)
	{
		FUNCTRACKER;

		::QueryPerformanceCounter(&message->enter_queue_timestamp);

		if (message->source.handle_id == IX_UNDEFINED)
		{
			message->source.handle_id = IxGetCurrLpId();
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

		LogDebug("SND (" << this << ") msg=[" << message->message_id_str << "] from proc=[" << IxGetCurrLpName() << "], rsp dst=[" << message->source.handle_id  << "] txn=[" << message->transaction_id << "]");
		return CCU_API_SUCCESS;
	}

	void
	LpHandle::CheckReader()
	{
		// We have to ensure that only one process can read from
		// the handle. This is part of csp interface contract design.
		if (_threadId == IX_UNDEFINED)
		{
			_threadId = ::GetCurrentThreadId();
			_fiberId  = ::GetCurrentFiber();
		}

		if (_threadId != ::GetCurrentThreadId() || _fiberId != ::GetCurrentFiber())
		{
			LogCrit("Detected reading handle from different lightweight processes.");
			throw;
		}

	}

	IxMsgPtr
	LpHandle::Read()
	{
		FUNCTRACKER;

		CheckReader();

		IxMsgPtr ptr;
		try 
		{
			_channel.reader() >> ptr;
			if (_interruptor != NULL)
			{
				_interruptor->SignalDataOut();
			}

			IX_PROFILE_ADD_DATA(WStringToString(ptr->message_id_str), ptr->enter_queue_timestamp);
			IX_PROFILE_ADD_DATA(L"MSG PUMP AVG", ptr->enter_queue_timestamp);
		} 
		catch(PoisonException p)
		{
			LogWarn(this << " poisoned.");
			return CCU_NULL_MSG;
		}

		LogDebug("RCV (" << this << ") msg=[" << ptr->message_id_str << "] to=[" << IxGetCurrLpName() << "], rsp dst=[" << ptr->source.handle_id  << "] txn=[" << ptr->transaction_id << "]");
		return ptr;

	}


	IxMsgPtr
	LpHandle::Wait(IN Time timeout, IN IxApiErrorCode &res)
	{

		FUNCTRACKER;

		CheckReader();

		LogTrace(L"Waiting on " << this << L" " << csp::GetMilliSeconds(timeout) << " ms.");

		// if user passed 0 as timeout and there are messages 
		// pending csp will prefer to return TIMEOUT code.
		if (GetMilliSeconds(timeout) == 0)
		{
			switch (_channel.reader().pending())
			{
				case true:  goto read;
				default:	goto timeout;
			} 
		} 
		else 
		{
			
			Guard * guards[2];
			guards[0] = _channel.reader().inputGuard();
			guards[1] = new RelTimeoutGuard(timeout);
			Alternative alt(guards,2);
			
			switch (alt.priSelect())
			{
				case 0:  goto read;
				default: goto timeout;
			} 

		}

		LogCrit("Shut 'er down Clancy, she's pumping mud!");
		throw;

timeout:
		LogDebug(L"TMT (" << this << ") - timeout");
		res = CCU_API_TIMEOUT;
		return CCU_NULL_MSG;

read:
		res = CCU_API_SUCCESS;
		return this->Read();


	}


	wstring 
	LpHandle::HandleName() const 
	{ 
		return _name; 
	}

	void 
	LpHandle::HandleName(const wstring &val) 
	{ 
		_name = val; 
	}

	LpHandle::~LpHandle(void)
	{

	}

	LpHandlePair::LpHandlePair(const LpHandlePair &other)
	{
		this->inbound = other.inbound;
		this->outbound = other.outbound;	
	}

	wostream& 
	operator << (wostream &ostream, const LpHandle *lpHandlePtr)
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

#define MAX_NUM_OF_CHANNELS_IN_SELECT 10

	IxApiErrorCode 
		SelectFromChannels(
		IN  HandlesList &param_handles_list,
		IN  Time timeout, 
		OUT int &res_index, 
		OUT IxMsgPtr &res_event)
	{
		FUNCTRACKER;

		size_t count = param_handles_list.size();

		if (count == 0 || 
			count > MAX_NUM_OF_CHANNELS_IN_SELECT)
		{
			return CCU_API_FAILURE;
		}

		
		// +1 for timeout
		Guard* guards[MAX_NUM_OF_CHANNELS_IN_SELECT+1];
		LpHandlePtr handles[MAX_NUM_OF_CHANNELS_IN_SELECT];
		

		// First loop is to check that may be there are already 
		// messages	so we won't do heavy operations.
		//
		int index = 0;
		for (HandlesList::iterator iter = param_handles_list.begin(); 
			iter!= param_handles_list.end(); iter++)
		{
			LpHandlePtr ptr = (*iter);
			if (ptr->InboundPending())
			{
				res_index = index;
				res_event = ptr->Read();
				return CCU_API_SUCCESS;
			}

			handles[index] = ptr;

			index++;
		}

		for (size_t i = 0; i < count; i++)
		{
			guards[i] = handles[i]->_channel.reader().inputGuard();
		}
		guards[count] = new RelTimeoutGuard(timeout);

		Alternative alt(guards,count+1);
		int wait_res = alt.priSelect();
		

		// timeout
		if (wait_res == count)
		{
			LogDebug("Select timeout.");
			res_index = IX_UNDEFINED;
			return CCU_API_TIMEOUT;
		} 
		
		res_index = wait_res;
		res_event = handles[wait_res]->Read();
		return CCU_API_SUCCESS;
		
	}

#pragma region LpHandlePair
	LpHandlePair::LpHandlePair()
	{

	}

	LpHandlePair::LpHandlePair(IN LpHandlePtr inbound, IN LpHandlePtr outbound)
	{
		this->inbound = inbound;
		this->outbound = outbound;
	}
#pragma endregion LpHandlePair

}
