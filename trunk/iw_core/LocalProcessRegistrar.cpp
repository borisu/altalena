#include "StdAfx.h"
#include "LocalProcessRegistrar.h"
#include "Logger.h"
#include "IwBase.h"

using namespace boost;

namespace ivrworx
{



LocalProcessRegistrar *
LocalProcessRegistrar::_instance = NULL;

mutex 
LocalProcessRegistrar::_instanceMutex;


RegistrationGuard::RegistrationGuard(IN LpHandlePtr ptr, 
									 IN const string& service_name):
_handleUid(ptr->GetObjectUid())
{
	LocalProcessRegistrar::Instance().RegisterChannel(_handleUid,ptr,service_name);
}

void RegistrationGuard::dismiss()
{
	_handleUid = IW_UNDEFINED;
}



RegistrationGuard::~RegistrationGuard()
{
	if (_handleUid!=IW_UNDEFINED) 
		LocalProcessRegistrar::Instance().UnregisterChannel(_handleUid);
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
LocalProcessRegistrar::RegisterChannel(IN int handle_id, IN LpHandlePtr ptr, IN const string& service_id)
{
	
	if (handle_id == IW_UNDEFINED)
	{
		return;
	}
	
	mutex::scoped_lock lock(_mutex);
	if ( _locProcessesMap.find(handle_id) != _locProcessesMap.end())
	{
		throw critical_exception("trying to register the same process twice");
	}

	_locProcessesMap[handle_id] = ptr;
	_servicesMap[handle_id] = service_id;
	
	
	
	LogTrace("Mapped " << handle_id << " to (" << ptr.get() << ")");
}

void
LocalProcessRegistrar::UnregisterChannel(IN int handle_id)
{
	if (handle_id == IW_UNDEFINED)
	{
		return;
	}
	
	mutex::scoped_lock lock(_mutex);
	if (_locProcessesMap.find(handle_id) == _locProcessesMap.end())
	{
		return;
	}

	_locProcessesMap.erase(handle_id);
	_servicesMap.erase(handle_id);
	


	ListenersMap::iterator iter = _listenersMap.find(handle_id);
	if (iter != _listenersMap.end())
	{
		HandlesVector &list = (*iter).second;
		for (HandlesVector::iterator set_iter = list.begin(); 
			set_iter != list.end(); 
			set_iter++)
		{
			(*set_iter)->Send(new MsgShutdownEvt(handle_id));
		}

		list.clear();

		_listenersMap.erase(iter);
			
	}

	LogTrace("Unregistered handle " << handle_id);
}


LpHandlePtr 
LocalProcessRegistrar::GetHandle(IN int procId)
{
	return GetHandle(procId,"");

}

void
LocalProcessRegistrar::AddShutdownListener(IN int procId, IN LpHandlePtr channel)
{
	
	mutex::scoped_lock lock(_mutex);

	//
	// Create new list if needed.
	//
	if (_listenersMap.find(procId) == _listenersMap.end())
	{
		HandlesVector list; 
		_listenersMap[procId] = list;
	}

	_listenersMap[procId].push_back(channel);

}

LpHandlePtr
LocalProcessRegistrar::GetHandle(IN int procId, IN const string &qpath)
{
	mutex::scoped_lock lock(_mutex);

	LocalProcessesMap::iterator i = 
		_locProcessesMap.find(procId);

	if (i != _locProcessesMap.end())
	{
		return (*i).second;
	}

	
	return IW_NULL_HANDLE;
	

}

LpHandlePtr
LocalProcessRegistrar::GetHandle( IN const string &regex)
{
	mutex::scoped_lock lock(_mutex);

	boost::regex e(regex);

	for (LocalProcessesMap::iterator i = _locProcessesMap.begin();
		i != _locProcessesMap.end();
		++i)
	{
		
		ProcId proc_id = (*i).first;
		string service_name = _servicesMap[proc_id];

		boost::smatch what;
		if (true == boost::regex_match(service_name, what, e, boost::match_extra))
		{
			return (*i).second;
		}

	}

	return IW_NULL_HANDLE;

}

IW_CORE_API void 
AddShutdownListener( IN LpHandlePair observable_pair, 
					 IN LpHandlePtr listener_handle)
{
	LocalProcessRegistrar::Instance().AddShutdownListener(
		observable_pair.inbound->GetObjectUid(), 
		listener_handle);

}

IW_CORE_API LpHandlePtr 
GetHandle(IN int handle_id)
{
	return LocalProcessRegistrar::Instance().GetHandle(handle_id);
}

IW_CORE_API LpHandlePtr 
GetHandle(IN const string &service_regex)
{
	return LocalProcessRegistrar::Instance().GetHandle(service_regex);
}


}