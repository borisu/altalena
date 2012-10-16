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

#include "stdafx.h"
#include "LightweightProcess.h"
#include "LocalProcessRegistrar.h"
#include "Logger.h"
#include "Profiler.h"
#include "DllHelpers.h"

#pragma push_macro("SendMessage")
#undef SendMessage

namespace ivrworx
{

	typedef 
	map<PVOID, RunningContext*> ProcMap;

	int AppData::getType()
	{
		return IW_UNDEFINED;
	}

#define TLS_PROC_MAP 15
	ProcMap *GetTlsProcMap()
	{
		LPVOID res = NULL;
		if (GetCoreData(TLS_PROC_MAP,&res) && res != NULL)
		{
			return (ProcMap *)res;
		}
		else
		{
			StoreCoreData(TLS_PROC_MAP,new ProcMap());
			return  GetTlsProcMap();
		}

	}

	RunningContext::RunningContext(
		IN LpHandlePair pair,
		IN const string &owner_name,
		IN BOOL start_suspended):
	_pair(pair),
	_inbound(pair.inbound),
	_outbound(pair.outbound),
	_bucket(new Bucket()),
	_transactionTimeout(5000),
	_startSuspended(start_suspended),
	_appData(new AppData())
	{
		FUNCTRACKER;

		// process id is as its inbound handle id
		Init(_inbound->GetObjectUid(), owner_name);
	}

	void
	RunningContext::Init(int process_id, const string &owner_name)
	{

		FUNCTRACKER;

		if (_inbound == _outbound)
		{
			LogCrit("Cannot use the same handle for outbound and inbound channel.");
			throw;
		}

		if (_inbound != IW_NULL_HANDLE)
		{
			_inbound->HandleName(owner_name);
		}

		if (_outbound != IW_NULL_HANDLE)
		{
			_outbound->HandleName(owner_name);
		}

		_processId = process_id;

		_inbound->Direction(MSG_DIRECTION_INBOUND);
		_outbound->Direction(MSG_DIRECTION_OUTBOUND);

		_name = owner_name;

	}




	BOOL 
	RunningContext::HandleOOBMessage(IN IwMessagePtr msg)
	{
		FUNCTRACKER;

		LogDebug("First Chance OOB msg:"<< msg->message_id_str);

		switch (msg->message_id)
		{
		case MSG_PING:
			{
				SendResponse(msg,new MsgPong());
				return  TRUE;
			}
		default:
			{
				return FALSE;
			}
		}

	}

	AppData*
	RunningContext::GetAppData() 
	{ 
		return _appData;
	};

	void
	RunningContext::SetAppData(AppData* data) 
	{ 
		_appData = data;
	};

	string 
	RunningContext::Name()
	{ 
		return _name; 
	}

	void 
	RunningContext::Name(IN const string &val) 
	{
		_name = val; 
	}

	string 
	RunningContext::ServiceId()
	{ 
		return _name; 
	}

	void 
	RunningContext::ServiceId(IN const string &serviceId) 
	{
		_serviceId = serviceId; 
	}




	ApiErrorCode
	RunningContext::SendMessage(
		IN ProcId qid, 
		IN IwMessage *message)
	{
		return SendMessage(qid, IwMessagePtr(message));
	}


	ApiErrorCode
	RunningContext::SendResponse(
		IN IwMessagePtr request, 
		IN IwMessage* response)
	{
		response->copy_data_on_response(request.get());
		return SendMessage(IwMessagePtr(response));
	}

	ApiErrorCode
	RunningContext::SendMessage(
		IN ProcId qid, 
		IN IwMessagePtr message)
	{

		message->dest.handle_id = qid;
		return SendMessage(message);
	}

	ApiErrorCode
	RunningContext::SendMessage(
		IN LpHandlePtr handle, 
		IN IwMessagePtr message)
	{
		return SendMessage(handle->GetObjectUid(), message);
	}


	ApiErrorCode
	RunningContext::SendMessage(IN IwMessagePtr message)
	{

		FUNCTRACKER;

		int handle_id = message->dest.handle_id;
		if (message->source.handle_id == IW_UNDEFINED)
		{
			message->source.handle_id = _processId;
		}

		if (message->transaction_id == IW_UNDEFINED)
		{
			message->transaction_id = GenerateNewTxnId();
		}


		LpHandlePtr localHandlePtr;

		// send message via the same IPC interface they arrived from
		if (message->preferrable_ipc_interface != IW_UNDEFINED)
		{
			localHandlePtr = LocalProcessRegistrar::Instance().GetHandle(message->preferrable_ipc_interface);
		}


		if (localHandlePtr == IW_NULL_HANDLE)
		{
			localHandlePtr = 
				LocalProcessRegistrar::Instance().GetHandle(handle_id,message->dest.queue_path);
		}


		if (localHandlePtr != IW_NULL_HANDLE)
		{
			localHandlePtr->Send(message);
			return API_SUCCESS;
		}

		LogWarn("Unknown destination for msg:" << message->message_id_str << ", dst:"<< handle_id );
		return API_UNKNOWN_DESTINATION;

	}


