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
		:LightweightProcess(pair,"IvrScript"),
		_conf(conf),
		_initialMsg(msg),
		_stackPair(stack_pair)
	{
		FUNCTRACKER;
	}

	ProcScriptRunner::ProcScriptRunner(
		IN Configuration &conf,
		IN LpHandlePair stack_pair, 
		IN LpHandlePair pair)
		:LightweightProcess(pair,"IvrScript"),
		_conf(conf),
		_stackPair(stack_pair)
	{
		FUNCTRACKER;
	}

	ProcScriptRunner::~ProcScriptRunner()
	{

	}

	void 
	ProcScriptRunner::RunStandAlone()
	{
		FUNCTRACKER;

		const string &script_name = _conf.SuperScript();

		try
		{
			START_FORKING_REGION;

			CLuaVirtualMachine vm;
			IX_PROFILE_CODE(vm.InitialiseVM());

			if (vm.Ok() == false)
			{
				LogCrit("Couldn't initialize lua vm");
				throw;
			}

			CLuaDebugger debugger(vm);

			// compile the script if needed
			IwScript script(_conf,vm);

			bool res = false;
			IX_PROFILE_CODE(res = script.CompileFile(script_name.c_str()));
			if (res == false)
			{
				LogWarn("Error compiling/running super script:" << script_name);
				return;
			}

			END_FORKING_REGION

		}
		catch (std::exception e)
		{
			LogWarn("Exception while running script:" << _conf.ScriptFile() <<", e:" << e.what() << ", iwh:" << _initialMsg->stack_call_handle);
		}

		LogDebug("Super script:" << script_name << " completed successfully.");

	}

	void 
	ProcScriptRunner::RunIncomingCallHandler()
	{
		FUNCTRACKER;

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


			const MediaFormatsPtrList &codecs_list = _conf.MediaFormats();
			for (MediaFormatsPtrList::const_iterator iter = codecs_list.begin(); iter != codecs_list.end(); iter++)
			{
				call_session.EnableMediaFormat(**iter);
			}

			CLuaVirtualMachine vm;
			IX_PROFILE_CODE(vm.InitialiseVM());

			if (vm.Ok() == false)
			{
				LogCrit("Couldn't initialize lua vm");
				throw;
			}

			CLuaDebugger debugger(vm);

			// compile the script if needed
			IwCallHandlerScript script(_conf,vm,call_session);

			try 
			{
				bool res = false;
				IX_PROFILE_CODE(res = script.CompileFile(_conf.ScriptFile().c_str()));
				if (res == false)
				{
					LogWarn("Error compiling/running script:" << _conf.ScriptFile() << " ,iwh:" << _initialMsg->stack_call_handle);
					return;
				}

			}
			catch (script_hangup_exception)
			{
				script.RunOnHangupScript();
			}

			END_FORKING_REGION
		}
		catch (std::exception e)
		{
			LogWarn("Exception while running script:" << _conf.ScriptFile() <<", e:" << e.what() << ", iwh:" << _initialMsg->stack_call_handle);
		}

		LogDebug("script:" << _conf.ScriptFile() << ", iwh:" << _initialMsg->stack_call_handle <<" completed successfully.");

	}


	void 
	ProcScriptRunner::real_run()
	{
		
		if (_initialMsg)
		{
			RunIncomingCallHandler();
		}
		else
		{
			RunStandAlone();
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

				LogDebug("Call running scripts:" << _conf.ScriptFile() << "], iwh:" << _stackHandle <<" received hangup event. The script will be terminated.");
				throw script_hangup_exception();

			}
		default:
			{
				return FALSE;
			}

		}

	}

