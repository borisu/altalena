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
#include "ActiveObject.h"
#include "Logger.h"

namespace ivrworx
{
	ActiveObject::ActiveObject():
	_shutdownFlag(FALSE),
	_started(false)
	{
		
	}

	ActiveObject::~ActiveObject(void)
	{
		FUNCTRACKER;

		StopActiveObjectLwProc();
		
	}

	void
	ActiveObject::StartActiveObjectLwProc(IN ScopedForking &forking, IN LpHandlePair pair, IN const string &name)
	{
		FUNCTRACKER;

		if (_started )
		{
			LogCrit("Started object twice");
			throw;
		}

		_started = true;

		_handlePair = pair;

		ProcEventListener * evt_listener = new ProcEventListener(*this, pair, name);
		_listenerBucket = evt_listener->_bucket;

		forking.forkInThisThread(evt_listener);
	}

	void
	ActiveObject::StopActiveObjectLwProc()
	{
		FUNCTRACKER;

		if (!_started)
		{
			return;
		}

		_started = false;

		_shutdownFlag = TRUE;
		_handlePair.inbound->Send(new MsgShutdownReq());

		// We must ensure that the process stops otherwise 
		// it may use deleted objects.
		if (_listenerBucket)
		{
			LightweightProcess::Join(_listenerBucket);
		}


	}

	void 
	ActiveObject::RemoveEventListener(IN LpHandlePtr listener_handle)
	{
		if (!listener_handle)
			return;

		for( EventListenersList::iterator iter = _listenersMap.begin(); iter != _listenersMap.end(); iter++)
		{
			if ((*iter)->GetObjectUid() == listener_handle->GetObjectUid())
			{
				iter = _listenersMap.erase(iter);
				if (iter == _listenersMap.end())
					break;
			}
		}
	}

	void 
	ActiveObject::AddEventListener(IN LpHandlePtr listener_handle)
	{
		// don't add the same handle twice
		RemoveEventListener(listener_handle);
		_listenersMap.push_back(listener_handle);
	}

	void
	ActiveObject::UponActiveObjectEvent(IN IwMessagePtr ptr)
	{
		for( EventListenersList::iterator iter = _listenersMap.begin(); iter != _listenersMap.end(); iter++)
		   (*iter)->Send(ptr);

	}

	ProcEventListener::ProcEventListener(
		IN ActiveObject &object,
		IN LpHandlePair pair, 
		IN const string &name):
		LightweightProcess(pair,name),
		_activeObject(object)
	{

	}

	void 
	ProcEventListener::real_run()
	{

		FUNCTRACKER;

		volatile BOOL &shutdown_flag = _activeObject._shutdownFlag;
		ApiErrorCode err_code = API_SUCCESS;

		while (shutdown_flag != TRUE )
		{
			IwMessagePtr ptr =  _inbound->Wait(Seconds(1000), err_code);
			if (shutdown_flag == TRUE)
			{
				break;
			}

			if (err_code == API_TIMEOUT)	
			{
				continue;
			}
				
			switch (ptr->message_id)
			{
			case MSG_PROC_SHUTDOWN_REQ:
				{
					shutdown_flag = TRUE;
					break;
				}
			default:
				{
					_activeObject.UponActiveObjectEvent(ptr);
				}
			}
		}

		_activeObject._started = false;
	}
}