	int
	RunningContext::ProcessId()
	{
		return _processId;
	}


	void
	RunningContext::Join(BucketPtr bucket)
	{
		bucket->fallInto();
	}


	ApiErrorCode
	RunningContext::DoRequestResponseTransaction(
		IN LpHandlePtr dest_handle, 
		IN IwMessagePtr request, 
		OUT IwMessagePtr &response,
		IN Time timeout,
		IN string transaction_name)
	{

		FUNCTRACKER;

		DECLARE_NAMED_HANDLE(txn_handle);
		txn_handle->HandleName(transaction_name); // for logging purposes
		txn_handle->Direction(MSG_DIRECTION_INBOUND);

		RegistrationGuard guard(txn_handle);

		request->source.handle_id = txn_handle->GetObjectUid();
		request->transaction_id = GenerateNewTxnId();

		if (IW_FAILURE(dest_handle->Send(request)))
		{
			return API_UNKNOWN_DESTINATION;
		}

		ApiErrorCode res = WaitForTxnResponse(
			txn_handle,
			response,
			timeout);

		LogTrace("RunningContext::DoRequestResponseTransaction - res:" << res);
		return res;

	}

	ApiErrorCode	
	RunningContext::DoRequestResponseTransaction(
		IN ProcId dest_proc_id, 
		IN IwMessagePtr request, 
		OUT IwMessagePtr &response, 
		IN Time timout, 
		IN string transaction_name)
	{

		FUNCTRACKER;

		DECLARE_NAMED_HANDLE(txn_handle);
		txn_handle->HandleName(transaction_name);
		txn_handle->Direction(MSG_DIRECTION_INBOUND);

		RegistrationGuard guard(txn_handle);


		request->source.handle_id = txn_handle->GetObjectUid();
		request->dest.handle_id = dest_proc_id;
		request->transaction_id = GenerateNewTxnId();

		ApiErrorCode res = SendMessage(request);
		if (IW_FAILURE(res))
		{
			goto end;
		}

		res = WaitForTxnResponse(
			txn_handle,
			response,
			timout);
end:

		LogTrace("RunningContext::DoRequestResponseTransaction - res:" << res);
		return res;

	}

	ApiErrorCode 
	RunningContext::WaitForTxnResponse(
		IN LpHandlePtr txn_handle,
		OUT IwMessagePtr &response,
		IN Time timout)
	{

		FUNCTRACKER;

		HandlesVector temp_list;
		temp_list.push_back(txn_handle);

		int temp_index = IW_UNDEFINED;

		ApiErrorCode res = 
			WaitForTxnResponse(temp_list,temp_index,response,timout);

		return res;

	}

	ApiErrorCode 
	RunningContext::WaitForTxnResponse(
		IN  const HandlesVector &handles_list,
		OUT int &index,
		OUT IwMessagePtr &response,
		IN  Time timeout)
	{

		FUNCTRACKER;

		//
		// Create lists of handles that 
		// we are waiting messages from.
		//
		HandlesVector list;
		list.push_back(_inbound);
		list.insert(list.end(),handles_list.begin(),handles_list.end());


		int interrupted_handle_index = IW_UNDEFINED;
		ApiErrorCode res= API_SUCCESS;


		//
		// Message waiting loop.
		//
		sign32 timeLeftToWaitMs = GetMilliSeconds(timeout);
		if (timeLeftToWaitMs < 0)
		{
			LogWarn("Illegal value for timeout " << timeLeftToWaitMs);
			return API_WRONG_PARAMETER;
		}
		while (timeLeftToWaitMs >= 0)
		{
			int start = ::GetTickCount();

			ApiErrorCode err_code = 
				SelectFromChannels(
				list,
				MilliSeconds(timeLeftToWaitMs), 
				interrupted_handle_index, 
				response);

			if (IW_FAILURE(err_code))
			{
				return err_code;
			}


			switch (interrupted_handle_index )
			{
			case 0:
				{
					BOOL res = this->HandleOOBMessage(response);
					if (res == FALSE)
					{
						LogWarn("Unhandled OOB msg:" << response->message_id_str);
						return API_UNKNOWN_RESPONSE;
					} 
					else 
					{
						// it was terminator who stopped the transaction, so we have
						// to give transaction unit one more chance
						int delta = ::GetTickCount() - start;
						timeLeftToWaitMs -= delta;

						if (timeLeftToWaitMs < 0)
						{
							timeLeftToWaitMs = 0;
						}

						continue;
					}

				}
			default:
				{
					index = interrupted_handle_index - 1;
					goto select_end;
				}
			}// switch
		} // while

select_end:		

		return res;

	}


