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
#include "ProcScriptRunner.h"

namespace ivrworx
{
	ProcScriptRunner::ProcScriptRunner(IN Configuration &conf,
		IN shared_ptr<MsgCallOfferedReq> msg, 
		IN LpHandlePair stack_pair, 
		IN LpHandlePair pair)
		:LightweightProcess(pair,__FUNCTION__),
		_conf(conf),
		_initialMsg(msg),
		_stackPair(stack_pair)
	{
		FUNCTRACKER;
	}

	ProcScriptRunner::~ProcScriptRunner()
	{

	}


	void 
	ProcScriptRunner::real_run()
	{
#pragma TODO ("Initializing vm and compiling script file is a terrible bottleneck")
		try
		{
			
			
			_stackHandle = _initialMsg->stack_call_handle;

			START_FORKING_REGION;

			CallWithDirectRtp call_session(
				_stackPair,
				forking,
				_initialMsg);

			// the script should be terminated if caller hanged up
			call_session.SetEventListener(MSG_CALL_HANG_UP_EVT,_inbound);


			const CodecsPtrList &codecs_list = _conf.CodecList();
			for (CodecsPtrList::const_iterator iter = codecs_list.begin(); iter != codecs_list.end(); iter++)
			{
				call_session.EnableMediaFormat(**iter);
			}

			CLuaVirtualMachine vm;
			IX_PROFILE_CODE(vm.InitialiseVM());

			IxScript script(vm,call_session);

			bool res = false;
			IX_PROFILE_CODE(res = script.CompileFile(_conf.ScriptFile().c_str()));
			if (res == false)
			{
				LogWarn("Error Compiling/Running script=[" << _conf.ScriptFile() << "]");
				return;
			}

			LogDebug("Script=[" << _conf.ScriptFile() << "], ix call handle=[" << _initialMsg->stack_call_handle <<"] completed successfully.");

			END_FORKING_REGION
		}
		catch (std::exception e)
		{
			LogWarn("Exception while running script=[ " << _conf.ScriptFile() <<"] e=[" << e.what() << "].");
		}

	}

	BOOL 
	ProcScriptRunner::HandleOOBMessage(IN IwMessagePtr msg)
	{
		// pings
		if (TRUE == LightweightProcess::HandleOOBMessage(msg))
		{
			return TRUE;
		}
		switch (msg->message_id)
		{
		case MSG_CALL_HANG_UP_EVT:
			{

				LogDebug("Call running script=[" << _conf.ScriptFile() << "], ix call handle=[" << _stackHandle <<"] received hangup event.");
				return FALSE;

				break;

			}
		default:
			{
				return FALSE;
			}

		}

	}



	IxScript::IxScript(IN CLuaVirtualMachine &vm_ptr, 
		IN CallWithDirectRtp &call_session)
		:CLuaScript(vm_ptr),
		_callSession(call_session),
		_vmPtr(vm_ptr)
	{

		// !!! The order should be preserved for later switch statement !!!
		_methodBase = RegisterFunction("answer");
		RegisterFunction("hangup");
		RegisterFunction("wait");
		RegisterFunction("play");
		RegisterFunction("wait_for_dtmf");

	}


	IxScript::~IxScript()
	{

	}

	int 
	IxScript::ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) 
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
		case 3:
			{
				return LuaPlay(vm);
			}
		case 4:
			{
				return LuaWaitForDtmf(vm);
			}

		}

		return 0;

	}


	void 
	IxScript::HandleReturns (CLuaVirtualMachine& vm, const char *strFunc)
	{

	}

	int
	IxScript::LuaAnswerCall(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		ApiErrorCode res = _callSession.AcceptCall();

		if (IW_SUCCESS(res))
		{
			return 0;
		} else 
		{
			return -1;
		}

	}

	int
	IxScript::LuaHangupCall(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		ApiErrorCode res = _callSession.HagupCall();

		if (IW_SUCCESS(res))
		{
			return 0;
		} else 
		{
			return -1;
		}

	}

	int
	IxScript::LuaWait(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;

		lua_State *state = (lua_State *) vm;
		long time_to_sleep  = (long) lua_tonumber (state, -1);

		LogDebug("Sleep for " << time_to_sleep << " ms. ix stack handle=[" << _callSession.StackCallHandle() << "].");

		csp::SleepFor(MilliSeconds(time_to_sleep));
		return 0;

	}

	int
	IxScript::LuaPlay(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;
		

		lua_State *state = (lua_State *) vm;
		size_t string_length = 0;
		const char *file_to_play = lua_tolstring(state, -1, &string_length);

		LogDebug("Play file [" << file_to_play << "]  ix stack handle=[" << _callSession.StackCallHandle() << "].");

		ApiErrorCode res = _callSession.PlayFile(file_to_play);

		if (IW_SUCCESS(res))
		{
			return 0;
		} else 
		{
			return -1;
		}

		return 0;

	}

	int
	IxScript::LuaWaitForDtmf(CLuaVirtualMachine& vm)
	{
		lua_State *state = (lua_State *) vm;
		long time_to_sleep  = (long) lua_tonumber (state, -1);

		LogDebug("Wait for dtmf  for " << time_to_sleep << " ms. ix stack handle=[" << _callSession.StackCallHandle() << "].");

		int dtmf = -1;
		ApiErrorCode res = _callSession.WaitForDtmf(dtmf, MilliSeconds(time_to_sleep));

		if (IW_SUCCESS(res) || res == API_TIMEOUT)
		{
			return 0;
		} else 
		{
			return -1;
		}

		return 0;

	}

}

