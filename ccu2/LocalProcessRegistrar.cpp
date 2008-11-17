#include "StdAfx.h"
#include "LocalProcessRegistrar.h"
#include "CcuLogger.h"
#include "Ccu.h"



LocalProcessRegistrar *
LocalProcessRegistrar::_instance = NULL;

mutex 
LocalProcessRegistrar::_instanceMutex;


RegistrationGuard::RegistrationGuard(LpHandlePtr ptr)
{
	LocalProcessRegistrar::Instance().RegisterChannel(GetObjectUid(),ptr);
}

RegistrationGuard::~RegistrationGuard()
{
	LocalProcessRegistrar::Instance().UnregisterChannel(GetObjectUid());
}

LocalProcessRegistrar::LocalProcessRegistrar(void)
{
}

LocalProcessRegistrar::~LocalProcessRegistrar(void)
{
}

LocalProcessRegistrar &
LocalProcessRegistrar::Instance()
{
	mutex::scoped_lock lock(_instanceMutex);

	if (_instance == NULL)
	{
		_instance = new LocalProcessRegistrar();
	}

	return *_instance;

}

void
LocalProcessRegistrar::RegisterChannel(int procId, LpHandlePtr ptr)
{
	//
	// Scope to refrain taking nested locks
	// logs & collection
	//
	{
		mutex::scoped_lock lock(_mutex);
		if ( _locProcessesMap.find(procId) != _locProcessesMap.end())
		{
			LogCrit(L"Registered process=[" << procId << L"] >>twice<<");
			throw;
		}

		_locProcessesMap[procId] = ptr;
	}
	LogDebug(">>Registered<< channel id=[" << dec << procId << "] ->  handle=[" << ptr.get() << "]");
}

void
LocalProcessRegistrar::UnregisterChannel(int procId)
{
	//
	// Scope to refrain taking nested locks
	// logs & collection
	//
	{
		mutex::scoped_lock lock(_mutex);
		if (_locProcessesMap.find(procId) == _locProcessesMap.end())
		{
			return;
		}

		_locProcessesMap.erase(procId);


		ListenersMap::iterator iter = _listenersMap.find(procId);
		if (iter != _listenersMap.end())
		{
			HandlesList list = (*iter).second;

			for (HandlesList::iterator set_iter = list.begin(); 
				set_iter != list.end(); 
				set_iter++)
			{
				(*set_iter)->Send(new CcuMsgShutdownEvt(procId));
			}

			list.clear();

			_listenersMap.erase(iter);
			
		}


	}
	LogDebug(">>Unregistered<< channel id=[" << dec << procId << "]");
}


LpHandlePtr 
LocalProcessRegistrar::GetHandle(int procId)
{
	return GetHandle(
		procId,
		L"");

}

#pragma TODO("TODO: Add remove and see if there's no possible leak")

void
LocalProcessRegistrar::AddShutdownListener(IN int procId, IN LpHandlePtr channel)
{
	//
	// Scope to refrain taking nested locks
	// logs & collection
	//
	{
		mutex::scoped_lock lock(_mutex);

		HandlesList list;
		if (_listenersMap.find(procId) == _listenersMap.end())
		{
			_listenersMap[procId] = list;
		}

		list.push_back(channel);
	}
}

LpHandlePtr
LocalProcessRegistrar::GetHandle(int procId,const wstring &qpath)
{
	//
	// Scope to refrain taking nested locks
	// logs & collection
	//
	{
		mutex::scoped_lock lock(_mutex);

		//
		// user specified q path
		// => send it to IPC
		//
		if (!qpath.empty())
		{
			LocalProcessesMap::iterator i = 
				_locProcessesMap.find(IPC_DISPATCHER_Q);

			if (i==_locProcessesMap.end())
			{
				return CCU_NULL_LP_HANDLE;
			}

			return (*i).second;
		}

		
		//
		// user specified no destination queue 
		// but pid is well known process
		// => if the process found in registrar send 
		// it locally, otherwise send it to IPC
		//
		if (qpath.empty() && IsWellKnownPid(procId))
		{

			LocalProcessesMap::iterator i = 
				_locProcessesMap.find(procId);

			if (i != _locProcessesMap.end())
			{
				return (*i).second;
			}

			i =	_locProcessesMap.find(IPC_DISPATCHER_Q);

			if (i !=_locProcessesMap.end())
			{
				return (*i).second;
			}

			return CCU_NULL_LP_HANDLE;
		}

		//
		// user specified no destination queue 
		// and pid is not well known process
		// => if the process found in registrar send 
		// it locally
		//
		LocalProcessesMap::iterator i = 
			_locProcessesMap.find(procId);

		if (i != _locProcessesMap.end())
		{
			return (*i).second;
		}

		
		return CCU_NULL_LP_HANDLE;
	}

}