	ApiErrorCode
	RunningContext::SendReadyMessage()
	{
		MsgProcReady* ready = new MsgProcReady();
		ready->thread_id = ::GetCurrentThread();

		_outbound->Send(ready);

		return API_SUCCESS;
	}

	ApiErrorCode
	RunningContext::Ping(IN ProcId qid)
	{
		FUNCTRACKER;

		IwMessagePtr dummy_response = NULL_MSG;

		ApiErrorCode res = this->DoRequestResponseTransaction(
			qid,
			IwMessagePtr(new MsgPing()),
			dummy_response,
			MilliSeconds(_transactionTimeout),
			"Ping-Pong TXN");

		return res;

	}

	ApiErrorCode
	RunningContext::Ping(IN LpHandlePair pair)
	{
		FUNCTRACKER;

		IwMessagePtr dummy_response = NULL_MSG;

		ApiErrorCode res = this->DoRequestResponseTransaction(
			pair.inbound,
			IwMessagePtr(new MsgPing()),
			dummy_response,
			MilliSeconds(_transactionTimeout),
			"Ping-Pong TXN");

		return res;

	}


	ApiErrorCode
	RunningContext::WaitTillReady(IN Time time, IN LpHandlePair pair)
	{


		// MSG_PROC_READY must be the first message sent
		// so we do not create special process for running 
		// the transaction
		ApiErrorCode res = API_FAILURE;
		IwMessagePtr response = pair.outbound->Wait(
			time,
			res);

		if (res == API_TIMEOUT)
		{
			Shutdown(Seconds(0),pair);
			return API_FAILURE;
		}

		switch (response->message_id)
		{
		case MSG_PROC_READY:
			{
				return API_SUCCESS;
			}
		default:
			{
				return API_FAILURE;
			}
		}
	}

	ApiErrorCode
	RunningContext::Shutdown(IN Time time, IN LpHandlePair pair)
	{

		FUNCTRACKER;
		IwMessagePtr response = NULL_MSG;

		ApiErrorCode res = this->DoRequestResponseTransaction(
			pair.inbound,
			IwMessagePtr(new MsgShutdownReq()),
			response,
			time,
			"Shutdown TXN");

		if (IW_FAILURE(res))
		{
			return res;
		}

		switch (response->message_id)
		{
		case MSG_PROC_SHUTDOWN_ACK:
			{
				return API_SUCCESS;
			}
		default:
			{
				return API_FAILURE;
			}
		}
	}

	long 
	RunningContext::TransactionTimeout() const 
	{ 
		return _transactionTimeout; 
	}

	void 
	RunningContext::TransactionTimeout(long val) 
	{
		_transactionTimeout = val; 
	}


	RunningContext::~RunningContext(void)
	{
		FUNCTRACKER;
	}


	BOOL 
	RunningContext::InboundPending()
	{
		return _inbound->InboundPending();
	}


	IwMessagePtr 
	RunningContext::GetInboundMessage(IN Time timeout, OUT ApiErrorCode &res)
	{
		return _inbound->Wait(timeout, res);
	}

	LightweightProcess::LightweightProcess(
		IN LpHandlePair pair, 
		IN const string &owner_name ,
		IN BOOL start_suspended):
	RunningContext(pair,owner_name,start_suspended)
	{

	}

	LightweightProcess::~LightweightProcess(void)
	{

	}

	IW_CORE_API int
	GetCurrLpId()
	{
		RunningContext *proc = 
			GetCurrRunningContext();

		if (GetCurrRunningContext() != NULL )
		{
			return proc->ProcessId();
		} else {
			return IW_UNDEFINED;
		}

	}