#pragma region IwScript

	IwScript::IwScript(
		IN Configuration &conf,
		IN CLuaVirtualMachine &vm)
		:CLuaScript(vm),
		_vmPtr(vm),
		_confTable(vm),
		_lanesTable(vm),
		_conf(conf)
	{
		FUNCTRACKER;

		if (vm.Ok() == false)
		{
			LogCrit("Detected uninitialized vm");
			throw;
		}

		_confTable.Create("conf");
		_confTable.AddParam("sounds_dir",_conf.SoundsPath());


		// !!! The order should be preserved for later switch statement !!!
		_methodBase = RegisterFunction("iw_log");
		RegisterFunction("wait");
		RegisterFunction("run");

	}

	IwScript::~IwScript()
	{

	}

	void 
	IwScript::HandleReturns (CLuaVirtualMachine& vm, const char *strFunc)
	{

	}


	int 
	IwScript::ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) 
	{
		switch (iFunctionNumber - _methodBase)
		{
		case 0:
			{
				return LuaLog(vm);
			}
		case 1:
			{
				return LuaWait(vm);
			}
		case 2:
			{
				return LuaRun(vm);
			}
		default:
			{
				LogWarn ("Unknown function called id:" << iFunctionNumber);
				return -1;
			}

		}

		return 0;
	}

	int
	IwScript::LuaRun(CLuaVirtualMachine& vm)
	{
		lua_State *state = (lua_State *) vm;

		if (lua_isfunction(state, -1) == 0 )
		{
			LogWarn("Wrong type of parameter for Run");
			return 0;
		}
		
		DECLARE_NAMED_HANDLE_PAIR(runner_pair);
		csp::Run(new ProcBlockingOperationRunner(runner_pair,vm));

		return 0;

	}

	ProcBlockingOperationRunner::ProcBlockingOperationRunner(LpHandlePair pair, CLuaVirtualMachine& vm)
		:LightweightProcess(pair,"ProcBlockingOperationRunner"),
		_vm(vm)
	{
		FUNCTRACKER;

	}

	void
	ProcBlockingOperationRunner::real_run()
	{
		FUNCTRACKER;

		lua_State *state = (lua_State *) _vm;
		bool res = _vm.CallFunction(0);

		if (res == false)
		{
			LogWarn("Error running long operation");
			return;
		}

		lua_pushnumber (state, API_SUCCESS);
	}

	int
	IwScript::LuaWait(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;

		lua_State *state = (lua_State *) vm;

		if (lua_isnumber (state, -1) != 1 )
		{
			LogWarn("Wrong type of parameter for wait");
			return 0;
		}

		long time_to_sleep  = (long) lua_tonumber (state, -1);

		LogDebug("Wait for " << time_to_sleep);

		csp::SleepFor(MilliSeconds(time_to_sleep));
		return 0;

	}

	int
	IwScript::LuaLog(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		lua_State *state = (lua_State *) vm;

		if (lua_isnumber(state, -2) != 1 )
		{
			LogWarn("Wrong type of parameter for log - loglevel");
			return 0;
		}

		if (lua_isstring(state, -1) != 1 )
		{
			LogWarn("Wrong type of parameter for log - logstring");
			return 0;
		}

		size_t string_length = 0;
		LogLevel log_level = (LogLevel)((long)lua_tonumber(state,  -2));
		const char *log_string = lua_tolstring(state, -1, &string_length);

		switch(log_level)
		{
		case LOG_LEVEL_OFF:
			{
				break;
			}
		case LOG_LEVEL_CRITICAL:
			{
				LogCrit(log_string);
				break;
			}
		case LOG_LEVEL_WARN:
			{
				LogWarn(log_string);
				break;
			}
		case LOG_LEVEL_INFO:
			{
				LogInfo(log_string);
				break;
			}
		case LOG_LEVEL_DEBUG:
			{
				LogDebug(log_string);
				break;
			}
		case LOG_LEVEL_TRACE:
			{
				LogTrace(log_string);
				break;
			}
		default:
			{
				LogDebug(log_string);
			}
		}


		lua_pushnumber (state, API_SUCCESS);
		return 1;

	}

