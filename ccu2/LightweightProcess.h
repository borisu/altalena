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

#pragma once
#include "ccu.h"
#include "UIDOwner.h"
#include "LpHandle.h"


using namespace csp;
using namespace boost;
using namespace std;

#define I_AM_READY SendReadyMessage();

typedef list<CSProcess*> ProcessList;


typedef
shared_ptr<Bucket> BucketPtr;

class LightweightProcess: 
	public CSProcess, 
	public IFirstChanceOOBMsgHandler
{
public:

	
	LightweightProcess(
		IN LpHandlePair pair, 
		IN const wstring &owner_name = L"");

	LightweightProcess(
		IN LpHandlePair pair, 
		IN int inbound_channel_id, 
		IN const wstring &owner_name = L"");


	virtual ~LightweightProcess(void);

	BucketPtr _bucket;

	static void Join(BucketPtr bucket);


	virtual void run();

	virtual void real_run() = 0;


	virtual wstring Name();

	virtual int ProcessId();

	virtual void Name(IN const wstring &val);


	virtual CcuApiErrorCode SendMessage(
		IN CcuProcId dest_channel_id, 
		IN CcuMessage* message);

	virtual CcuApiErrorCode SendMessage(
		IN CcuProcId dest_channel_id, 
		IN CcuMsgPtr message);

	virtual CcuApiErrorCode SendMessage(
		IN CcuMsgPtr message);

	virtual CcuApiErrorCode SendResponse(
		IN CcuMsgPtr request, 
		IN CcuMessage* response);

	virtual BOOL HandleOOBMessage(
		IN CcuMsgPtr msg);

	virtual BOOL InboundPending();

	virtual CcuMsgPtr GetInboundMessage(IN Time timeout, OUT CcuApiErrorCode &res);

	long TransactionTimeout() const ;

	void TransactionTimeout(long val);

	CcuApiErrorCode TerminatePendingTransaction(
		IN std::exception e);

	CcuApiErrorCode SendReadyMessage();

	//
	// Process Management
	//
	CcuApiErrorCode Ping(
		IN CcuProcId qid); 

	CcuApiErrorCode Ping(
		IN LpHandlePair pair); 

	CcuApiErrorCode Shutdown(
		IN Time timeout, 
		IN LpHandlePair pair);

	CcuApiErrorCode WaitTillReady(
		IN Time timeout, 
		IN LpHandlePair pair);

	//
	// Transaction Management
	//
	CcuApiErrorCode	DoRequestResponseTransaction(
		IN CcuProcId dest_proc_id, 
		IN CcuMsgPtr request, 
		IN EventsSet &responses,
		OUT CcuMsgPtr &response,
		IN Time timout,
		IN wstring transaction_name);

	CcuApiErrorCode	DoRequestResponseTransaction(
		IN LpHandlePtr dest_handle, 
		IN CcuMsgPtr request, 
		IN EventsSet &responses,
		OUT CcuMsgPtr &response,
		IN Time timout,
		IN wstring transaction_name);

	CcuApiErrorCode WaitForTxnResponse(
		IN LpHandlePtr txn_handle,
		IN EventsSet &responses,
		OUT CcuMsgPtr &response,
		IN Time timout);

	LpHandlePair _pair;

	LpHandlePtr _inbound;

	LpHandlePtr _outbound;


	friend class ScopedRTPConnectionGuard;

protected:

	long _transactionTimeout;

	LpHandlePtr _transactionTerminator;

	wstring _name;
	
	int _processId;

	int _processAlias;

private:

	void Init(int UID, wstring owner_name);

};

#pragma endregion

#pragma region ProcFuncRunner

template <class T, class Y>
class ProcFuncRunner: 
	public LightweightProcess
{
private:

	typename boost::function<T(Y*)> _function;

	typename Y* _instance;

protected:

	void real_run()
	{
		_res = _function(_instance);
	}

public:

	ProcFuncRunner(
		LpHandlePair pair,
		boost::function<T(Y *)> funct,
		Y *instance, 
		T res, 
		wstring name = L"")
		:LightweightProcess(pair,name),
	_function(funct),
	_instance(instance),
	_res(res)
	{

	}

	typename T &_res;
};


template <class Y>
class ProcVoidFuncRunner: 
	public LightweightProcess
{
private:

	typename boost::function<void(Y*)> _function;

	typename Y* _instance;

protected:

	void real_run()
	{
		_function(_instance);
	}

public:

	ProcVoidFuncRunner(
		LpHandlePair pair,
		boost::function<void(Y *)> funct,
		Y *instance, 
		wstring name = L"")
		:LightweightProcess(pair,name),
		_function(funct),
		_instance(instance)
	{

	}

};

#pragma endregion


//
// owners map, used for logging mostly
//
LightweightProcess*
GetCurrLightWeightProc();

wstring 
IxGetCurrLpName();

int
IxGetCurLpId();






