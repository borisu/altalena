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
#include "IwScriptApi.h"


namespace ivrworx
{

IwScript::IwScript(IN ScopedForking &forking,
				   IN Configuration &conf,
				   IN CLuaVirtualMachine &vm)
				   :CLuaScript(vm),
				   _forking(forking),
				   _vmPtr(vm),
				   _confTable(vm),
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
	RegisterFunction("make_call");

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
	case 3:
		{
			return LuaMakeCall(vm);
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
IwScript::LuaMakeCall(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;
	IX_PROFILE_FUNCTION();

	lua_State *state = (lua_State *) vm;

	if (lua_isstring(state, -1) != 1 )
	{
		lua_pushnumber (state, API_FAILURE);
		LogWarn("Wrong type of parameter for make call");
		return 1;
	}

	size_t string_length = 0;
	const char *sip_uri = lua_tolstring(state, -1, &string_length);

	LogDebug("IwScript::LuaMakeCall dst:" << sip_uri);

	CallPtr call_ptr(new CallWithDirectRtp(_forking));

	const MediaFormatsPtrList &codecs_list = _conf.MediaFormats();
	for (MediaFormatsPtrList::const_iterator iter = codecs_list.begin(); iter != codecs_list.end(); iter++)
	{
		call_ptr->EnableMediaFormat(**iter);
	}

	ApiErrorCode res = call_ptr->MakeCall(sip_uri);
	LogDebug("IwScript::LuaMakeCall res:" << res);

	lua_pushnumber (state, res);
	if (IW_SUCCESS(res))
	{
		_callMap[call_ptr->StackCallHandle()] = call_ptr;
		lua_pushnumber (state, call_ptr->StackCallHandle());
	} else
	{
		lua_pushnil(state);
	}

	return 2;
}


int 
IwScript::LuaHangupCallByHandle(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;
	IX_PROFILE_FUNCTION();

	lua_State *state = (lua_State *) vm;

	if (lua_isnumber(state, -1) != 1 )
	{
		LogWarn("Wrong type of parameter for hangup_call");
		return 0;
	}

	HandleId handle =  (HandleId)lua_tonumber(state,  -1);

	CallMap::iterator iter  = _callMap.find(handle);
	if (iter == _callMap.end())
	{
		lua_pushnumber (state, API_FAILURE);
		
	} 
	else
	{
		(*iter).second->HangupCall();
		lua_pushnumber (state, API_SUCCESS);

	}

	return 1;

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

	LogStartScriptLog();
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

	LogStopScriptLog();


	lua_pushnumber (state, API_SUCCESS);
	return 1;

}


IwCallHandlerScript::IwCallHandlerScript(
	IN ScopedForking &forking,
	IN Configuration &conf,
	IN CLuaVirtualMachine &vm, 
	IN CallWithDirectRtp &call_session)
	:IwScript(forking,conf, vm),
	_incomingCallSession(call_session),
	_lineInTable(vm)
{
	FUNCTRACKER;

	if (vm.Ok() == false)
	{
		LogCrit("Detected uninitialized vm");
		throw;
	}

	_lineInTable.Create("linein");
	_lineInTable.AddParam("ani",_incomingCallSession.Ani());
	_lineInTable.AddParam("dnis",_incomingCallSession.Dnis());

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

	LogDebug("Running hangup script call iwh:" << _incomingCallSession.StackCallHandle());

	lua_State *state = (lua_State *) _vmPtr;

	lua_getfield(state, LUA_GLOBALSINDEX, "on_hangup");

	// hangup function exists?
	if (lua_isnil(state,-1) == 1)
	{
		LogDebug("Hangup script was not defined for call iwh:" << _incomingCallSession.StackCallHandle());
		return;
	}

	lua_call(state,0,0);

}


int
IwCallHandlerScript::LuaAnswerCall(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;
	IX_PROFILE_FUNCTION();

	LogDebug("Answer call iwh:" << _incomingCallSession.StackCallHandle());

	lua_State *state = (lua_State *) vm;

	ApiErrorCode res = _incomingCallSession.AcceptInitialOffer();
	lua_pushnumber (state, res);

	return 1;
}

int
IwCallHandlerScript::LuaHangupCall(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;
	IX_PROFILE_FUNCTION();

	LogDebug("Hangup call iw stack handle=[" << _incomingCallSession.StackCallHandle() << "].");

	lua_State *state = (lua_State *) vm;

	ApiErrorCode res = _incomingCallSession.HangupCall();
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

	LogDebug("Play file [" << file_to_play << "]  iw stack handle=[" << _incomingCallSession.StackCallHandle() << "].");

	ApiErrorCode res = _incomingCallSession.PlayFile(file_to_play,sync,loop);
	lua_pushnumber (state, res);

	return 1;

}

int
IwCallHandlerScript::LuaWaitForDtmf(CLuaVirtualMachine& vm)
{
	lua_State *state = (lua_State *) vm;
	long time_to_sleep  = (long) lua_tonumber (state, -1);

	LogDebug("Wait for dtmf  for " << time_to_sleep << " ms. iw stack handle=[" << _incomingCallSession.StackCallHandle() << "].");

	int dtmf = -1;
	ApiErrorCode res = _incomingCallSession.WaitForDtmf(dtmf, MilliSeconds(time_to_sleep));

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
		ApiErrorCode res = _incomingCallSession.SendRfc2833Dtmf(dtmf[curr_index++]);
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

	_incomingCallSession.BlindXfer(destination);


	lua_pushnumber (state, API_SUCCESS);
	return 1;


}


int
IwCallHandlerScript::LuaWaitTillHangup(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;
	IX_PROFILE_FUNCTION();

	LogDebug("Wait till hang up iwh:" << _incomingCallSession.StackCallHandle());

	lua_State *state = (lua_State *) vm;


	_incomingCallSession.WaitTillHangup();

	lua_pushnumber (state, API_SUCCESS);
	return 1;

}

}

