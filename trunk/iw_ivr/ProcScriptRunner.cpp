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


			const MediaFormatsPtrList &codecs_list = _conf.CodecList();
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
			IwScript script(_conf,vm,call_session);
			

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



	IwScript::IwScript(
		IN Configuration &conf,
		IN CLuaVirtualMachine &vm, 
		IN CallWithDirectRtp &call_session)
		:CLuaScript(vm),
		_callSession(call_session),
		_vmPtr(vm),
		_confTable(vm),
		_lineInTable(vm),
		_conf(conf)
	{

		if (vm.Ok() == false)
		{
			LogCrit("Detected uninitialized vm");
			throw;
		}

		
		_confTable.Create("conf");
		_confTable.AddParam("sounds_dir",_conf.SoundsPath());

		_lineInTable.Create("linein");
		_lineInTable.AddParam("ani",_callSession.Ani());
		_lineInTable.AddParam("dnis",_callSession.Dnis());

	
		// !!! The order should be preserved for later switch statement !!!
		_methodBase = RegisterFunction("answer");
		RegisterFunction("hangup");
		RegisterFunction("wait");
		RegisterFunction("play");
		RegisterFunction("wait_for_dtmf");
		RegisterFunction("send_dtmf");
		RegisterFunction("blind_xfer");

	}


	IwScript::~IwScript()
	{

	}

	int 
	IwScript::ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) 
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
		case 5:
			{
				return LuaSendDtmf(vm);
			}
		case 6:
			{
				return LuaBlindXfer(vm);
			}

		}

		return 0;

	}


	void 
	IwScript::HandleReturns (CLuaVirtualMachine& vm, const char *strFunc)
	{

	}

	int
	IwScript::LuaAnswerCall(CLuaVirtualMachine& vm)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		LogDebug("Answer call iw stack handle=[" << _callSession.StackCallHandle() << "].");

		lua_State *state = (lua_State *) vm;

		ApiErrorCode res = _callSession.AcceptInitialOffer();
		lua_pushnumber (state, res);

		return 1;
	}

	int
	IwScript::LuaHangupCall(CLuaVirtualMachine& vm)
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

		LogDebug("Wait for " << time_to_sleep << " ms. iw stack handle=[" << _callSession.StackCallHandle() << "].");

		csp::SleepFor(MilliSeconds(time_to_sleep));
		return 0;

	}

	int
	IwScript::LuaPlay(CLuaVirtualMachine& vm)
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
	IwScript::LuaWaitForDtmf(CLuaVirtualMachine& vm)
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
	IwScript::LuaSendDtmf(CLuaVirtualMachine& vm)
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
	IwScript::LuaBlindXfer(CLuaVirtualMachine& vm)
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

}

