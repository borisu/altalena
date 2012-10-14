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
#include "Logger.h"
#include "ProcHandleWaiter.h"
#include "LocalProcessRegistrar.h"



namespace ivrworx
{

	ApiErrorCode
	WaiterSession::WaitForSingleObject(HANDLE h, DWORD timeout, DWORD &os_res)
	{
		FUNCTRACKER;

		// Win API uses DWORD value while cppcsp uses signed long values.
		// So we cannot pass the value of timeout simply as Milliseconds(os_res)
		// in any case we treat INIFINTE value specially
		if (timeout > LONG_MAX && timeout != INFINITE )
		{
			return API_FAILURE;
		}


		IwMessagePtr response = NULL_MSG;

		DECLARE_NAMED_HANDLE(waiter_txn);
		waiter_txn->HandleName("Wait for handle TXN"); // for logging purposes
		waiter_txn->Direction(MSG_DIRECTION_INBOUND);

		RegistrationGuard guard(waiter_txn);


		MsgWaiterSubmitReq *msg = new MsgWaiterSubmitReq();
		msg->handle_to_wait		= h;
		msg->timeout			= timeout; 
		msg->source.handle_id	= waiter_txn->GetObjectUid();

		LpHandlePtr waiter_inbound = ivrworx::GetHandle("__waiter__");

#pragma push_macro("SendMessage")
#undef SendMessage
		ApiErrorCode res = GetCurrRunningContext()->SendMessage(waiter_inbound,IwMessagePtr(msg));
#pragma pop_macro("SendMessage")

		if (IW_FAILURE(res))
		{
			os_res = WAIT_FAILED;
			return res;
		}

		Time wait_timeout = Seconds(0);
		if (timeout == INFINITE)
		{
			// set dummy timeout with large number of milliseconds 
			wait_timeout = MilliSeconds(LONG_MAX);
		}
		else
		{
			wait_timeout = MilliSeconds((timeout));
		}


		do 
		{
			res = GetCurrRunningContext()->WaitForTxnResponse(
				waiter_txn,
				response, 
				wait_timeout);

		} while (timeout == INFINITE && res == API_TIMEOUT);

		

		if (IW_FAILURE(res))
		{
			os_res = WAIT_FAILED;
			return res;
		}



		shared_ptr<MsgWaiterAck> ack = 
			dynamic_pointer_cast<MsgWaiterAck>(response);

		os_res =  ack->error_code;

		LogTrace("res = " << dec << res << " wait 0x" << hex << os_res);

		return res;

	}


	ProcHandleWaiter::WaiterSlot::WaiterSlot():
		handle(NULL),
		timer_handle(NULL),
		orig_request(shared_ptr<MsgWaiterSubmitReq>((MsgWaiterSubmitReq*)NULL)),
		quick_access_index(IW_UNDEFINED),
		timeout(0)
	{
		FUNCTRACKER;

	}

	ProcHandleWaiter::WaiterSlot::WaiterSlot(const WaiterSlot& x)
	{
		this->handle = x.handle;
		this->timer_handle = x.timer_handle;
		this->orig_request = x.orig_request;
		this->quick_access_index = x.quick_access_index;
		this->timeout = x.timeout;
	}



	ProcHandleWaiter::ProcHandleWaiter(LpHandlePair pair)
		:LightweightProcess(pair,"ProcHandleWaiter")
	{
		FUNCTRACKER;

		_interruptor = SemaphoreInterruptorPtr(new SemaphoreInterruptor());
		_inbound->HandleInterruptor(_interruptor);

		ServiceId("__waiter__");
	}


	ProcHandleWaiter::~ProcHandleWaiter(void)
	{
	}


