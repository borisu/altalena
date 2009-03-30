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

namespace ivrworx
{

	typedef 
	map<PVOID, LightweightProcess*> ProcMap;

	__declspec (thread) ProcMap *tl_procMap = NULL;


	LightweightProcess::LightweightProcess(
		IN LpHandlePair pair,
		IN const string &owner_name):
	_pair(pair),
	_inbound(pair.inbound),
	_outbound(pair.outbound),
	_bucket(new Bucket()),
	_transactionTimeout(5000),
	_processAlias(IW_UNDEFINED)
	{
		FUNCTRACKER;

		// process id is as its inbound handle id
		Init(_inbound->GetObjectUid(), owner_name);
	}

	LightweightProcess::LightweightProcess(
		LpHandlePair pair,
		int process_alias,
		const string &owner_name
		):
	_pair(pair),
	_inbound(pair.inbound),
	_outbound(pair.outbound),
	_bucket(new Bucket()),
	_transactionTimeout(5000),
	_processAlias(process_alias)
	{
		FUNCTRACKER;

		// process id is as its inbound handle id
		Init(_inbound->GetObjectUid(), owner_name);
	}

	void
	LightweightProcess::Init(int process_id, string owner_name)
	{

		FUNCTRACKER;

		if (_inbound == _outbound)
		{
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
	LightweightProcess::HandleOOBMessage(IwMessagePtr msg)
	{
		FUNCTRACKER;

		LogDebug("First Chance OOB message=["<< msg->message_id_str <<"]");

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

	void
	LightweightProcess::run()
	{

		FUNCTRACKER;

		LogDebug("=== LP " << Name() << " pid=" << ProcessId() << ", alias=" << _processAlias << " START ===");

		RegistrationGuard guard(_inbound,_processAlias);

		// Created only once -  thread local storage 
		// of the fibers.
		if (tl_procMap == NULL)
		{
			tl_procMap = new ProcMap();
		}

		//
		// register itself within thread 
		// local storage
		//
		PVOID fiber = ::GetCurrentFiber();

		if (tl_procMap->find(fiber) != tl_procMap->end())
		{
			LogCrit("Fiber id already exists in the map. Have you run the routine manually?")
			throw;
		}

		if (fiber == (PVOID)NULL || 
			fiber == NON_FIBEROUS_THREAD )
		{
			// should never happen
			LogCrit("Shut 'er down Clancy, she's pumping mud!")
			throw;
		}

		(*tl_procMap)[fiber] = this;

		
		try
		{
			real_run();
		} 
		catch (std::exception e)
		{
			LogWarn("Exception in process=[" << Name() << "] what=[" << e.what() << "]");
		}

		

		_inbound->Poison();

		_bucket->flush();
         
		tl_procMap->erase(fiber);

		csp::CPPCSP_Yield();

		LogDebug("=== LP " << Name() << " pid=" << ProcessId() << ", alias=" << _processAlias << " END ===");

	}

	string 
	LightweightProcess::Name()
	{ 
		return _name; 
	}

	void 
	LightweightProcess::Name(IN const string &val) 
	{
		_name = val; 
	}

	ApiErrorCode
	LightweightProcess::SendMessage(
		IN ProcId qid, 
		IN IwMessage *message)
	{
		return SendMessage(qid, IwMessagePtr(message));
	}


	ApiErrorCode
	LightweightProcess::SendResponse(
		IN IwMessagePtr request, 
		IN IwMessage* response)
	{
		IX_PROFILE_CODE(response->copy_data_on_response(request.get()));
		return SendMessage(IwMessagePtr(response));
	}

	ApiErrorCode
	LightweightProcess::SendMessage(
		IN ProcId qid, 
		IN IwMessagePtr message)
	{

		message->dest.handle_id = qid;
		return SendMessage(message);
	}

	ApiErrorCode
	LightweightProcess::SendMessage(
		IN LpHandlePtr handle, 
		IN IwMessagePtr message)
	{
		return SendMessage(handle->GetObjectUid(), message);
	}


	ApiErrorCode
	LightweightProcess::SendMessage(IwMessagePtr message)
	{

		IX_PROFILE_FUNCTION();

		int handle_id = message->dest.handle_id;
		if (message->source.handle_id == IW_UNDEFINED)
		{
			message->source.handle_id = _processId;
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

		LogWarn("Unknown destination for message=[" << message->message_id_str 
			<< "] dest=["<< handle_id << "@" << message->dest.queue_path << "]");

		return API_FAILURE;

	}

	int
	LightweightProcess::ProcessId()
	{
		return _processId;
	}


	void
	LightweightProcess::Join(BucketPtr bucket)
	{
		bucket->fallInto();
	}


	ApiErrorCode
		LightweightProcess::DoRequestResponseTransaction(
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

		IX_PROFILE_CODE(dest_handle->Send(request));

		ApiErrorCode res = WaitForTxnResponse(
			txn_handle,
			response,
			timeout);

		if (res == API_TIMEOUT)
		{
			LogDebug("TIMEOUT txn=[" << request->transaction_id<< "]");
		}

		return res;

	}

	ApiErrorCode	
		LightweightProcess::DoRequestResponseTransaction(
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
			return res;
		}

		res = WaitForTxnResponse(
			txn_handle,
			response,
			timout);

		if (res == API_TIMEOUT)
		{
			LogDebug("TIMEOUT txn=[" << request->transaction_id<< "]");
		}

		return res;

	}

	ApiErrorCode	
	LightweightProcess::WaitForTxnResponse(
		IN LpHandlePtr txn_handle,
		OUT IwMessagePtr &response,
		IN Time timeout)
	{

		//
		// Create lists of handles that 
		// we are waiting messages from.
		//
		HandlesList list;
		int interrupted_handle_index = 0;

		list.push_back(_inbound);
		const int txn_term_index = interrupted_handle_index++;
		list.push_back(txn_handle);
		const int txn_inbound_index = interrupted_handle_index++;


		interrupted_handle_index = IW_UNDEFINED;
		ApiErrorCode res= API_SUCCESS;


		//
		// Message waiting loop.
		//
		sign32 timeLeftToWaitMs = GetMilliSeconds(timeout);
		if (timeLeftToWaitMs < 0)
		{
			LogWarn("Illegal value for timeout " << timeLeftToWaitMs);
			return API_FAILURE;
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


			if (interrupted_handle_index == txn_term_index)
			{
				BOOL res = this->HandleOOBMessage(response);
				if (res == FALSE)
				{
					LogWarn("Unhandled OOB message [" << response->message_id_str << "], throwing exception.");
					throw std::exception("Transaction was terminated");
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
			else 
			{

				break;

			}
		}

		return res;

	}


	ApiErrorCode
	LightweightProcess::SendReadyMessage()
	{
		_outbound->Send(new MsgProcReady());

		return API_SUCCESS;
	}

	ApiErrorCode
	LightweightProcess::Ping(ProcId qid)
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
	LightweightProcess::Ping(IN LpHandlePair pair)
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
	LightweightProcess::WaitTillReady(Time time, LpHandlePair pair)
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
	LightweightProcess::Shutdown(IN Time time, IN LpHandlePair pair)
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
	LightweightProcess::TransactionTimeout() const 
	{ 
		return _transactionTimeout; 
	}

	void 
	LightweightProcess::TransactionTimeout(long val) 
	{
		_transactionTimeout = val; 
	}


	LightweightProcess::~LightweightProcess(void)
	{
		FUNCTRACKER;
	}


	BOOL 
	LightweightProcess::InboundPending()
	{
		return _inbound->InboundPending();
	}


	IwMessagePtr 
	LightweightProcess::GetInboundMessage(IN Time timeout, OUT ApiErrorCode &res)
	{
		return _inbound->Wait(timeout, res);
	}

	int
	GetCurrLpId()
	{
		LightweightProcess *proc = 
			GetCurrLightWeightProc();

		if (GetCurrLightWeightProc() != NULL )
		{
			return proc->ProcessId();
		} else {
			return IW_UNDEFINED;
		}

	}

	LightweightProcess*
	GetCurrLightWeightProc()
	{
		PVOID fiber = ::GetCurrentFiber();
		if (tl_procMap== NULL || 
			fiber == (PVOID)NON_FIBEROUS_THREAD || 
			fiber == NULL)
		{
			return NULL;
		}

		LightweightProcess *proc = 
			(*tl_procMap)[::GetCurrentFiber()];

		return proc;

	}

	string 
	GetCurrLpName()
	{

		LightweightProcess *proc = 
			GetCurrLightWeightProc();

		if (GetCurrLightWeightProc() != NULL )
		{
			return proc->Name();
		} else {
			return "NOT CCU THREAD";
		}
	}
}



