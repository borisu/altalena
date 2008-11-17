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



#define  CCU_NON_FIBEROUS_THREAD ((PVOID)0x1E00)

typedef 
map<PVOID, LightweightProcess*> ProcMap;

__declspec (thread) ProcMap *tl_procMap= NULL;


LightweightProcess::LightweightProcess(
	LpHandlePair pair,
	const wstring &owner_name 
	):
_pair(pair),
_inbound(pair.inbound),
_outbound(pair.outbound),
_bucket(new Bucket()),
_transactionTimeout(5000),
_transactionTerminator(pair.inbound)
{
	FUNCTRACKER;

	Init(GetObjectUid(), owner_name);
}

LightweightProcess::LightweightProcess(
									   LpHandlePair pair,
									   int process_id,
									   const wstring &owner_name
									   ):
_pair(pair),
_inbound(pair.inbound),
_outbound(pair.outbound),
_bucket(new Bucket()),
_transactionTimeout(5000),
_transactionTerminator(pair.inbound)

{
	FUNCTRACKER;

	Init(process_id, owner_name);
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
		_inbound->OwnerProc(
			process_id,
			owner_name);
	}

	if (_outbound!= CCU_NULL_LP_HANDLE)
	{
		_outbound->OwnerProc(
			process_id,
			owner_name);
	}
		
	if (_inbound != CCU_NULL_LP_HANDLE)
	{
		LocalProcessRegistrar::Instance().RegisterChannel(process_id,_inbound);
		_inbound->FirstChanceOOBMsgHandler(this);
	}


	_processId = process_id;


	_inbound->Direction(CCU_MSG_DIRECTION_INBOUND);
	_outbound->Direction(CCU_MSG_DIRECTION_OUTBOUND);

	_name = owner_name;
	

}




BOOL 
LightweightProcess::HandleOOBMessage(CcuMsgPtr msg)
{
	FUNCTRACKER;

	LogDebug("First Chance OOB message=["<< msg->message_id_str <<"]");

	BOOL res = FALSE;
	switch (msg->message_id)
	{
		case CCU_MSG_PING:
			{
				LogDebug("Pong!");
				SendResponse(msg,new CcuMsgPong());
				res = TRUE;
				break;
			}
		default:
			{
				res =  FALSE;
			}
	}

	return res;
}

void
LightweightProcess::run()
{
	
	FUNCTRACKER;

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
	catch (exception e)
	{
		LogWarn("Exception in process" << e.what());
	}
	catch (...)
	{
		LogWarn("Exception in process");
	}

	_bucket->flush();

	tl_procMap->erase(fiber);

}

wstring 
LightweightProcess::Name()
{ 
	return _name; 
}

void 
LightweightProcess::Name(const wstring &val) 
{
	_name = val; 
}

CcuApiErrorCode
LightweightProcess::SendMessage(CcuProcId qid, 
								 CcuMessage *message)
{
	return SendMessage(qid, CcuMsgPtr(message));
}


CcuApiErrorCode
LightweightProcess::SendResponse(CcuMsgPtr request, 
									   CcuMessage* response)
{
	
	
	response->copy_data_on_response(request.get());
	return SendMessage(CcuMsgPtr(response));

}

CcuApiErrorCode
LightweightProcess::SendMessage(CcuProcId qid, 
								 CcuMsgPtr message)
{
	
	message->dest.proc_id = qid;

	return SendMessage(message);
}

