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
#include "IwBase.h"
#include "UIDOwner.h"
#include "LpHandle.h"
#include "Configuration.h"


using namespace csp;
using namespace boost;
using namespace std;

namespace ivrworx
{

#define I_AM_READY				SendReadyMessage()
#define	SEND_RESPONSE(P, R)		GetCurrRunningContext()->SendResponse(P, R)
#define	WAIT_TILL_READY(T, P)	GetCurrRunningContext()->WaitTillReady((T), (P))

typedef list<CSProcess*> ProcessList;


typedef
shared_ptr<Bucket> BucketPtr;

class RunningContext;

typedef shared_ptr<RunningContext>
RunningContextPtr;

IW_CORE_API void  RegisterContext(RunningContext* ctx);
IW_CORE_API RunningContext* UnegisterContext(void);

class IW_CORE_API AppData : boost::noncopyable
{
public:
	virtual int getType();
};

class IW_CORE_API RunningContext
{
public:

	RunningContext(
		IN LpHandlePair pair, 
		IN const string &owner_name = "",
		IN BOOL start_suspended = FALSE);

	
	virtual ~RunningContext(void);

	static void Join(BucketPtr bucket);

	virtual string Name();

	virtual void Name(IN const string &val);

	virtual string ServiceId();

	virtual void ServiceId(IN const string &serviceId);

	virtual int ProcessId();

#pragma push_macro("SendMessage")
#undef SendMessage
	virtual ApiErrorCode SendMessage(
		IN ProcId dest_channel_id, 
		IN IwMessage* message);

	virtual ApiErrorCode SendMessage(
		IN ProcId dest_channel_id, 
		IN IwMessagePtr message);

	virtual ApiErrorCode SendMessage(
		IN LpHandlePtr dest_handle, 
		IN IwMessagePtr message);

	virtual ApiErrorCode SendMessage(
		IN IwMessagePtr message);

	virtual ApiErrorCode SendResponse(
		IN IwMessagePtr request, 
		IN IwMessage* response);
#pragma pop_macro("SendMessage")

	virtual BOOL HandleOOBMessage(
		IN IwMessagePtr msg);

	virtual BOOL InboundPending();

	virtual IwMessagePtr GetInboundMessage(IN Time timeout, OUT ApiErrorCode &res);

	long TransactionTimeout() const ;

	void TransactionTimeout(long val);

	ApiErrorCode SendReadyMessage();

	//
	// Process Management
	//
	ApiErrorCode Ping(
		IN ProcId qid); 

	ApiErrorCode Ping(
		IN LpHandlePair pair); 

	ApiErrorCode Shutdown(
		IN Time timeout, 
		IN LpHandlePair pair);

	ApiErrorCode WaitTillReady(
		IN Time timeout, 
		IN LpHandlePair pair);

	//
	// Transaction Management
	//
	ApiErrorCode DoRequestResponseTransaction(
		IN ProcId dest_proc_id, 
		IN IwMessagePtr request, 
		OUT IwMessagePtr &response,
		IN Time timout,
		IN string transaction_name);

	ApiErrorCode DoRequestResponseTransaction(
		IN LpHandlePtr dest_handle, 
		IN IwMessagePtr request, 
		OUT IwMessagePtr &response,
		IN Time timout,
		IN string transaction_name);

	ApiErrorCode WaitForTxnResponse(
		IN LpHandlePtr txn_handle,
		OUT IwMessagePtr &response,
		IN Time timout);

	ApiErrorCode WaitForTxnResponse(
		IN  const HandlesVector &map,
		OUT int &index,
		OUT IwMessagePtr &response,
		IN  Time timout);

	virtual AppData *GetAppData();
	virtual void SetAppData(AppData *data);

	BucketPtr _bucket;

protected:

	long _transactionTimeout;

	string _name;
	
	int _processId;

	BOOL _startSuspended;

	string _serviceId;

	LpHandlePair _pair;

	LpHandlePtr _inbound;

	LpHandlePtr _outbound;

	 AppData *_appData;

private:

	void Init(int UID, const string &owner_name);

	friend IW_CORE_API void RegisterContext(RunningContext* ctx);

	friend IW_CORE_API RunningContext* UnregisterContext();


};

#pragma endregion

class IW_CORE_API LightweightProcess: 
	public CSProcess,
	public RunningContext
	
{
public:

	LightweightProcess(
		IN LpHandlePair pair, 
		IN const string &owner_name = "",
		IN BOOL start_suspended = FALSE);


	virtual ~LightweightProcess(void);

	virtual void run();

	virtual void real_run() = 0;

};


class IW_CORE_API IProcFactory
{
public:
	virtual LightweightProcess *Create(LpHandlePair pair, ConfigurationPtr conf) = 0;

	virtual ~IProcFactory(){};
};

typedef IProcFactory *  (*IWPROC)(VOID); 

//
// owners map, used for logging mostly
//
IW_CORE_API RunningContext*
GetCurrRunningContext();

IW_CORE_API string 
GetCurrLpName();

IW_CORE_API int
GetCurrLpId();

typedef shared_ptr<IProcFactory> 
ProcFactoryPtr;

typedef list<ProcFactoryPtr> 
FactoryPtrList;


typedef list<LpHandlePair>
HandlePairList;

IW_CORE_API ApiErrorCode
LoadConfiguredModules(IN ConfigurationPtr conf,
					  OUT FactoryPtrList &factoriesList);
IW_CORE_API ApiErrorCode
BootModulesSimple(
				  IN ConfigurationPtr conf,
				  IN ScopedForking &forking,
				  IN const FactoryPtrList &factories_list,
				  OUT HandlePairList &proc_handlepairs,
				  OUT HandlesVector &selected_handles);

IW_CORE_API ApiErrorCode
ShutdownModules(IN HandlePairList &proc_handlepairs,
				IN ConfigurationPtr conf);

}