	int
	ProcHandleWaiter::InitHandlesArray()
	{
		// prepare array to wait on
		int pair_index = 1;
		for (WaiterSlotList::iterator i = _slots.begin(); i != _slots.end(); i++)
		{

			WaiterSlot &slot = (*i);

			// obsolete slot
			if (slot.timer_handle == NULL)
			{
				i = _slots.erase(i);
				if (i == _slots.end())
				{
					break;
				}
				continue;
			}

			int event_index = 2*pair_index;
			int timer_index = event_index+1;

			_waitHandles[event_index] = slot.handle;
			_waitHandles[timer_index] = slot.timer_handle;

			_quickAccessArray[event_index] = &slot;
			_quickAccessArray[timer_index] = &slot;

			slot.quick_access_index = pair_index;

			LogTrace("Wait on pair - event handle:0x" << hex << slot.handle <<", timer handle:0x" << slot.timer_handle 
				<< ", event index:" << dec << event_index 
				<< ", timer index:" << timer_index
				<< ", timeout:" << slot.timeout);

			pair_index++;
		}

		int nCount = (pair_index)*2; 

		return nCount;

	}

	void
	ProcHandleWaiter::real_run()
	{

		I_AM_READY;

		// first two indexes reserved for iw messages
		// and its process timeout
		_waitHandles[0] = _interruptor->WinHnd();
		_waitHandles[1] = ::CreateWaitableTimer(
			NULL,    // A pointer to a SECURITY_ATTRIBUTES structure that specifies a security descriptor
			FALSE,   // If this parameter is TRUE, the timer is a manual-reset notification timer.
			NULL	 // The name of the timer object. The name is limited to MAX_PATH characters. Name comparison is case sensitive. 
			);

		// 1 min timeout
		LARGE_INTEGER liDueTime;
		liDueTime.QuadPart = (-1)*(60000*1000*10);

		BOOL os_res = ::SetWaitableTimer(
			_waitHandles[1],		 // A handle to the timer object.
			&liDueTime,		 // The time after which the state of the timer is to be set to signaled, in 100 nanosecond intervals.
			0 ,				 // The period of the timer, in milliseconds.
			NULL,			 // A pointer to an optional completion routine. 
			NULL,			 // A pointer to a structure that is passed to the completion routine.
			FALSE			 // If this parameter is TRUE, restores a system in suspended power conservation mode when the timer state is set to signaled. 
			);

		if (os_res == FALSE)
		{
			LogSysError("SetWaitableTimer");
			throw;
		}

		BOOL shutdown_flag = FALSE;
		while (shutdown_flag == FALSE)
		{
			

			int nCount = InitHandlesArray();
			DWORD wait_res =  ::WaitForMultipleObjects(
				nCount,		// The number of object handles in the array pointed to by lpHandles. The maximum number of object handles is MAXIMUM_WAIT_OBJECTS.
				_waitHandles,	// An array of object handles.
				FALSE,		// If this parameter is TRUE, the function returns when the state of all objects in the lpHandles array is signaled.
				INFINITE	// The time-out interval, in milliseconds. 
				);


			switch (wait_res)
			{
			case WAIT_TIMEOUT:
				{
					// timeout can be only by setting timer
					LogCrit("Shut'er down clancy. She's pumping mud.");
					throw;
				}
			case WAIT_FAILED:
				{
					LogSysError("WaitForMultipleObjects");
					throw;
				}
			case WAIT_OBJECT_0:
				{
					// process iw message
					shutdown_flag = ProcessIwMessage();
					continue;
				}
			case WAIT_OBJECT_0 + 1:
				{
					// process iw message
					LogInfo("ProcWaiter keep alive.");
					BOOL os_res = ::SetWaitableTimer(
						_waitHandles[1],		 // A handle to the timer object.
						&liDueTime,		 // The time after which the state of the timer is to be set to signaled, in 100 nanosecond intervals.
						0 ,				 // The period of the timer, in milliseconds.
						NULL,			 // A pointer to an optional completion routine. 
						NULL,			 // A pointer to a structure that is passed to the completion routine.
						FALSE			 // If this parameter is TRUE, restores a system in suspended power conservation mode when the timer state is set to signaled. 
						);

					if (os_res == FALSE)
					{
						LogSysError("SetWaitableTimer");
						throw;
					}
					continue;
				}
			default:
				{
					BOOL abandoned = FALSE;

					int signaled_index = IW_UNDEFINED;
					if (wait_res >= WAIT_OBJECT_0 && wait_res <= (WAIT_OBJECT_0 + nCount - 1))
					{
						signaled_index = wait_res - WAIT_OBJECT_0;
					} 
					else
					{
						signaled_index = wait_res - WAIT_ABANDONED_0;
						abandoned = TRUE;
					}

				
					WaiterSlot* ws_ptr = _quickAccessArray[signaled_index];

					LogTrace("Signalled index:" << signaled_index << ", abandoned:" << abandoned);
					

					MsgWaiterAck* ack = new MsgWaiterAck();

					_quickAccessArray[signaled_index] = NULL;
					_quickAccessArray[signaled_index+1] = NULL;

					// event ?
					if (signaled_index % 2 == 0)
					{
						ack->error_code  =  (abandoned == TRUE) ?WAIT_ABANDONED_0:WAIT_OBJECT_0;
					}
					// timeout
					else
					{
						if (abandoned == TRUE)
						{
							LogCrit("Timeout object abandoned");
							throw;
						}

						ack->error_code = WAIT_TIMEOUT;
					};

				
					SendResponse(ws_ptr->orig_request,ack);

					BOOL close_res = ::CloseHandle(ws_ptr->timer_handle);
					LogDebug("handle released:" << hex << ws_ptr->timer_handle);
					if (close_res == FALSE)
					{
						LogSysError("::CloseHandle");
						throw;
					}

					// this is how we mark obsolete slot
					// it will be removed later when iterating
					// over array of slots
					ws_ptr->timer_handle = NULL;

				} // default
			} // switch
		} // while

	}// real_run