CcuApiErrorCode
LightweightProcess::SendMessage(CcuMsgPtr message)
{
	
	int proc_id = message->dest.proc_id;
	if (message->source.proc_id == CCU_UNDEFINED)
	{
		message->source.proc_id = _processId;
	}
	
	
	LpHandlePtr localHandlePtr;

	// send message via the same IPC interface they arrived from
	if (message->preferrable_ipc_interface != CCU_UNDEFINED)
	{
		localHandlePtr = LocalProcessRegistrar::Instance().GetHandle(message->preferrable_ipc_interface);
	}
	

	if (localHandlePtr == CCU_NULL_LP_HANDLE)
	{
		localHandlePtr = 
			LocalProcessRegistrar::Instance().GetHandle(proc_id,message->dest.queue_path);
	}
	

	if (localHandlePtr != CCU_NULL_LP_HANDLE)
	{
		localHandlePtr->Send(message);
		return CCU_API_SUCCESS;
	}

	LogWarn("Unknown destination for message=[" << message->message_id_str 
		<< "] dest=["<< proc_id << "@" << message->dest.queue_path << "]");

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


CcuApiErrorCode
LightweightProcess::TerminatePendingTransaction(IN exception e)
{
	_transactionTerminator->Send(new CcuMsgShutdownReq());

	return CCU_API_SUCCESS;
}

CcuApiErrorCode
LightweightProcess::DoRequestResponseTransaction(
	IN LpHandlePtr dest_handle, 
	IN CcuMsgPtr request, 
	IN EventsSet &responses,
	OUT CcuMsgPtr &response,
	IN Time timout,
	IN wstring transaction_name)
{

	FUNCTRACKER;

	DECLARE_NAMED_HANDLE_PAIR(txn_pair);
	txn_pair.inbound->OwnerProcName(transaction_name); // for logging purposes

	RegistrationGuard guard(txn_pair.inbound);

	request->source.proc_id = guard.GetObjectUid();

	dest_handle->Send(request);

	CcuApiErrorCode res = WaitForTxnResponse(
		txn_pair,
		responses,
		response,
		timout);

	return res;

}

CcuApiErrorCode	
LightweightProcess::DoRequestResponseTransaction(
	IN CcuProcId dest_proc_id, 
	IN CcuMsgPtr request, 
	IN EventsSet &responses, 
	OUT CcuMsgPtr &response, 
	IN Time timout, 
	IN wstring transaction_name)
{

	FUNCTRACKER;

	DECLARE_NAMED_HANDLE_PAIR(txn_pair);
	txn_pair.inbound->OwnerProcName(transaction_name);

	RegistrationGuard guard(txn_pair.inbound);


	request->source.proc_id = guard.GetObjectUid();
	request->dest.proc_id = dest_proc_id;

	CcuApiErrorCode res = SendMessage(request);
	if (CCU_FAILURE(res))
	{
		return res;
	}

	res = WaitForTxnResponse(
		txn_pair,
		responses,
		response,
		timout);

	return res;

}

CcuApiErrorCode	
LightweightProcess::WaitForTxnResponse(
								  IN LpHandlePair txn_pair,
								  IN EventsSet &responses,
								  OUT CcuMsgPtr &response,
								  IN Time timeout)
{
	if (responses.empty())
	{
		return CCU_API_SUCCESS;
	}

	//
	// Create lists of handles that 
	// we are waiting messages from.
	//
	HandlesList list;
	int interrupted_handle_index = 0;

	list.push_back(_transactionTerminator);
	const int txn_term_index = interrupted_handle_index++;

	list.push_back(txn_pair.inbound);
	const int txn_inbound_index = interrupted_handle_index++;


	interrupted_handle_index = CCU_UNDEFINED;
	CcuApiErrorCode res= CCU_API_SUCCESS;


	//
	// Message waiting loop.
	//
	long timeLeftToWaitMs = GetMilliSeconds(timeout);
	while (timeLeftToWaitMs >= 0)
	{
		int start = ::GetTickCount();
		CcuApiErrorCode err_code = 
			SelectFromChannels(
			list,
			MilliSeconds(timeLeftToWaitMs), 
			interrupted_handle_index, 
			CCU_NULL_MSG,
			true);


		if (err_code == CCU_API_TIMEOUT)
		{
			return err_code;
		}

		if (interrupted_handle_index == txn_term_index)
		{
			BOOL res = HandleOOBMessage(_transactionTerminator->Wait(Seconds(0),err_code));
			if (res == FALSE)
			{
				throw exception("Transaction was terminated");
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

			response = 
				txn_pair.inbound->WaitForMessages(
				MilliSeconds(0),
				responses,
				res
				);

			break;

		}
	}

	return res;

}


CcuApiErrorCode
LightweightProcess::SendReadyMessage()
{
	_outbound->Send(new CcuMsgProcReady());

	return CCU_API_SUCCESS;
}

CcuApiErrorCode
LightweightProcess::Ping(CcuProcId qid)
{
	FUNCTRACKER;

	CcuMsgPtr dummy_response = CCU_NULL_MSG;

	EventsSet map;
	map.insert(CCU_MSG_PONG);

	CcuApiErrorCode res = this->DoRequestResponseTransaction(
		qid,
		CcuMsgPtr(new CcuMsgPing()),
		map,
		dummy_response,
		MilliSeconds(_transactionTimeout),
		L"Ping-Pong TXN");


	return res;

}

CcuApiErrorCode
LightweightProcess::Ping(IN LpHandlePair pair)
{
	FUNCTRACKER;

	CcuMsgPtr dummy_response = CCU_NULL_MSG;

	EventsSet map;
	map.insert(CCU_MSG_PONG);

	CcuApiErrorCode res = this->DoRequestResponseTransaction(
		pair.inbound,
		CcuMsgPtr(new CcuMsgPing()),
		map,
		dummy_response,
		MilliSeconds(_transactionTimeout),
		L"Ping-Pong TXN");

	return res;

}


CcuApiErrorCode
LightweightProcess::WaitTillReady(Time time, LpHandlePair pair)
{


	// CCU_MSG_PROC_READY must be the first message sent
	// so we do not create special process for running 
	// the transaction
	CcuApiErrorCode res = CCU_API_FAILURE;
	pair.outbound->WaitForMessages(
		time,
		res,
		SL(CCU_MSG_PROC_READY));

	if (res == CCU_API_TIMEOUT)
	{
		Shutdown(Seconds(0),pair);

		return CCU_API_FAILURE;
	}

	return CCU_API_SUCCESS;

}

CcuApiErrorCode
LightweightProcess::Shutdown(IN Time time, IN LpHandlePair pair)
{

	FUNCTRACKER;
	CcuMsgPtr response = CCU_NULL_MSG;

	CcuMsgShutdownReq *msg = new CcuMsgShutdownReq();

	EventsSet map;
	map.insert(CCU_MSG_PROC_SHUTDOWN_ACK);


	CcuApiErrorCode res = this->DoRequestResponseTransaction(
		pair.inbound,
		CcuMsgPtr(msg),
		map,
		response,
		time,
		L"Shutdown TXN");

	return res;

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
	LocalProcessRegistrar::Instance().UnregisterChannel(GetObjectUid());
}


BOOL 
LightweightProcess::InboundPending()
{
	return _inbound->InboundPending();
}


CcuMsgPtr 
LightweightProcess::GetInboundMessage()
{
	return _inbound->Wait();
}

InterruptibleBySemaphore::InterruptibleBySemaphore(LpHandlePtr interruptible_channel)
:_sim(new SemaphoreInterruptor())
{
	interruptible_channel->HandleInterruptor(shared_dynamic_cast<Interruptor>(_sim));

}



HANDLE
InterruptibleBySemaphore::MsgHandle()
{
	return _sim->Handle();
}

InterruptibleBySemaphore::~InterruptibleBySemaphore()
{
	
}

int
GetCurrCcuProcId()
{
	LightweightProcess *proc = 
		GetCurrLightWeightProc();

	if (GetCurrLightWeightProc() != NULL )
	{
		return proc->ProcessId();
	} else {
		return CCU_UNDEFINED;
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
GetCurrThreadOwner()
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



