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

	IxMsgPtr
	LpHandle::Read()
	{
		FUNCTRACKER;

		IxMsgPtr ptr;
		try 
		{
			_channel.reader() >> ptr;
			if (_interruptor != NULL)
			{
				_interruptor->SignalDataOut();
			}

			IX_PROFILE_ADD_DATA(WStringToString(ptr->message_id_str), ptr->enter_queue_timestamp);
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
			list<Guard*> list;
			list.push_back(_channel.reader().inputGuard());
			list.push_back (new RelTimeoutGuard(timeout));
			Alternative alt(list);
			
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


	IxApiErrorCode 
		SelectFromChannels(
		IN  HandlesList &param_handles_list,
		IN  Time timeout, 
		OUT int &res_index, 
		OUT IxMsgPtr &res_event)
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
		for (HandlesList::iterator iter = param_handles_list.begin(); 
			iter!= param_handles_list.end(); 
			iter++)
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
			LogTrace("select timeout.");
			res_index = IX_UNDEFINED;
			return CCU_API_TIMEOUT;
		} 
		else
		{
			res_index = wait_res - 1;

			HandlesList::iterator iter = param_handles_list.begin();
			std::advance(iter, res_index);
			res_event = (*iter)->Read();

			return CCU_API_SUCCESS;
		}
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