	BOOL
	ProcHandleWaiter::ProcessIwMessage()
	{

		if ( _inbound->InboundPending() == FALSE)
		{
			LogWarn("Handle signaled without message");
			return FALSE;
		}

		ApiErrorCode err_code = API_FAILURE;
		IwMessagePtr msg = _inbound->Wait(Seconds(0), err_code);

		if (IW_FAILURE(err_code))
		{
			LogWarn("Error reading message err:" << err_code);
			return FALSE;
		}

		switch(msg->message_id)
		{
		case MSG_PROC_SHUTDOWN_EVT:
			{
				return TRUE;
			}
		case MSG_WAITER_SUBMIT:
			{
				shared_ptr<MsgWaiterSubmitReq> submit_req
					= dynamic_pointer_cast<MsgWaiterSubmitReq>(msg);

				WaiterSlot s;

				s.timeout = submit_req->timeout;
				s.handle = submit_req->handle_to_wait;
				s.orig_request = submit_req;
				s.timer_handle = ::CreateWaitableTimer(
					NULL,   // A pointer to a SECURITY_ATTRIBUTES structure that specifies a security descriptor
					TRUE,  // If this parameter is TRUE, the timer is a manual-reset notification timer.
					NULL	// The name of the timer object. The name is limited to MAX_PATH characters. Name comparison is case sensitive. 
					);

				LogDebug("handle created:" << hex << s.timer_handle);
				

				
				LARGE_INTEGER liDueTime;
				liDueTime.QuadPart = -(((__int64)submit_req->timeout)*1000*10);

				BOOL os_res = ::SetWaitableTimer(
					s.timer_handle,  // A handle to the timer object.
					&liDueTime,		 // The time after which the state of the timer is to be set to signaled, in 100 nanosecond intervals.
					0 ,				 // The period of the timer, in milliseconds.
					NULL,			 // A pointer to an optional completion routine. 
					NULL,			 // A pointer to a structure that is passed to the completion routine.
					FALSE			 // If this parameter is TRUE, restores a system in suspended power conservation mode when the timer state is set to signaled. 
					);

				if (os_res == FALSE)
				{
					LogSysError("SetWaitableTimer");
					throw;
				}

				_slots.push_back(s);

				break;
			}
		default:
			{
				BOOL oob_res = HandleOOBMessage(msg);
				if (oob_res == FALSE)
				{
					LogCrit("Unhandled OOB message msg:" << msg->message_id);
					throw;
				}
			}
		}

		return FALSE;

	}

}