	IW_CORE_API RunningContext*
	GetCurrRunningContext()
	{
		PVOID fiber = ::GetCurrentFiber();
		if (GetTlsProcMap() == NULL || 
			fiber == (PVOID)NON_FIBEROUS_THREAD || 
			fiber == NULL)
		{
			return NULL;
		}

		ProcMap::iterator iter =  GetTlsProcMap()->find(::GetCurrentFiber());
		return  (iter == GetTlsProcMap()->end()) ? NULL : iter->second;
	}

	IW_CORE_API string 
	GetCurrLpName()
	{

		RunningContext *proc = 
			GetCurrRunningContext();

		if (GetCurrRunningContext() != NULL )
		{
			return proc->Name();
		} else {
			return "NOT CCU THREAD";
		}
	}

	IW_CORE_API void RegisterContext(RunningContext *ctx)
	{
		FUNCTRACKER;

		if (ctx == NULL)
			return;

		LogDebug("=== START service:" << ctx->_serviceId << " inbound:" << ctx->_inbound << " START ===");

		RegistrationGuard guard(ctx->_inbound,ctx->_serviceId);

		//
		// register itself within thread 
		// local storage
		//
		PVOID fiber = ::GetCurrentFiber();

		if (GetTlsProcMap()->find(fiber) != GetTlsProcMap()->end())
		{
			LogCrit("Fiber id already exists in the map. Have you run the routine manually?")
				throw;
		}

		if (fiber == (PVOID)NULL || 
			fiber == NON_FIBEROUS_THREAD )
		{
			// should never happen
			LogCrit("Shut 'er down Clancy, she's pumping mud!")
				throw critical_exception("cannot determine thread type");
		}


		(*GetTlsProcMap())[fiber] = ctx ;

		guard.dismiss();

	}

	IW_CORE_API RunningContext* 
	UnregisterContext()
	{
		RunningContext* ctx = 
			GetCurrRunningContext();

		if (ctx == NULL)
			return NULL;

		ctx->_inbound->Poison();
		ctx->_bucket->flush();


		PVOID fiber = ::GetCurrentFiber();
		GetTlsProcMap()->erase(GetTlsProcMap()->find(fiber));

		csp::CPPCSP_Yield();

		ctx->_outbound->Send(new MsgShutdownEvt());

		LocalProcessRegistrar::Instance().UnregisterChannel(ctx->_inbound->GetObjectUid());

		LogDebug("=== END service:" << ctx->_serviceId << " inbound:" << ctx->_inbound << " END ===");

		return ctx;

	}


	void
	LightweightProcess::run()
	{
		RegisterContext(this);
		
		//
		// Wait for resume message to start the process.
		//
		if (_startSuspended == TRUE)
		{
			ApiErrorCode res = API_SUCCESS;
			do 
			{
				IwMessagePtr msg =  _inbound->Wait(Seconds(60),res);
				if (res == API_TIMEOUT)
				{
					continue;
				}

				switch (msg->message_id)
				{
				case MSG_PROC_SHUTDOWN_EVT:
					{
						goto clean;
					}
				case MSG_PROC_RESUME:
					{
						break;
					}
				default:
					{
						BOOL res = HandleOOBMessage(msg);
						if (res == FALSE)
						{
							LogDebug("Unhandled OOB msg");
							goto clean;
						}
					}
				}// switch
			} while (true);
		}; // if

		try
		{
			real_run();
		} 
		catch (exception &e)
		{
			LogWarn("Exception proc:" << Name() << ", what:" << e.what());
		}

clean:
		UnregisterContext();
		
	}

	IW_CORE_API ApiErrorCode
	BootProcesses(IN ConfigurationPtr conf,
		IN FactoryPtrList &factoriesList,
		OUT HandlePairList &procHandlePairs,
		OUT HandlesVector &shutdownHandles)
	{

		BOOL all_booted = TRUE;

		int default_timeout = conf->GetInt("default_boot_time");

		for (FactoryPtrList::iterator i = factoriesList.begin(); 
			i !=  factoriesList.end(); 
			++i)
		{
			// listen to process shutdown event
			DECLARE_NAMED_HANDLE(shutdown_handle);
			DECLARE_NAMED_HANDLE_PAIR(proc_pair);

			AddShutdownListener(proc_pair,shutdown_handle);
			try 
			{
				LightweightProcess *p = (*i)->Create(proc_pair,conf);
				string name = p->Name();

				LogInfo("Booting process name:" << name << ", service uri:" << p->ServiceId());


				Run(p);
				if (IW_FAILURE(GetCurrRunningContext()->WaitTillReady(MilliSeconds(default_timeout), proc_pair)))
				{
					LogCrit("Cannot start proc:" << name);
					all_booted = FALSE;
					break;
				};

			} 
			catch (exception &e)
			{
				LogWarn("Exception during booting one of the processes, what:" << e.what());
				goto exit;
			}


			procHandlePairs.push_back(proc_pair);
			shutdownHandles.push_back(shutdown_handle);

		};

		return API_SUCCESS;
exit:
		return API_FAILURE;

	}

