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

#define I_AM_READY SendReadyMessage();

#define	SEND_RESPONSE(P, R) GetCurrLightWeightProc()->SendResponse(P, R);

typedef list<CSProcess*> ProcessList;


typedef
shared_ptr<Bucket> BucketPtr;

class LightweightProcess: 
	public CSProcess
{
public:

	LightweightProcess(
		IN LpHandlePair pair, 
		IN const string &owner_name = "",
		IN BOOL start_suspended = FALSE);

	
	LightweightProcess(
		IN LpHandlePair pair, 
		IN int inbound_channel_id, 
		IN const string &owner_name = "",
		IN BOOL start_suspended = FALSE);


	virtual ~LightweightProcess(void);

	static void Join(BucketPtr bucket);

	virtual void run();

	virtual void real_run() = 0;

	virtual string Name();

	virtual int ProcessId();

	virtual void Name(IN const string &val);

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

	LpHandlePair _pair;

	LpHandlePtr _inbound;

	LpHandlePtr _outbound;

	BucketPtr _bucket;

protected:

	long _transactionTimeout;

	string _name;
	
	int _processId;

	int _processAlias;

	BOOL _startSuspended;

private:

	void Init(int UID, const string &owner_name);

};

#pragma endregion

class IProcFactory
{
public:
	virtual LightweightProcess *Create(LpHandlePair pair, Configuration &conf) = 0;

	virtual ~IProcFactory(){};
};

//
// owners map, used for logging mostly
//
LightweightProcess*
GetCurrLightWeightProc();

string 
GetCurrLpName();

int
GetCurrLpId();

}




