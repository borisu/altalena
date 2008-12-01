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
#include "ProcVcs.h"
#include "ProcPipeIPCDispatcher.h"
#include "ProcIncomingCallHandler.h"
#include "CallWithRTPManagment.h"
#include "SipStackFactory.h"


ProcVcs::ProcVcs(IN LpHandlePair pair, IN CnxInfo sip_stack_media)
:LightweightProcess(pair,VCS_Q,	__FUNCTIONW__),
_sipStackData(sip_stack_media),
_conf(NULL)
{
}

ProcVcs::ProcVcs(IN LpHandlePair pair, IN CcuConfiguration &conf)
:LightweightProcess(pair,VCS_Q,	__FUNCTIONW__),
_conf(&conf),
_sipStackData(conf.VcsCnxInfo())
{
}



ProcVcs::~ProcVcs(void)
{
}

void
ProcVcs::real_run()
{

	START_FORKING_REGION;
	
	//
	// Start IPC for VCS queue
	//
	DECLARE_NAMED_HANDLE_PAIR(ipc_pair);
	FORK(new ProcPipeIPCDispatcher(ipc_pair,VCS_Q));
	if (CCU_FAILURE(WaitTillReady(Seconds(5),ipc_pair)))
	{
		return;
	};

	
	//
	// Start SIP stack process
	//
	DECLARE_NAMED_HANDLE_PAIR(stack_pair);
	FORK(SipStackFactory::CreateSipStack(
		stack_pair, 
		_sipStackData,
		ICallHandlerCreatorPtr(new VcsCallHandlerCreator(_inbound, stack_pair))));

	if (CCU_FAILURE(WaitTillReady(Seconds(5),stack_pair)))
	{
		Shutdown(Seconds(5), ipc_pair);
		return;
	};

	I_AM_READY;

	//
	// Login Agents To Campaign
	//

	AgentsList agents;
	_conf->Agents(agents);

	
	ProcessList proclist;
	// confirm login agents in parallel
	for (AgentsList::iterator iter = agents.begin(); iter!=agents.end(); iter++)
	{
		// dummy variable
		CcuApiErrorCode res = CCU_API_SUCCESS;
		
		// the syntax from hell
		ProcFuncRunner<CcuApiErrorCode,ProcVcs> *agent_login_proc 
			= new  ProcFuncRunner<CcuApiErrorCode,ProcVcs>(		
					bind<CcuApiErrorCode>(&ProcVcs::InitialLogin, _1, *iter, stack_pair),
					this,
					res,
					__FUNCTIONW__);

		proclist.push_back(agent_login_proc);

	}

	RunInThisThread(InParallelOneThread(proclist.begin(),proclist.end()));


	HandlesList list;
	int index = 0;
	
	list.push_back(ipc_pair.outbound);
	const int ipc_index = index++;

	list.push_back(stack_pair.outbound);
	const int stack_index = index++;

	list.push_back(_inbound);
	const int inbound_index = index++;

	
	//
	// Message Loop
	// 
	BOOL shutdown_flag = FALSE;
	CcuMsgPtr event;
	while(shutdown_flag == FALSE)
	{
		
		int index = -1;
		CcuApiErrorCode err_code = 
			SelectFromChannels(
			list,
			Seconds(60), 
			index, 
			event);

		if (err_code == CCU_API_TIMEOUT)
		{
			LogInfo(">>Keep Alive<<");
			continue;
		}

		if (index == stack_index)
		{
			shutdown_flag = ProcessStackMessage(event);

		} else if (index == inbound_index)
		{
			
			shutdown_flag = ProcessInboundMessage(event,forking);
			

		}

	}

	Shutdown(Time(Seconds(5)),stack_pair);
	Shutdown(Time(Seconds(5)),ipc_pair);

	if (event != CCU_NULL_MSG && 
		event->message_id == CCU_MSG_PROC_SHUTDOWN_REQ)
	{

		SendResponse(event,new CcuMsgShutdownAck());
	}

	END_FORKING_REGION;

}

BOOL 
ProcVcs::ProcessInboundMessage(IN CcuMsgPtr event, IN ScopedForking &forking)
{
	FUNCTRACKER;
	switch (event->message_id)
	{
	case CCU_MSG_PROC_SHUTDOWN_REQ:
		{
			return TRUE;
		}
	case CCU_MSG_START_CALL_HANDLER:
		{
			shared_ptr<CcuMsgStartHandlerProc> msg = 
				dynamic_pointer_cast<CcuMsgStartHandlerProc>(event);

			FORK_IN_THIS_THREAD(msg->h);

			break;

		}
	default:
		{
			BOOL oob_res = HandleOOBMessage(event);
			if (oob_res = FALSE)
			{
				LogWarn("Unknown message received id=[" << event->message_id_str << "]");
				_ASSERT(false);
			}
			
		}
	}

	return FALSE;

}

BOOL 
ProcVcs::ProcessStackMessage(CcuMsgPtr ptr)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case CCU_MSG_PROC_SHUTDOWN_REQ:
		{
			
			return TRUE;
		}

	default:
		{

		}
	}

	return FALSE;

}

CcuApiErrorCode
ProcVcs::InitialLogin(Agent agent, LpHandlePair stack_pair)
{
	FUNCTRACKER;

	CallWithRTPManagment call(stack_pair, *this);


	CcuApiErrorCode res = call.MakeCall(agent.media_address);
	if (CCU_FAILURE(res))
	{
		LogWarn("Error making call to dest=[" << agent.media_address << "] res=[" << res << "]");
		return res;
	}

	res = call.PlayFile(L".\\sounds\\welcome.wav");	
	if (CCU_FAILURE(res))
	{
		LogWarn("Error playing welcome res=[" << res << "]");
		return res;
	}

	return CCU_API_TIMEOUT;
}



VcsCallHandlerCreator::VcsCallHandlerCreator(
	IN LpHandlePtr main_proc_handle,
	IN LpHandlePair stack_pair):
_mainProcHandle(main_proc_handle),
_stackPair(stack_pair)
{

}



LpHandlePair  
VcsCallHandlerCreator::CreateCallHandler(
	IN LpHandlePair stack_pair, 
	IN int stack_handle,
	IN CnxInfo offered_media)
{
	
	DECLARE_NAMED_HANDLE_PAIR(new_handler_pair);

	ProcIncomingCallHandler *h = 
		new ProcIncomingCallHandler(new_handler_pair, _stackPair);



	CcuMsgStartHandlerProc *msg = new CcuMsgStartHandlerProc();
	msg->h = h;

	_mainProcHandle->Send(msg);

	return new_handler_pair;


}


