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
#include "Logger.h"
#include "Profiler.h"
#include "LocalProcessRegistrar.h"



#define MAX_NUM_OF_ARGUMENTS 10

using namespace boost;

namespace ivrworx 
{

	string GetCurrLpName();

	HandleId GetCurrLpId();

#pragma  region Interruptors

	SemaphoreInterruptor::SemaphoreInterruptor():
	_handle(NULL)
	{

		_handle =  ::CreateSemaphore(
			NULL,
			0,
			MAX_MESSAGES_IN_QUEUE,
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
	SemaphoreInterruptor::WinHnd()
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
	IocpInterruptor::WinHandle()
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

	HandleDirection 
	LpHandle::Direction() const 
	{ 
		return _direction; 
	}

	void 
	LpHandle::Direction(IN HandleDirection val) 
	{ 
		_direction = val; 
	}

#pragma endregion Direction


	LpHandle::LpHandle():
	_bufferFactory(MAX_MESSAGES_IN_QUEUE),
	_channel(_bufferFactory),
	_direction(MSG_DIRECTION_UNDEFINED),
	_threadId(IW_UNDEFINED),
	_fiberId(NULL),
	_size(0)
	{
		LogTrace("LpHandle(" << this << ")");
	}

	bool 
	LpHandle::InboundPending()
	{
		CheckReader();

		return _channel.reader().pending();
	}

	void 
	LpHandle::HandleInterruptor(IN InterruptorPtr interruptor)
	{
		_interruptor = interruptor;
	}

	void 
	LpHandle::Poison()
	{
		FUNCTRACKER;

		_channel.reader().poison();
		_channel.writer().poison();
	}


	ApiErrorCode 
	LpHandle::Send(IN IwMessage *message)
	{
		return Send(IwMessagePtr(message));
	}

	ApiErrorCode 
	LpHandle::Send(IN IwMessagePtr message)
	{
		FUNCTRACKER;

		::QueryPerformanceCounter(&message->enter_queue_timestamp);
		if (message->source.handle_id == IW_UNDEFINED)
		{	
			message->source.handle_id = GetCurrLpId();
		};

		try 
		{
			_channel.writer() << message;
			_size++;
			if (_interruptor != NULL)
			{
				_interruptor->SignalDataIn();
			}
		} 
		catch(PoisonException p)
		{
			LogWarn(this << " poisoned.");
			return API_FAILURE;
		}

		LpHandlePtr rsp_handle;
		rsp_handle = GetHandle(message->source.handle_id);
		LogDebug("snd " << message->message_id_str << " to (" << this << "), via (" << rsp_handle <<").");
		return API_SUCCESS;
	}

	BOOL 
	LpHandle::PoisonedForWrite()
	{
		try 
		{
			_channel.reader().checkPoison();
		} catch(exception)
		{
			return TRUE;
		}
		
		return FALSE;
	}



	void
	LpHandle::CheckReader()
	{
		// We have to ensure that only one process can read from
		// the handle. This is part of csp interface contract design.
		if (_threadId == IW_UNDEFINED)
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

	IwMessagePtr
	LpHandle::Read()
	{
		FUNCTRACKER;

		CheckReader();

		IwMessagePtr ptr;
		try 
		{
			_channel.reader() >> ptr;
			_size--;
			if (_interruptor != NULL)
			{
				_interruptor->SignalDataOut();
			}

		} 
		catch(PoisonException p)
		{
			LogWarn(this << " poisoned.");
			return NULL_MSG;
		}

		LpHandlePtr rsp_handle = GetHandle(ptr->source.handle_id);
		LogDebug("rcv " << ptr->message_id_str << " to (" << this << "), via (" << rsp_handle <<").");
		return ptr;

	}


	IwMessagePtr
	LpHandle::Wait(IN Time timeout, OUT ApiErrorCode &res)
	{

		FUNCTRACKER;

		CheckReader();

		LogTrace("Waiting on " << this << " " << csp::GetMilliSeconds(timeout) << " ms.");

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
		LogDebug("TMT (" << this << ") - timeout");
		res = API_TIMEOUT;
		return NULL_MSG;

read:
		res = API_SUCCESS;
		return this->Read();


	}

	string 
	LpHandle::HandleName() const 
	{ 
		return _name; 
	}

	void 
	LpHandle::HandleName(IN const string &val) 
	{ 
		_name = val; 
	}

	LpHandle::~LpHandle(void)
	{
		LogTrace("~LpHandle(" << this << ")");
		Poison();
	}

	LpHandlePair::LpHandlePair(const LpHandlePair &other)
	{
		this->inbound = other.inbound;
		this->outbound = other.outbound;	
	}

	ostream& 
	operator << (ostream &ostream, const LpHandle *lpHandlePtr)
	{
		if (lpHandlePtr == NULL)
		{
			return ostream << "NULL";

		};

		return ostream 
			<< lpHandlePtr->GetObjectUid()
			<< "," << (lpHandlePtr->Direction() == MSG_DIRECTION_INBOUND ? "in":"out") 
			<< "," << lpHandlePtr->HandleName() << ", size:" << lpHandlePtr->_size; 
	
	}

	int 
	LpHandle::Size()
	{
		return _size;
	}

#define MAX_NUM_OF_CHANNELS_IN_SELECT 10

	ApiErrorCode 
	SelectFromChannels(
		IN  HandlesVector &param_handles_list,
		IN  Time timeout, 
		OUT int &res_index, 
		OUT IwMessagePtr &res_event)
	{
		FUNCTRACKER;

		size_t count = param_handles_list.size();

		if (count == 0 || 
			count > MAX_NUM_OF_CHANNELS_IN_SELECT)
		{
			return API_FAILURE;
		}

		
		// +1 for timeout
		Guard* guards[MAX_NUM_OF_CHANNELS_IN_SELECT+1];
		LpHandlePtr handles[MAX_NUM_OF_CHANNELS_IN_SELECT];
		

		// First loop is to check that may be there are already 
		// messages	so we won't do heavy operations.
		//
		int index = 0;
		for (HandlesVector::iterator iter = param_handles_list.begin(); 
			iter!= param_handles_list.end(); iter++)
		{
			LpHandlePtr ptr = (*iter);
			if (ptr->InboundPending())
			{
				res_index = index;
				res_event = ptr->Read();
				return API_SUCCESS;
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
			LogTrace("Select timeout.");
			res_index = IW_UNDEFINED;
			return API_TIMEOUT;
		} 
		
		res_index = wait_res;
		res_event = handles[wait_res]->Read();
		return API_SUCCESS;
		
	}

#pragma region LpHandlePair
	LpHandlePair::LpHandlePair()
	{

	}

	LpHandlePair::LpHandlePair(IN LpHandlePtr inbound, IN LpHandlePtr outbound)
	{
		this->inbound  = inbound;
		this->outbound = outbound;
	}
#pragma endregion LpHandlePair

}
