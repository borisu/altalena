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

#include "Ccu.h"
#include "LightweightProcess.h"
#include "CcuCommon.h"
#include "Ais.h"

struct QueueSlot
{
	QueueSlot();

	QueueSlot(const QueueSlot &other);

	CcuMsgPtr orig_request;
};

typedef 
vector<AgentInfo> AgentInfoVector;

typedef 
list<QueueSlot> WaitingList;

#pragma TODO("Consider taking ProcCcuFacade as a parameter to use its interrupter")

class AisSession
{
public:

	AisSession(LightweightProcess &process);

	CcuApiErrorCode Login(IN const AgentInfo &info);

	CcuApiErrorCode Logout(IN const wstring &agent_name);

	CcuApiErrorCode AllocateAgent(OUT AgentInfo &info, IN const Time &timeout);

	CcuApiErrorCode FreeAgent(IN const wstring &info);

private:

	LightweightProcess &_process;

};

class ProcAis :
	public LightweightProcess
{
public:

	ProcAis(LpHandlePair pair);

	virtual ~ProcAis(void);

protected:

	void UponAgentLoginReq(CcuMsgPtr ptr);

	void UponAgentLogoutReq(CcuMsgPtr ptr);

	void UponAllocateAgentReq(CcuMsgPtr ptr);

	void UponAgentFreeReq(CcuMsgPtr ptr);
	
	void UponAgentQuitQueueReq(CcuMsgPtr ptr);

	void TryToMatch();

	void real_run();

	AgentInfoVector _agentInfos;

	WaitingList _waitingList;
	
};
