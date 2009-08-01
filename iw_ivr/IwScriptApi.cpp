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

#define IW_REGISTER_GLOBAL_VAR(V,N) lua_pushnumber((lua_State *)V,N); \
	lua_setglobal ((lua_State *)V, #N);


namespace ivrworx
{

IwScript::IwScript(IN ScopedForking &forking,
				   IN Configuration &conf,
				   IN CLuaVirtualMachine &vm)
				   :CLuaScript(vm),
				   _forking(forking),
				   _vmPtr(vm),
				   _confTable(vm),
				   _conf(conf),
				   _scriptState(SCRIPT_STATE_NORMAL)
{
	FUNCTRACKER;

	if (vm.Ok() == false)
	{
		LogCrit("Detected uninitialized vm");
		throw;
	}

	IW_REGISTER_GLOBAL_VAR(vm,API_SUCCESS);
	IW_REGISTER_GLOBAL_VAR(vm,API_FAILURE);
	IW_REGISTER_GLOBAL_VAR(vm,API_HANGUP);
	IW_REGISTER_GLOBAL_VAR(vm,API_SERVER_FAILURE);
	IW_REGISTER_GLOBAL_VAR(vm,API_SUCCESS);
	IW_REGISTER_GLOBAL_VAR(vm,API_TIMEOUT);
	IW_REGISTER_GLOBAL_VAR(vm,API_WRONG_PARAMETER);
	IW_REGISTER_GLOBAL_VAR(vm,API_WRONG_STATE);


	_confTable.Create("conf");
	_confTable.AddParam("sounds_dir",_conf.SoundsPath());


	// !!! The order should be preserved for later switch statement !!!
	_methodBase = RegisterFunction("iw_log");
	RegisterFunction("wait");
	RegisterFunction("run");
	RegisterFunction("make_call");
	RegisterFunction("hangup");
	RegisterFunction("play");
	RegisterFunction("wait_for_dtmf");
	RegisterFunction("send_dtmf");
	RegisterFunction("blind_xfer");
	RegisterFunction("wait_till_hangup");
	RegisterFunction("stop_play");


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
	case 4:
		{
			return LuaHangupCall(vm);
		}
	case 5:
		{
			return LuaPlayFile(vm);
		}
	case 6:
		{
			return LuaWaitForDtmf(vm);
		}
	case 7:
		{
			return LuaSendDtmf(vm);
		}
	case 8:
		{
			return LuaBlindXfer(vm);
		}
	case 9:
		{
			return LuaWaitTillHangup(vm);
		}
	case 10:
		{
			return LuaStopPlay(vm);
		}
	default:
		{
			LogWarn ("IwScript::ScriptCalling - unknown function called id:" << iFunctionNumber);
			return -1;
		}

	}

	return 0;
}

int
IwScript::LuaStopPlay(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;
	
	lua_State *state = (lua_State *) vm;

	if (_scriptState != SCRIPT_STATE_NORMAL)
	{
		lua_pushnumber (state, API_WRONG_STATE);
		return 1;
	}

	if (lua_isnumber(state, -1) != 1 )
	{
		LogWarn("IwScript::LuaStopPlay - Wrong type of parameter for hangup_call");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	HandleId handle =  (HandleId)lua_tonumber(state,  -1);

	CallMap::iterator iter  = _callMap.find(handle);
	if (iter == _callMap.end())
	{
		LogWarn("IwScript::LuaHangupCall - not found iwh:" << handle);
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;

	} 

	CallPtr call = (*iter).second;
	
	ApiErrorCode res = call->StopPlay();
	lua_pushnumber (state, res);


	return 1;

}

int
IwScript::LuaRun(CLuaVirtualMachine& vm)
{
	if (_scriptState != SCRIPT_STATE_NORMAL)
	{
		return 0;
	}
	
	lua_State *state = (lua_State *) vm;

	if (lua_isfunction(state, -1) == 0 )
	{
		LogWarn("IwScript::LuaRun - closure param of incorrect type");
		return 0;
	}

	_scriptState = SCRIPT_STATE_RUNNING_LONG_OPERATION;

	DECLARE_NAMED_HANDLE_PAIR(runner_pair);
	csp::Run(new ProcBlockingOperationRunner(runner_pair,vm));

	_scriptState = SCRIPT_STATE_NORMAL;

	return 0;
}


int
IwScript::LuaWait(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;

	lua_State *state = (lua_State *) vm;

	if (lua_isnumber (state, -1) != 1 )
	{
		LogWarn("IwScript::LuaWait - timeout param of incorrect type");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	long time_to_sleep  = (long) lua_tonumber (state, -1);

	LogDebug("IwScript::LuaWait - Wait for " << time_to_sleep);

	csp::SleepFor(MilliSeconds(time_to_sleep));
	lua_pushnumber (state, API_SUCCESS);

	return 1;

}


int 
IwScript::LuaMakeCall(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;
	
	lua_State *state = (lua_State *) vm;

	if (_scriptState != SCRIPT_STATE_NORMAL)
	{
		lua_pushnumber (state, API_WRONG_STATE);
		lua_pushnil(state);
		return 2;
	}

	if (lua_isstring(state, -1) != 1 )
	{
		LogWarn("IwScript::LuaMakeCall - dst parameter of incorrect type.");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		lua_pushnil(state);
		return 2;
	}

	size_t string_length = 0;
	const char *sip_uri = lua_tolstring(state, -1, &string_length);

	LogDebug("IwScript::LuaMakeCall - dst:" << sip_uri);

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
	} 
	else
	{
		lua_pushnil(state);
	}

	return 2;
}


int 
IwScript::LuaHangupCall(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;
	

	lua_State *state = (lua_State *) vm;

	if (_scriptState != SCRIPT_STATE_NORMAL)
	{
		lua_pushnumber (state, API_WRONG_STATE);
		return 1;
	}

	if (lua_isnumber(state, -1) != 1 )
	{
		LogWarn("IwScript::LuaHangupCall - Wrong type of parameter for hangup_call");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	HandleId handle =  (HandleId)lua_tonumber(state,  -1);

	CallMap::iterator iter  = _callMap.find(handle);
	if (iter == _callMap.end())
	{
		LogWarn("IwScript::LuaHangupCall - not found iwh:" << handle);
		lua_pushnumber (state, API_WRONG_PARAMETER);
		
	} 
	else
	{
		(*iter).second->HangupCall();
		lua_pushnumber (state, API_SUCCESS);

	}

	_callMap.erase(iter);

	return 1;

}

int 
IwScript::LuaPlayFile(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;

	lua_State *state = (lua_State *) vm;

	if (_scriptState != SCRIPT_STATE_NORMAL)
	{
		lua_pushnumber (state, API_WRONG_STATE);
		return 1;
	}

	if (lua_isnumber(state, -4) != 1 )
	{
		LogWarn("IwScript::LuaPlayFile - Wrong type of parameter for play - handle");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	if (lua_isstring(state, -3) != 1 )
	{
		LogWarn("IwScript::LuaPlayFile - Wrong type of parameter for play - filename");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	if (lua_isboolean(state, -2) != 1 )
	{
		LogWarn("IwScript::LuaPlayFile - Wrong type of parameter for play - sync");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	if (lua_isboolean(state, -1) != 1 )
	{
		LogWarn("IwScript::LuaPlayFile - Wrong type of parameter for play - loop");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	
	int handle	= (int)lua_tonumber(state, -4);

	
	CallMap::iterator iter  = _callMap.find(handle);
	if (iter == _callMap.end())
	{
		lua_pushnumber (state, API_WRONG_PARAMETER);
		LogWarn("IwScript::LuaPlayFile - cannot find call iwh:" << handle);
		return 1;
	}
	
	CallPtr call = (*iter).second;

	size_t string_length	 = 0;
	const char *file_to_play = lua_tolstring(state, -3, &string_length);
	BOOL sync				 = lua_toboolean(state, -2);
	BOOL loop				 = lua_toboolean(state, -1);

	LogDebug("Play file:" << file_to_play << ", iwh:" << call->StackCallHandle() );

	ApiErrorCode res = call->PlayFile(file_to_play,sync,loop);
	lua_pushnumber (state, res);

	return 1;

}

int
IwScript::LuaWaitForDtmf(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;

	lua_State *state = (lua_State *) vm;

	if (_scriptState != SCRIPT_STATE_NORMAL)
	{
		lua_pushnumber (state, API_WRONG_STATE);
		lua_pushnil(state);
		return 2;
	}

	if (lua_isnumber(state, -2) != 1 )
	{
		LogWarn("IwScript::LuaWaitForDtmf - handle param of incorrect type.");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	if (lua_isnumber(state, -1) != 1 )
	{
		LogWarn("IwScript::LuaWaitForDtmf - timeout param of incorrect type.");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}


	long handle = (long) lua_tonumber (state, -2);

	CallMap::iterator iter  = _callMap.find(handle);
	if (iter == _callMap.end())
	{
		lua_pushnumber (state, API_WRONG_PARAMETER);
		LogWarn("IwScript::LuaWaitForDtmf - Cannot find call iwh:" << handle);
		return 1;

	}

	CallPtr call = (*iter).second;

	long time_to_sleep  = (long) lua_tonumber (state, -1);

	LogDebug("IwScript::LuaWaitForDtmf - timeout:" << time_to_sleep << ", iwh:" << handle);

	int dtmf = -1;
	ApiErrorCode res = call->WaitForDtmf(dtmf, MilliSeconds(time_to_sleep));

	lua_pushnumber (state, res);
	if (IW_SUCCESS(res))
	{
		lua_pushnumber (state, dtmf);
	} 
	else
	{
		lua_pushnil(state);
	}

	return 2;

}

int
IwScript::LuaSendDtmf(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;

	lua_State *state = (lua_State *) vm;

	if (_scriptState != SCRIPT_STATE_NORMAL)
	{
		lua_pushnumber (state, API_WRONG_STATE);
		return 1;
	}

	if (lua_isnumber(state, -1) != 1 )
	{
		LogWarn("IwScript::LuaWaitForDtmf - handle param of incorrect type.");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	if (lua_isstring(state, -2) == 0 )
	{
		LogWarn("IwScript::LuaWaitForDtmf - dtmf param of incorrect type.");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	long handle = (long) lua_tonumber (state, -2);

	CallMap::iterator iter  = _callMap.find(handle);
	if (iter == _callMap.end())
	{
		lua_pushnumber (state, API_WRONG_PARAMETER);
		LogWarn("IwScript::LuaWaitForDtmf - Cannot find call iwh:" << handle);
		return 1;

	} 

	CallPtr call = (*iter).second;

	const char* dtmf = lua_tostring(state, -1);

	int curr_index = 0;
	while (dtmf[curr_index] != '\0')
	{
		ApiErrorCode res = call->SendRfc2833Dtmf(dtmf[curr_index++]);
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
	FUNCTRACKER;

	lua_State *state = (lua_State *) vm;

	if (_scriptState != SCRIPT_STATE_NORMAL)
	{
		lua_pushnumber (state, API_WRONG_STATE);
		return 1;
	}

	if (lua_isnumber(state, -2) != 1 )
	{
		LogWarn("IwCallHandlerScript::LuaBlindXfer - handle param of incorrect type.");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	if (lua_isstring(state, -1) != 1 )
	{
		LogWarn("IwCallHandlerScript::LuaBlindXfer - destination para, of incorrect type");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	long handle = (long) lua_tonumber (state, -2);

	CallMap::iterator iter  = _callMap.find(handle);
	if (iter == _callMap.end())
	{
		lua_pushnumber (state, API_WRONG_PARAMETER);
		LogWarn("IwScript::LuaWaitForDtmf - Cannot find call iwh:" << handle);
		return 1;

	}

	CallPtr call = (*iter).second;


	const char* destination = lua_tostring(state, -1);
	ApiErrorCode res = call->BlindXfer(destination);

	if (IW_SUCCESS(res))
	{
		_callMap.erase(iter);
	}


	lua_pushnumber (state, res);
	return 1;


}


int
IwScript::LuaWaitTillHangup(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;

	lua_State *state = (lua_State *) vm;

	if (_scriptState != SCRIPT_STATE_NORMAL)
	{
		lua_pushnumber (state, API_WRONG_STATE);
		return 1;
	}

	if (lua_isnumber(state, -1) != 1 )
	{
		LogWarn("IwScript::LuaWaitTillHangup - handle param of incorrect type.");
		lua_pushnumber (state, API_WRONG_PARAMETER);
		return 1;
	}

	long handle = (long) lua_tonumber (state, -1);

	LogDebug("IwScript::LuaWaitTillHangup - iwh:" << handle);

	CallMap::iterator iter  = _callMap.find(handle);
	if (iter == _callMap.end())
	{
		lua_pushnumber (state, API_WRONG_PARAMETER);
		LogWarn("IwScript::LuaWaitTillHangup - Cannot find call iwh:" << handle);
		return 1;

	}

	CallPtr call = (*iter).second;

	call->WaitTillHangup();

	lua_pushnumber (state, API_SUCCESS);
	return 1;

}


int
IwScript::LuaLog(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;
	
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
	IN CallWithDirectRtpPtr call_session)
	:IwScript(forking,conf, vm),
	_incomingCallSession(call_session),
	_lineInTable(vm)
{
	FUNCTRACKER;

	if (vm.Ok() == false)
	{
		LogCrit("Detected uninitialized vm");
	}

	// register INCOMING

	lua_pushnumber((lua_State *)_vmPtr,call_session->StackCallHandle());
	lua_setglobal ((lua_State *)_vmPtr,"INCOMING");

	
	_callMap[call_session->StackCallHandle()] = call_session;


	_lineInTable.Create("linein");
	_lineInTable.AddParam("ani",_incomingCallSession->Ani());
	_lineInTable.AddParam("dnis",_incomingCallSession->Dnis());


	// !!! The order should be preserved for later switch statement !!!
	_methodBase = RegisterFunction("answer");
	
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
	

	LogDebug("Running hangup script call iwh:" << _incomingCallSession->StackCallHandle());

	lua_State *state = (lua_State *) _vmPtr;

	lua_getfield(state, LUA_GLOBALSINDEX, "on_hangup");

	// hangup function exists?
	if (lua_isnil(state,-1) == 1)
	{
		LogDebug("Hangup script was not defined for call iwh:" << _incomingCallSession->StackCallHandle());
		return;
	}

	lua_call(state,0,0);

}


int
IwCallHandlerScript::LuaAnswerCall(CLuaVirtualMachine& vm)
{
	FUNCTRACKER;

	LogDebug("Answer call iwh:" << _incomingCallSession->StackCallHandle());

	lua_State *state = (lua_State *) vm;

	if (_scriptState != SCRIPT_STATE_NORMAL)
	{
		lua_pushnumber (state, API_WRONG_STATE);
		return 1;
	}

	ApiErrorCode res = _incomingCallSession->AcceptInitialOffer();
	lua_pushnumber (state, res);

	return 1;
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

} // namespace