	IW_CORE_API ApiErrorCode
    LoadConfiguredModules(IN ConfigurationPtr conf,
		OUT FactoryPtrList &factoriesList)
	{
		
		ListOfAny modules;
		conf->GetArray("modules",modules);

		list<HMODULE> modules_list;

		
		for (ListOfAny::iterator iter = modules.begin(); 
			iter!=modules.end();
			++iter)
		{
			string &module_name = 
				any_cast<string>(*iter);

			HMODULE handle = ::LoadLibraryA(module_name.c_str());
			if (handle ==  NULL)
			{
				LogSysError("LoadLibraryA");
				LogCrit("LoadConfiguredModules - module_name:"<< module_name)
				return API_FAILURE;
			}

			modules_list.push_back(handle);

			FARPROC f = ::GetProcAddress(handle,"?GetIwFactory@ivrworx@@YAPAVIProcFactory@1@XZ");
			if (f ==  NULL)
			{
				LogSysError("GetProcAddress");
				LogCrit("Error Loading :" << module_name);
				goto error;
			}

			IProcFactory *factory = ((IWPROC)f)();
			factoriesList.push_back(ProcFactoryPtr(factory));

			LogInfo("LoadConfiguredModules - loaded module_name:" << module_name);

		}


		return API_SUCCESS;

error:

		// this should clear shared pointers
		factoriesList.clear();

		for (list<HMODULE>::iterator iter = modules_list.begin(); 
			iter!=modules_list.end();
			++iter)
		{
			::FreeLibrary(*iter);
		}

		return API_FAILURE;
	
	};

	IW_CORE_API ApiErrorCode
	BootModulesSimple(
		IN ConfigurationPtr conf,
		IN ScopedForking &forking,
		IN const FactoryPtrList &factories_list,
		OUT HandlePairList &proc_handlepairs,
		OUT HandlesVector &selected_handles)
	{
		
		int default_boot_time = conf->GetInt("default_boot_time");

		if (factories_list.size() == 0)
		{
			LogWarn("No processes to boot, exiting.");
			return API_SUCCESS;
		};


		BOOL all_booted = TRUE;
		for (FactoryPtrList::const_iterator i = factories_list.begin(); 
			i !=  factories_list.end(); 
			++i)
		{
			// listen to process shutdown event
			DECLARE_NAMED_HANDLE(shutdown_handle);
			DECLARE_NAMED_HANDLE_PAIR(proc_pair);

			AddShutdownListener(proc_pair,shutdown_handle);
			try 
			{
				LightweightProcess *p = (*i)->Create(proc_pair,conf);
				const string &name = p->Name();

				LogInfo("Booting process name:" << name << ", service uri:" << p->ServiceId());


				FORK(p);
				if (IW_FAILURE(GetCurrRunningContext()->WaitTillReady(MilliSeconds(default_boot_time), proc_pair)))
				{
					LogCrit("Cannot start proc:" << name);
					all_booted = FALSE;
					break;
				};

			} 
			catch (exception &e)
			{
				LogWarn("Exception during booting one of the processes, what:" << e.what());
				goto exit;
			}


			proc_handlepairs.push_back(proc_pair);
			selected_handles.push_back(shutdown_handle);

		};

		if (all_booted == FALSE)
		{
			goto exit;
		}

		return API_SUCCESS;

		
exit:

		// shutdown in reverse order (pop) first one
		ShutdownModules(proc_handlepairs,conf);
		return API_FAILURE;

	}

	IW_CORE_API ApiErrorCode
	ShutdownModules(IN HandlePairList &proc_handlepairs,
		IN ConfigurationPtr conf)
	{
		// used for shutdown timeout also
		int default_boot_time = conf->GetInt("default_boot_time");
		for (HandlePairList::reverse_iterator i = proc_handlepairs.rbegin(); 
			i!= proc_handlepairs.rend();
			++i)
		{
			LogInfo("Shutting down proc:" << (*i).inbound->HandleName());
			GetCurrRunningContext()->Shutdown(Seconds(default_boot_time), (*i));
		}

		return API_SUCCESS;
	}

}



#pragma pop_macro("SendMessage")