#pragma endregion IwScript

	IwCallHandlerScript::IwCallHandlerScript(
		IN Configuration &conf,
		IN CLuaVirtualMachine &vm, 
		IN CallWithDirectRtp &call_session)
		:IwScript(conf, vm),
		_callSession(call_session),
		_lineInTable(vm)
	{
		FUNCTRACKER;

		if (vm.Ok() == false)
		{
			LogCrit("Detected uninitialized vm");
			throw;
		}

		_lineInTable.Create("linein");
		_lineInTable.AddParam("ani",_callSession.Ani());
		_lineInTable.AddParam("dnis",_callSession.Dnis());

		// !!! The order should be preserved for later switch statement !!!
		_methodBase = RegisterFunction("answer");
		RegisterFunction("hangup");
		RegisterFunction("play");
		RegisterFunction("wait_for_dtmf");
		RegisterFunction("send_dtmf");
		RegisterFunction("blind_xfer");
		RegisterFunction("wait_till_hangup");

	}

	int 
	IwCallHandlerScript::ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) 
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
				return LuaPlay(vm);
			}
		case 3:
			{
				return LuaWaitForDtmf(vm);
			}
		case 4:
			{
				return LuaSendDtmf(vm);
			}
		case 5:
			{
				return LuaBlindXfer(vm);
			}
		case 6:
			{
				return LuaWaitTillHangup(vm);
			}
		default:
			{
				
				return IwScript::ScriptCalling(vm,iFunctionNumber);
			}

		}

		return 0;

	}

	void 
	IwCallHandlerScript::RunOnHangupScript()
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		LogDebug("Running hangup script call iwh:" << _callSession.StackCallHandle());

		lua_State *state = (lua_State *) _vmPtr;

		lua_getfield(state, LUA_GLOBALSINDEX, "on_hangup");

		// hangup function exists?
		if (lua_isnil(state,-1) == 1)
		{
			LogDebug("Hangup script was not defined for call iwh:" << _callSession.StackCallHandle());
			return;
		}

		lua_call(state,0,0);

	}


	int
	IwCallHandlerScript::LuaAnswerCall(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		LogDebug("Answer call iwh:" << _callSession.StackCallHandle());

		lua_State *state = (lua_State *) vm;

		ApiErrorCode res = _callSession.AcceptInitialOffer();
		lua_pushnumber (state, res);

		return 1;
	}

	int
	IwCallHandlerScript::LuaHangupCall(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		LogDebug("Hangup call iw stack handle=[" << _callSession.StackCallHandle() << "].");

		lua_State *state = (lua_State *) vm;

		ApiErrorCode res = _callSession.HagupCall();
		lua_pushnumber (state, res);

		return 1;

	}

	
	int
	IwCallHandlerScript::LuaPlay(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;
		

		lua_State *state = (lua_State *) vm;

		if (lua_isstring(state, -3) != 1 )
		{
			LogWarn("Wrong type of parameter for play - filename");
			return 0;
		}

		if (lua_isboolean(state, -2) != 1 )
		{
			LogWarn("Wrong type of parameter for play - sync");
			return 0;
		}


		if (lua_isboolean(state, -1) != 1 )
		{
			LogWarn("Wrong type of parameter for play - loop");
			return 0;
		}


		size_t string_length = 0;
		const char *file_to_play = lua_tolstring(state, -3, &string_length);
		BOOL sync				 = lua_toboolean(state, -2);
		BOOL loop				 = lua_toboolean(state, -1);

		LogDebug("Play file [" << file_to_play << "]  iw stack handle=[" << _callSession.StackCallHandle() << "].");

		ApiErrorCode res = _callSession.PlayFile(file_to_play,sync,loop);
		lua_pushnumber (state, res);

		return 1;

	}

	int
	IwCallHandlerScript::LuaWaitForDtmf(CLuaVirtualMachine& vm)
	{
		lua_State *state = (lua_State *) vm;
		long time_to_sleep  = (long) lua_tonumber (state, -1);

		LogDebug("Wait for dtmf  for " << time_to_sleep << " ms. iw stack handle=[" << _callSession.StackCallHandle() << "].");

		int dtmf = -1;
		ApiErrorCode res = _callSession.WaitForDtmf(dtmf, MilliSeconds(time_to_sleep));

		lua_pushnumber (state, res);
		if (IW_SUCCESS(res))
		{
			lua_pushnumber (state, dtmf);
		} else
		{
			lua_pushnil(state);
		}
		
		return 2;

	}

	int
	IwCallHandlerScript::LuaSendDtmf(CLuaVirtualMachine& vm)
	{
		lua_State *state = (lua_State *) vm;

		if (lua_isstring(state, -1) == 0 )
		{
			LogWarn("Wrong type of parameter for send dtmf");
			return 0;
		}


		const char* dtmf = lua_tostring(state, -1);

		int curr_index = 0;
		while (dtmf[curr_index] != '\0')
		{
			ApiErrorCode res = _callSession.SendRfc2833Dtmf(dtmf[curr_index++]);
			if (IW_FAILURE(res))
			{
				lua_pushnumber (state, res);
				return 1;
			}
			csp::SleepFor(MilliSeconds(200));
		}

		
		lua_pushnumber (state, API_SUCCESS);
		return 1;


	}

	int
	IwCallHandlerScript::LuaBlindXfer(CLuaVirtualMachine& vm)
	{
		lua_State *state = (lua_State *) vm;

		if (lua_isstring(state, -1) == 0 )
		{
			LogWarn("Wrong type of parameter for blind xfer");
			return 0;
		}


		const char* destination = lua_tostring(state, -1);

		_callSession.BlindXfer(destination);


		lua_pushnumber (state, API_SUCCESS);
		return 1;


	}

	
	int
	IwCallHandlerScript::LuaWaitTillHangup(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		LogDebug("Wait till hang up iwh:" << _callSession.StackCallHandle());

		lua_State *state = (lua_State *) vm;


		_callSession.WaitTillHangup();

		lua_pushnumber (state, API_SUCCESS);
		return 1;

	}

	

}

