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
#include "ProcLuaScriptRunner.h"
#include "ScriptSession.h"
#include "CcuLogger.h"
#include "LuaScript.h"
#include "Call.h"

ProcLuaScriptRunner::ProcLuaScriptRunner(IN LpHandlePair pair, IN LpHandlePair stack_handle)
:LightweightProcess(pair,__FUNCTIONW__),
_stackHandle(stack_handle)
{
}

ProcLuaScriptRunner::~ProcLuaScriptRunner(void)
{
}

void
ProcLuaScriptRunner::real_run()
{
	_vm.InitialiseVM ();

	START_FORKING_REGION;
	I_AM_READY;

	BOOL shutdownFlag = FALSE;
	while (shutdownFlag  == FALSE)
	{
		CcuApiErrorCode err_code = CCU_API_SUCCESS;
		CcuMsgPtr ptr =  _inbound->Wait(Seconds(60), err_code);

		if (err_code == CCU_API_TIMEOUT)
		{
			LogDebug("SCS >>Keep Alive<<");
			continue;
		}

		switch (ptr->message_id)
		{
		case CCU_MSG_START_SCRIPT:
			{
				
				DECLARE_NAMED_HANDLE_PAIR(script_pair);
				FORK_IN_THIS_THREAD(
					new ProcVoidFuncRunner<ProcLuaScriptRunner>(
						script_pair,
						bind<void>(&ProcLuaScriptRunner::StartScript, _1, ptr),
						this,
						L"Script Runner"));
				break;
			}
		
		case CCU_MSG_PROC_SHUTDOWN_REQ:
			{
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

				LogWarn(">>Unknown<< messsage to Scrpt Runner id=[" << ptr->message_id_str <<"]");
			}
		} // switch
	}// while

	END_FORKING_REGION;
}


void
ProcLuaScriptRunner::StartScript(CcuMsgPtr msg)
{
	shared_ptr<CcuMsgStartScript> start_script_msg  = 
		dynamic_pointer_cast<CcuMsgStartScript>(msg);

	CallWithRTPManagment call_session(
		_stackHandle,
		start_script_msg->incoming_call_handle,
		start_script_msg->offered_cnx_info,
		*this);

	CallFlowScript script( _vm, call_session);

	string s = WStringToString(start_script_msg->script_buffer);

	if (!script.CompileBuffer((unsigned char*)s.c_str(), s.length()))
	{
		SendResponse(msg, new CcuMsgNack());
	}

	script.Go();
}

CallFlowScript::CallFlowScript(IN CLuaVirtualMachine &vm, IN CallWithRTPManagment &call_session)
:CLuaScript(vm),
_callSession(call_session)
{
	_methodBase = RegisterFunction("answer");
	
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
		return AnswerCall(vm);
	}
	
	return 0;

}


void 
CallFlowScript::HandleReturns (CLuaVirtualMachine& vm, const char *strFunc)
{

}

int
CallFlowScript::AnswerCall(CLuaVirtualMachine& vm)
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


