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
#include "CallWithRTPManagment.h"
#include "SipStackFactory.h"



#pragma TODO("Leave only ctor that takes configuration as a ref")
ProcVcs::ProcVcs(IN LpHandlePair pair, IN CnxInfo sip_stack_media)
:LightweightProcess(pair,VCS_Q,	__FUNCTIONW__),
_sipStackData(sip_stack_media),
_conf(NULL)
{
}

ProcVcs::ProcVcs(IN LpHandlePair pair, IN CcuConfiguration *conf)
:LightweightProcess(pair,VCS_Q,	__FUNCTIONW__),
_conf(conf),
_sipStackData(conf->VcsCnxInfo())
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
	_stackPair = stack_pair;
	FORK(SipStackFactory::CreateSipStack(stack_pair,*_conf));
	if (CCU_FAILURE(WaitTillReady(Seconds(5),stack_pair)))
	{
		Shutdown(Seconds(5), ipc_pair);
		return;
	};


	I_AM_READY;

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
			LogInfo(">>VCS Keep Alive<<");
			continue;
		}

		if (index == stack_index)
		{
			shutdown_flag = ProcessStackMessage(event, forking);
		} 
		else if (index == inbound_index)
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
ProcVcs::ProcessStackMessage(IN CcuMsgPtr ptr, IN ScopedForking &forking)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case CCU_MSG_CALL_OFFERED:
		{
			
			shared_ptr<CcuMsgCallOfferedReq> call_offered_msg = 
				dynamic_pointer_cast<CcuMsgCallOfferedReq>(ptr);

			DECLARE_NAMED_HANDLE_PAIR(script_pair);
			FORK_IN_THIS_THREAD(
				new ProcVoidFuncRunner<ProcVcs>(
				script_pair,
				bind<void>(&ProcVcs::StartScript, _1, ptr),
				this,
				L"Script Runner"));

			break;

		}
	case CCU_MSG_PROC_SHUTDOWN_REQ:
		{
			return TRUE;
			break;
		}
	default:
		{

		}
	}

	return FALSE;

}

void ProcVcs::StartScript(IN CcuMsgPtr msg)
{
	try
	{
		shared_ptr<CcuMsgCallOfferedReq> start_script_msg  = 
			dynamic_pointer_cast<CcuMsgCallOfferedReq>(msg);

		CallWithRTPManagment call_session(
			_stackPair,
			start_script_msg->stack_call_handle,
			start_script_msg->remote_media,
			*this);

		CLuaVirtualMachine vm;
		vm.InitialiseVM();

		CallFlowScript script(vm,call_session);

		if (!script.CompileFile(WStringToString(_conf->ScriptFile()).c_str()))
		{
			LogWarn(">>Error Compiling/Running<< script=[" << _conf->ScriptFile() << "]");
			SendResponse(msg, new CcuMsgCallOfferedNack());
			return;
		}

		LogDebug("script=[" << _conf->ScriptFile() << "], ix call handle=[" << start_script_msg->stack_call_handle <<"]");
	}
	catch (std::exception e)
	{
		LogWarn("Exception while running script=[ " << _conf->ScriptFile() <<"] e=[" << e.what() << "]");
	}

	
}

CallFlowScript::CallFlowScript(
							   IN CLuaVirtualMachine &vm_ptr, 
							   IN CallWithRTPManagment &call_session)
:CLuaScript(vm_ptr),
_callSession(call_session),
_vmPtr(vm_ptr)
{
	
	// !!! The order should be preserved for later switch statement !!!
	_methodBase = RegisterFunction("answer");
	 RegisterFunction("hangup");
	 RegisterFunction("wait");

}

CallFlowScript::~CallFlowScript()
{

}

int 
CallFlowScript::ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) 
{
	switch (iFunctionNumber - _methodBase)
	{
	case 0:
		{
			return LuaAnswerCall(vm);
		}
	case 1:
		{
			return LuaHangupCall(vm);
		}
	case 2:
		{
			return LuaWait(vm);
		}
		
	}

	return 0;

}


void 
CallFlowScript::HandleReturns (CLuaVirtualMachine& vm, const char *strFunc)
{

}

int
CallFlowScript::LuaAnswerCall(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;

	CcuApiErrorCode res = _callSession.AcceptCall();

	if (CCU_SUCCESS(res))
	{
		return 0;
	} else 
	{
		return -1;
	}

}

int
CallFlowScript::LuaHangupCall(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;

	CcuApiErrorCode res = _callSession.HagupCall();

	if (CCU_SUCCESS(res))
	{
		return 0;
	} else 
	{
		return -1;
	}

}

int
CallFlowScript::LuaWait(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;

	lua_State *state = (lua_State *) vm;
	long time_to_sleep  = (long) lua_tonumber (state, -1);

	csp::SleepFor(MilliSeconds(time_to_sleep));
	return 0;
	

}


