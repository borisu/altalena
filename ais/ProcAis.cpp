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

#include "StdAfx.h"
#include "ProcAis.h"
#include "ProcPipeIPCDispatcher.h"
#include "CcuLogger.h"
#include "Ais.h"


QueueSlot::QueueSlot()
{
	
}

QueueSlot::QueueSlot(const QueueSlot &other)
{
	orig_request = other.orig_request;
}

CcuApiErrorCode
AisSession::Login(IN const AgentInfo &info)
{
	FUNCTRACKER;

	CcuMsgPtr response = CCU_NULL_MSG;

	CcuMsgAgentLogin *msg = new CcuMsgAgentLogin();
	msg->agent_info = info;


	EventsSet map;
	map.insert(CCU_MSG_AGENT_LOGIN_ACK);
	map.insert(CCU_MSG_AGENT_LOGIN_NACK);


	CcuApiErrorCode res = _process.DoRequestResponseTransaction(
		AIS_Q,
		CcuMsgPtr(msg),
		map,
		response,
		Time(MilliSeconds(_process.TransactionTimeout())),
		L"Agent Login TXN");

	if (res != CCU_API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_AGENT_LOGIN_ACK:
		{
			break;
		}
	case CCU_MSG_AGENT_LOGIN_NACK:
		{
			res = CCU_API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}

	return res;

}

CcuApiErrorCode
AisSession::Logout(IN const wstring &agent_name)
{
	FUNCTRACKER;

	CcuMsgPtr response = CCU_NULL_MSG;

	CcuMsgAgentLogout *msg = new CcuMsgAgentLogout();
	msg->agent_name = agent_name;


	// we are not interested in response
	EventsSet map;

	CcuApiErrorCode res = _process.DoRequestResponseTransaction(
		AIS_Q,
		CcuMsgPtr(msg),
		map,
		response,
		Time(MilliSeconds(_process.TransactionTimeout())),
		L"Agent Logout TXN");



	return res;

}

CcuApiErrorCode
AisSession::AllocateAgent(OUT AgentInfo &info, IN const Time &timeout)
{
	FUNCTRACKER;

	CcuMsgPtr response = CCU_NULL_MSG;

	CcuMsgAgentAllocateReq *msg = new CcuMsgAgentAllocateReq();
	msg->handle = CCU_UNDEFINED; // future use
	


	EventsSet map;
	map.insert(CCU_MSG_AGENT_ALLOCATE_ACK);
	map.insert(CCU_MSG_AGENT_ALLOCATE_NACK);


	CcuApiErrorCode res = _process.DoRequestResponseTransaction(
		AIS_Q,
		CcuMsgPtr(msg),
		map,
		response,
		Time(MilliSeconds(_process.TransactionTimeout())),
		L"Agent Allocate TXN");

	if (res != CCU_API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_AGENT_ALLOCATE_ACK:
		{
			shared_ptr<CcuMsgAgentAllocateAck> ack = 
				dynamic_pointer_cast<CcuMsgAgentAllocateAck> (response);

			info = ack->info;

			break;
		}
	case CCU_MSG_AGENT_LOGIN_NACK:
		{
			res = CCU_API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}

	return res;

}


ProcAis::ProcAis(LpHandlePair pair):
LightweightProcess(pair,AIS_Q,__FUNCTIONW__)
{
	FUNCTRACKER;
}

ProcAis::~ProcAis(void)
{
	FUNCTRACKER;
}


AisSession::AisSession(LightweightProcess &process):
_process(process)
{

}


void 
ProcAis::real_run()
{
	FUNCTRACKER;

	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(ipc_pair);

	//
	// Start IPC
	//
	FORK(new ProcPipeIPCDispatcher(ipc_pair,AIS_Q));
	if (CCU_FAILURE(WaitTillReady(Seconds(5), ipc_pair)))
	{
		LogCrit("Cannot start AIS IPC interface. Exiting...");
		throw;
	}

	I_AM_READY;


	BOOL shutdownFlag = FALSE;
	while (shutdownFlag  == FALSE)
	{
		CcuApiErrorCode err_code = CCU_API_SUCCESS;
		CcuMsgPtr ptr =  _inbound->Wait(Seconds(60), err_code);

		if (err_code == CCU_API_TIMEOUT)
		{
			LogDebug("AIS >>Keep Alive<<");
			continue;
		}

		switch (ptr->message_id)
		{
		case CCU_MSG_AGENT_LOGIN_REQ:
			{
				UponAgentLoginReq(ptr);
				break;
			}
		case CCU_MSG_AGENT_LOGOUT_REQ:
			{
				UponAgentLogoutReq(ptr);
				break;
			}
		case CCU_MSG_AGENT_ALLOCATE_REQ:
			{
				UponAllocateAgentReq(ptr);
				break;
			}
		case CCU_MSG_AGENT_FREE_REQ:
			{
				UponAgentFreeReq(ptr);
				break;
			}
		case CCU_MSG_AGENT_QUIT_QUEUE_REQ:
			{
				UponAgentQuitQueueReq(ptr);
				break;
			}
		case CCU_MSG_PROC_SHUTDOWN_REQ:
			{
				Shutdown(Seconds(5),ipc_pair);
				shutdownFlag  = TRUE;
				SendResponse(ptr, new CcuMsgShutdownAck());
				break;
			}
		default:
			{
				if (HandleOOBMessage(ptr))
				{
					continue;
				}

				LogWarn(">>Unknown<< messsage to AIS id=[" << ptr->message_id_str <<"]");
			}
		} // switch
	}// while

	END_FORKING_REGION;
}

void
ProcAis::UponAgentLoginReq(CcuMsgPtr ptr)
{
	FUNCTRACKER;

	shared_ptr<CcuMsgAgentLogin> login_msg = 
		dynamic_pointer_cast<CcuMsgAgentLogin> (ptr);

	if (login_msg->agent_info.name.empty())
	{
		LogWarn("Agent name is >>empty<<.");
		SendResponse(ptr, new CcuMsgAgentLoginNack());
		return;
	}

	// iterate over currently logged in agents
	// and see if its not already logged in
	for (AgentInfoVector::iterator iter = _agentInfos.begin(); 
		iter != _agentInfos.end();
		iter++)
	{
		if ( (*iter).name == login_msg->agent_info.name)
		{
			SendResponse(ptr, new CcuMsgAgentLoginNack());
			return;
		}
	}

	CcuMsgAgentLoginAck *ack = new CcuMsgAgentLoginAck();
	SendResponse(ptr,ack);

	login_msg->agent_info.state = CCU_AGENT_STATE_AVAILABLE;
	_agentInfos.push_back(login_msg->agent_info);

	TryToMatch();

}

void
ProcAis::UponAgentLogoutReq(CcuMsgPtr ptr)
{
	shared_ptr<CcuMsgAgentLogout> logout_msg = 
		dynamic_pointer_cast<CcuMsgAgentLogout> (ptr);

	if (logout_msg->agent_name.empty())
	{
		LogWarn("Agent name is >>empty<<.");
		SendResponse(ptr, new CcuMsgAgentLoginNack());
		return;
	}

	
	AgentInfoVector::iterator iter = _agentInfos.begin();
	for (;iter != _agentInfos.end();iter++)
	{
		if ( (*iter).name == logout_msg->agent_name)
		{
			break;
		}
	}

	if (iter == _agentInfos.end())
	{
		LogWarn("Logout for >>non logged in<< agent=[" << logout_msg->agent_name << "]");
		return;
	}

	_agentInfos.erase(iter);

}

void
ProcAis::UponAllocateAgentReq(CcuMsgPtr ptr)
{
	shared_ptr<CcuMsgAgentAllocateReq> agent_alloc_msg = 
		dynamic_pointer_cast<CcuMsgAgentAllocateReq> (ptr);

	
	// no immediately available agent found 
	// add it to the queue

	QueueSlot slot;

	slot.orig_request = agent_alloc_msg;
	_waitingList.push_back(slot);

	
	TryToMatch();

}

void
ProcAis::UponAgentFreeReq(CcuMsgPtr ptr)
{

}

void 
ProcAis::UponAgentQuitQueueReq(CcuMsgPtr ptr)
{

}

#pragma TODO("Employ more efficient matching data structures and algorithm.")

void
ProcAis::TryToMatch()
{
	FUNCTRACKER;

	if (_waitingList.empty())
	{
		return;
	}

	AgentInfoVector::iterator iter = _agentInfos.begin();
	for (;iter != _agentInfos.end();iter++)
	{
		if ( (*iter).state == CCU_AGENT_STATE_AVAILABLE)
		{
			break;
		}
	}

	if (iter == _agentInfos.end())
	{
		return;
	}

	AgentInfo &info = *iter;

	QueueSlot slot = _waitingList.front();
	_waitingList.pop_front();

	LogDebug(">>Matched<< agent=[" << info.name << "] for req=[" << slot.orig_request->source.queue_path << "]");

	CcuMsgAgentAllocateAck *ack = new CcuMsgAgentAllocateAck();
	ack->info = info;
	
	SendResponse(slot.orig_request, ack);

}


