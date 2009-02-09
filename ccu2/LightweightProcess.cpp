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
#include "CcuLogger.h"
#include "Profiler.h"

namespace ivrworx
{

#define  CCU_NON_FIBEROUS_THREAD ((PVOID)0x1E00)

	typedef 
	map<PVOID, LightweightProcess*> ProcMap;

	__declspec (thread) ProcMap *tl_procMap = NULL;


	LightweightProcess::LightweightProcess(
		IN LpHandlePair pair,
		IN const wstring &owner_name ):
	_pair(pair),
	_inbound(pair.inbound),
	_outbound(pair.outbound),
	_bucket(new Bucket()),
	_transactionTimeout(5000),
	_processAlias(IX_UNDEFINED)
	{
		FUNCTRACKER;

		// process id is as its inbound handle id
		Init(_inbound->GetObjectUid(), owner_name);
	}

	LightweightProcess::LightweightProcess(
		LpHandlePair pair,
		int process_alias,
		const wstring &owner_name
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
	LightweightProcess::Init(int process_id, wstring owner_name)
	{

		FUNCTRACKER;

		if (_inbound == _outbound)
		{
			throw;
		}

		if (_inbound != CCU_NULL_LP_HANDLE)
		{
			_inbound->HandleName(owner_name);
		}

		if (_outbound != CCU_NULL_LP_HANDLE)
		{
			_outbound->HandleName(owner_name);
		}

		_processId = process_id;

		_inbound->Direction(CCU_MSG_DIRECTION_INBOUND);
		_outbound->Direction(CCU_MSG_DIRECTION_OUTBOUND);

		_name = owner_name;

	}




	BOOL 
	LightweightProcess::HandleOOBMessage(IxMsgPtr msg)
	{
		FUNCTRACKER;

		LogDebug("First Chance OOB message=["<< msg->message_id_str <<"]");

		switch (msg->message_id)
		{
		case CCU_MSG_PING:
			{
				SendResponse(msg,new CcuMsgPong());
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
			fiber == CCU_NON_FIBEROUS_THREAD )
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

	wstring 
	LightweightProcess::Name()
	{ 
		return _name; 
	}

	void 
	LightweightProcess::Name(IN const wstring &val) 
	{
		_name = val; 
	}

	IxApiErrorCode
	LightweightProcess::SendMessage(
		IN IxProcId qid, 
		IN IxMessage *message)
	{
		return SendMessage(qid, IxMsgPtr(message));
	}


	IxApiErrorCode
	LightweightProcess::SendResponse(
		IN IxMsgPtr request, 
		IN IxMessage* response)
	{
		IX_PROFILE_CODE(response->copy_data_on_response(request.get()));
		return SendMessage(IxMsgPtr(response));
	}

	IxApiErrorCode
		LightweightProcess::SendMessage(
		IN IxProcId qid, 
		IN IxMsgPtr message)
	{

		message->dest.handle_id = qid;
		return SendMessage(message);
	}

	IxApiErrorCode
	LightweightProcess::SendMessage(IxMsgPtr message)
	{

		IX_PROFILE_FUNCTION();

		int handle_id = message->dest.handle_id;
		if (message->source.handle_id == IX_UNDEFINED)
		{
			message->source.handle_id = _processId;
		}


		LpHandlePtr localHandlePtr;

		// send message via the same IPC interface they arrived from
		if (message->preferrable_ipc_interface != IX_UNDEFINED)
		{
			localHandlePtr = LocalProcessRegistrar::Instance().GetHandle(message->preferrable_ipc_interface);
		}


		if (localHandlePtr == CCU_NULL_LP_HANDLE)
		{
			localHandlePtr = 
				LocalProcessRegistrar::Instance().GetHandle(handle_id,message->dest.queue_path);
		}


		if (localHandlePtr != CCU_NULL_LP_HANDLE)
		{
			localHandlePtr->Send(message);
			return CCU_API_SUCCESS;
		}

		LogWarn("Unknown destination for message=[" << message->message_id_str 
			<< "] dest=["<< handle_id << "@" << message->dest.queue_path << "]");

		return CCU_API_FAILURE;

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


	IxApiErrorCode
		LightweightProcess::DoRequestResponseTransaction(
		IN LpHandlePtr dest_handle, 
		IN IxMsgPtr request, 
		OUT IxMsgPtr &response,
		IN Time timout,
		IN wstring transaction_name)
	{

		FUNCTRACKER;

		DECLARE_NAMED_HANDLE(txn_handle);
		txn_handle->HandleName(transaction_name); // for logging purposes
		txn_handle->Direction(CCU_MSG_DIRECTION_INBOUND);

		RegistrationGuard guard(txn_handle);

		request->source.handle_id = txn_handle->GetObjectUid();
		request->transaction_id = GenerateNewTxnId();

		IX_PROFILE_CODE(dest_handle->Send(request));

		IxApiErrorCode res = WaitForTxnResponse(
			txn_handle,
			response,
			timout);

		if (res == CCU_API_TIMEOUT)
		{
			LogDebug("TIMEOUT txn=[" << request->transaction_id<< "]");
		}

		return res;

	}

	IxApiErrorCode	
		LightweightProcess::DoRequestResponseTransaction(
		IN IxProcId dest_proc_id, 
		IN IxMsgPtr request, 
		OUT IxMsgPtr &response, 
		IN Time timout, 
		IN wstring transaction_name)
	{

		FUNCTRACKER;

		DECLARE_NAMED_HANDLE(txn_handle);
		txn_handle->HandleName(transaction_name);
		txn_handle->Direction(CCU_MSG_DIRECTION_INBOUND);

		RegistrationGuard guard(txn_handle);


		request->source.handle_id = txn_handle->GetObjectUid();
		request->dest.handle_id = dest_proc_id;
		request->transaction_id = GenerateNewTxnId();

		IxApiErrorCode res = SendMessage(request);
		if (CCU_FAILURE(res))
		{
			return res;
		}

		res = WaitForTxnResponse(
			txn_handle,
			response,
			timout);

		if (res == CCU_API_TIMEOUT)
		{
			LogDebug("TIMEOUT txn=[" << request->transaction_id<< "]");
		}

		return res;

	}

	IxApiErrorCode	
	LightweightProcess::WaitForTxnResponse(
		IN LpHandlePtr txn_handle,
		OUT IxMsgPtr &response,
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


		interrupted_handle_index = IX_UNDEFINED;
		IxApiErrorCode res= CCU_API_SUCCESS;


		//
		// Message waiting loop.
		//
		long timeLeftToWaitMs = GetMilliSeconds(timeout);
		while (timeLeftToWaitMs >= 0)
		{
			int start = ::GetTickCount();

			IxApiErrorCode err_code = 
				SelectFromChannels(
				list,
				MilliSeconds(timeLeftToWaitMs), 
				interrupted_handle_index, 
				response);

			if (CCU_FAILURE(err_code))
			{
				return err_code;
			}


			if (interrupted_handle_index == txn_term_index)
			{
				BOOL res = this->HandleOOBMessage(response);
				if (res == FALSE)
				{
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


	IxApiErrorCode
	LightweightProcess::SendReadyMessage()
	{
		_outbound->Send(new CcuMsgProcReady());

		return CCU_API_SUCCESS;
	}

	IxApiErrorCode
	LightweightProcess::Ping(IxProcId qid)
	{
		FUNCTRACKER;

		IxMsgPtr dummy_response = CCU_NULL_MSG;

		IxApiErrorCode res = this->DoRequestResponseTransaction(
			qid,
			IxMsgPtr(new CcuMsgPing()),
			dummy_response,
			MilliSeconds(_transactionTimeout),
			L"Ping-Pong TXN");

		return res;

	}

	IxApiErrorCode
	LightweightProcess::Ping(IN LpHandlePair pair)
	{
		FUNCTRACKER;

		IxMsgPtr dummy_response = CCU_NULL_MSG;

		IxApiErrorCode res = this->DoRequestResponseTransaction(
			pair.inbound,
			IxMsgPtr(new CcuMsgPing()),
			dummy_response,
			MilliSeconds(_transactionTimeout),
			L"Ping-Pong TXN");

		return res;

	}


	IxApiErrorCode
	LightweightProcess::WaitTillReady(Time time, LpHandlePair pair)
	{


		// CCU_MSG_PROC_READY must be the first message sent
		// so we do not create special process for running 
		// the transaction
		IxApiErrorCode res = CCU_API_FAILURE;
		IxMsgPtr response = pair.outbound->Wait(
			time,
			res);

		if (res == CCU_API_TIMEOUT)
		{
			Shutdown(Seconds(0),pair);
			return CCU_API_FAILURE;
		}

		switch (response->message_id)
		{
		case CCU_MSG_PROC_READY:
			{
				return CCU_API_SUCCESS;
			}
		default:
			{
				return CCU_API_FAILURE;
			}
		}
	}

	IxApiErrorCode
	LightweightProcess::Shutdown(IN Time time, IN LpHandlePair pair)
	{

		FUNCTRACKER;
		IxMsgPtr response = CCU_NULL_MSG;

		IxApiErrorCode res = this->DoRequestResponseTransaction(
			pair.inbound,
			IxMsgPtr(new CcuMsgShutdownReq()),
			response,
			time,
			L"Shutdown TXN");

		if (CCU_FAILURE(res))
		{
			return res;
		}

		switch (response->message_id)
		{
		case CCU_MSG_PROC_SHUTDOWN_ACK:
			{
				return CCU_API_SUCCESS;
			}
		default:
			{
				return CCU_API_FAILURE;
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


	IxMsgPtr 
	LightweightProcess::GetInboundMessage(IN Time timeout, OUT IxApiErrorCode &res)
	{
		return _inbound->Wait(timeout, res);
	}

	int
	IxGetCurrLpId()
	{
		LightweightProcess *proc = 
			GetCurrLightWeightProc();

		if (GetCurrLightWeightProc() != NULL )
		{
			return proc->ProcessId();
		} else {
			return IX_UNDEFINED;
		}

	}

	LightweightProcess*
	GetCurrLightWeightProc()
	{
		PVOID fiber = ::GetCurrentFiber();
		if (tl_procMap== NULL || 
			fiber == (PVOID)CCU_NON_FIBEROUS_THREAD || 
			fiber == NULL)
		{
			return NULL;
		}

		LightweightProcess *proc = 
			(*tl_procMap)[::GetCurrentFiber()];

		return proc;

	}

	wstring 
	IxGetCurrLpName()
	{

		LightweightProcess *proc = 
			GetCurrLightWeightProc();

		if (GetCurrLightWeightProc() != NULL )
		{
			return proc->Name();
		} else {
			return L"NOT CCU THREAD";
		}
	}
}



