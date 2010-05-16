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
#include "BridgeMacros.h"
#include "LoggerBridge.h"
#include "ConfBridge.h"
#include "CallBridge.h"
#include "MscmlCallBridge.h"
#include "ls_sqlite3.h"
#include "LuaRestoreStack.h"

#define IW_SCRIPT_RUNNER_ID "$OBJECT1$"


/*!
 * \brief
 * Write brief comment for ivrworx here.
 * 
 * Write detailed description for ivrworx here.
 * 
 * \remarks
 * Write remarks for ivrworx here.
 * 
 * \see
 * Separate items with the '|' character.
 */
namespace ivrworx
{
	
	class ProcBlockingOperationRunner
		:public LightweightProcess
	{
	public:
		ProcBlockingOperationRunner(LpHandlePair pair, lua_State *L, int &err):
		  LightweightProcess(pair,"LongOp runner"),
		  _L(L),
		  _err(err)
		  {

		  };

	protected:

		void real_run()
		{

			FUNCTRACKER;

			
			if (lua_isfunction (_L, -1))
			{
				_err  = lua_pcall (_L, 0, 0, 0);
			} 
			else
			{
				LogWarn("ProcBlockingOperationRunner::real_run - wrong param");
				_err = 1;
			}


		}

	protected:

		lua_State * _L;

		int &_err;

	};

	ProcScriptRunner* 
	GetScriptRunner(lua_State *state)
	{
		CLuaRestoreStack stack_guard(state);

		lua_pushstring(state, IW_SCRIPT_RUNNER_ID);
		lua_gettable(state, LUA_REGISTRYINDEX);

		ProcScriptRunner *runner = 
			(ProcScriptRunner*)lua_touserdata(state,-1);

		return runner;
	}


	ProcScriptRunner::ProcScriptRunner(
		IN Configuration &conf,
		IN const string &script_name,
		IN const char *precompiled_buffer,
		IN size_t buffer_size,
		IN shared_ptr<MsgCallOfferedReq> msg, 
		IN LpHandlePair spawn_pair,
		IN LpHandlePair pair)
		:LightweightProcess(pair,"IvrScript"),
		_conf(conf),
		_initialMsg(msg),
		_scriptName(script_name),
		_precompiledBuffer(precompiled_buffer),
		_bufferSize(buffer_size),
		_forking(NULL),
		_spawnPair(spawn_pair)
	{
		FUNCTRACKER;
	}


	ProcScriptRunner::ProcScriptRunner(
		IN Configuration &conf,
		IN shared_ptr<MsgIvrStartScriptReq> req,
		IN LpHandlePair spawn_pair,
		IN LpHandlePair pair)
		:LightweightProcess(pair,"IvrScript"),
		_conf(conf),
		_startScriptReq(req),
		_spawnPair(spawn_pair),
		_forking(NULL)
	{

	}

	ProcScriptRunner::~ProcScriptRunner()
	{
		FUNCTRACKER;

	}

	int
	ProcScriptRunner::LuaRunLongOperation(lua_State * state)
	{
		
	
		DECLARE_NAMED_HANDLE_PAIR(runner_pair);
		int err = 0;
		csp::Run(new ProcBlockingOperationRunner(runner_pair,state,err));

		err ? lua_pushnumber (state, API_FAILURE): lua_pushnumber (state, API_SUCCESS);

		return 1;
	}

	int
	ProcScriptRunner::LuaSpawn(lua_State *L)
	{
		FUNCTRACKER;
		
		ApiErrorCode result = API_WRONG_PARAMETER;

		if (lua_isstring(L, -2) != 1 ) 
		{ 
			lua_pushnumber (L, result); 
			return 1;
		};

		string filename = 
			lua_tostring(L,-2);

		
		shared_ptr<ProcParamMap> map_ptr(new ProcParamMap());

		/* table is in the stack at index 't' */
		lua_pushnil(L);  /* first key */
		while (lua_next(L, -2) != 0) {

			ProcParam p;
			long index = IW_UNDEFINED;

			// key
			switch (lua_type(L,-2))
			{
			case LUA_TNUMBER:
				{
					lua_Number number = lua_tonumber(L,-2);
					lua_number2int(index, number);
					break;
				}
			case LUA_TBOOLEAN:
			case LUA_TSTRING:
			case LUA_TLIGHTUSERDATA:
			case LUA_TTABLE:
			case LUA_TFUNCTION: 
			case LUA_TUSERDATA: 
			case LUA_TTHREAD:
			default:
				{
					LogWarn("ProcScriptRunner::LuaSpawn - Unsupported key type - " << lua_type(L, -2));
					/* removes 'value'*/
					lua_pop(L, 1);
					goto exit;

				}
			}

			// value
			switch (lua_type(L,-1))
			{
			case LUA_TNUMBER:
				{
					lua_Number number = lua_tonumber(L,-1);
					int value = IW_UNDEFINED;
					lua_number2int(value, number);

					p.param_type = PT_INTEGER;
					p.int_value = value;
					

					break;
				}
			case LUA_TBOOLEAN:
				{
					int value = lua_toboolean(L,-1);

					p.param_type = PT_BOOL;
					p.bool_value = value;
					
					break;
				}
			case LUA_TSTRING:
				{

					const char *value = lua_tostring(L,-1);
					p.param_type = PT_STRING;
					p.string_value = value;

					break;

				}
			case LUA_TLIGHTUSERDATA:
				{
					void *value = lua_touserdata(L,-1);
					p.param_type = PT_LIGHTUSERDATA;
					p.ud_value = value;

					break;

				}
			case LUA_TTABLE:
			case LUA_TFUNCTION: 
			case LUA_TUSERDATA: 
			case LUA_TTHREAD:
			default:
				{
					LogWarn("ProcScriptRunner::LuaSpawn - Unsupported value type - " << lua_type(L, -2));
					/* removes 'value'*/
					lua_pop(L, 1);
					goto exit;
				}
			};

			(*map_ptr)[index] = p;

			/* removes 'value'; keeps 'key' for next iteration */
			lua_pop(L, 1);

			MsgIvrStartScriptReq *req = new MsgIvrStartScriptReq();
			req->script_name = filename;
			req->params_map = map_ptr;

			ProcScriptRunner *runner = 
				GetScriptRunner(L);


			if (runner == NULL || runner->_forking == NULL)
			{
				LogWarn("ProcScriptRunner::LuaSpawn - Cannot find object1");
				return 0;
			}

			result = runner->_spawnPair.outbound->Send(req);

		}


exit:
		lua_pop(L, 1); // pop the key
		lua_pushnumber (L, result); 
		return 1;
		
	}

	void enable_configured_media_formats(Configuration &conf, CallWithRtpManagementPtr call_ptr)
	{
		ListOfAny codecs_list;
		conf.GetArray("codecs",codecs_list);

		for (ListOfAny::iterator conf_iter = codecs_list.begin(); 
			conf_iter != codecs_list.end(); 
			conf_iter++)
		{

			string conf_codec_name =  any_cast<string>(*conf_iter);
			MediaFormat media_format = MediaFormat::GetMediaFormat(conf_codec_name);


			call_ptr->EnableMediaFormat(media_format);
		}

	}

	

	int
	ProcScriptRunner::LuaCreateCall(lua_State *state)
	{
		FUNCTRACKER;

		
		ProcScriptRunner *runner = 
			GetScriptRunner(state);

		if (runner == NULL || runner->_forking == NULL)
		{
			LogWarn("ProcScriptRunner::LuaCreateCall - Cannot find object1");
			return 0;
		}

		CallWithRtpManagementPtr call_ptr 
			(new CallWithRtpManagement(runner->_conf, *runner->_forking));

		enable_configured_media_formats(runner->_conf,call_ptr);

		
		Luna<CallBridge>::PushObject(state,new CallBridge(call_ptr));

		return 1;
	}


	int
	ProcScriptRunner::LuaCreateMscmlCall(lua_State *state)
	{
		FUNCTRACKER;


		ProcScriptRunner *runner = 
			GetScriptRunner(state);

		if (runner == NULL || runner->_forking == NULL)
		{
			LogWarn("ProcScriptRunner::LuaCreateMscmlCall - Cannot find object1");
			return 0;
		}

		MscmlCallPtr call_ptr 
			(new ResipMscmlCall(*runner->_forking));

		Luna<MscmlCallBridge>::PushObject(state,new MscmlCallBridge(call_ptr));

		return 1;
	}


	int
	ProcScriptRunner::LuaWait(lua_State *state)
	{
		FUNCTRACKER;

		LUA_INT_PARAM(state,time_to_sleep,-1);

		LogDebug("IwScript::LuaWait - Wait for " << time_to_sleep);

#pragma warning (suppress:4244)
		csp::SleepFor(MilliSeconds(time_to_sleep));
		lua_pushnumber (state, API_SUCCESS);

		return 1;
	}

	BOOL 
	ProcScriptRunner::RunScript(IwScript &script)
	{

		FUNCTRACKER;

		bool res = false;
		if (_precompiledBuffer!=NULL)
		{
			res = script.CompileBuffer((unsigned char*)_precompiledBuffer,_bufferSize);
		} 
		else
		{
			res = script.CompileFile(_scriptName.c_str());
		}

		return res;

	}

	void 
	ProcScriptRunner::real_run()
	{

		FUNCTRACKER;

		HandleId iwh = (_initialMsg ? _initialMsg->stack_call_handle : IW_UNDEFINED);

		try
		{
			START_FORKING_REGION;

			CLuaVirtualMachine vm;
			vm.InitialiseVM();

			if (vm.Ok() == false)
			{
				LogCrit("Couldn't initialize lua vm");
				return;
			}

			CLuaDebugger debugger(vm);

			LuaTable ivrworx_table(vm);
			ivrworx_table.Create("ivrworx");
			ivrworx_table.AddParam(NAME(API_SUCCESS),API_SUCCESS);
			ivrworx_table.AddParam(NAME(API_FAILURE),API_FAILURE);
			ivrworx_table.AddParam(NAME(API_SERVER_FAILURE),API_SERVER_FAILURE);
			ivrworx_table.AddParam(NAME(API_TIMEOUT),API_TIMEOUT);
			ivrworx_table.AddParam(NAME(API_WRONG_PARAMETER),API_WRONG_PARAMETER);
			ivrworx_table.AddParam(NAME(API_WRONG_STATE),API_WRONG_STATE);
			ivrworx_table.AddParam(NAME(API_HANGUP),API_HANGUP);
			ivrworx_table.AddParam(NAME(API_UNKNOWN_DESTINATION),API_UNKNOWN_DESTINATION);
			ivrworx_table.AddParam(NAME(API_UNKNOWN_RESPONSE),API_UNKNOWN_RESPONSE);
			ivrworx_table.AddParam(NAME(API_FEATURE_DISABLED),API_FEATURE_DISABLED);
			ivrworx_table.AddFunction("sleep",ProcScriptRunner::LuaWait);
			ivrworx_table.AddFunction("run",ProcScriptRunner::LuaRunLongOperation);
			ivrworx_table.AddFunction("createcall",ProcScriptRunner::LuaCreateCall);
			ivrworx_table.AddFunction("createmscmlcall",ProcScriptRunner::LuaCreateMscmlCall);
			ivrworx_table.AddFunction("spawn",ProcScriptRunner::LuaSpawn);


			
			
			//
			// ivrworx.LOGGER
			//
			LoggerBridge b(vm);
			Luna<LoggerBridge>::RegisterType(vm,TRUE);
			Luna<LoggerBridge>::RegisterObject(vm,&b,ivrworx_table.TableRef(),"LOGGER");

			//
			// ivrworx.CONF
			//
			ConfBridge conf_bridge(&_conf);
			Luna<ConfBridge>::RegisterType(vm,TRUE);
			Luna<ConfBridge>::RegisterObject(vm,&conf_bridge,ivrworx_table.TableRef(),"CONF");

			// CallBridge
			Luna<CallBridge>::RegisterType(vm,FALSE);


			// MscmlCallBridge
			Luna<MscmlCallBridge>::RegisterType(vm,FALSE);

			//
			// register sql library
			//
			if (!luaopen_luasql_sqlite3(vm))
			{
				LogWarn("ProcScriptRunner::real_run - cannot register sql library");
				return;
			};



			//
			// register this
			//
			lua_pushstring(vm, IW_SCRIPT_RUNNER_ID);
			lua_pushlightuserdata(vm, this);
			lua_settable(vm, LUA_REGISTRYINDEX);

			
			_forking = &forking;


			//
			// incoming call case
			//
			if (_initialMsg)
			{
				

				_stackHandle = _initialMsg->stack_call_handle;


				CallWithRtpManagementPtr call_ptr(
					new CallWithRtpManagement(
					_conf,
					forking,
					_initialMsg));

				enable_configured_media_formats(_conf,call_ptr);

				//
				// ivrworx.INCOMING
				//
				// will be deleted by lua gc
				CallBridge *call_bridge = new CallBridge(call_ptr);
				
				Luna<CallBridge>::RegisterObject(vm,call_bridge,ivrworx_table.TableRef(),"INCOMING");


				// compile the script if needed
				IwScript script(vm);
				RunScript(script);
				


			} 
			else if (_startScriptReq)
			{

			}
			//
			// super script case
			//
			else 
			{
				IwScript script(vm);
				RunScript(script);

			}

			
			END_FORKING_REGION
			_forking = NULL;
		}
		catch (std::exception e)
		{
			LogWarn("Exception while running script:" << _scriptName <<", e:" << e.what() << ", iwh:" << iwh);
		}

		LogDebug("script:" << _scriptName << ", iwh:" << iwh << " completed.") ;
		
	}


	BOOL 
	ProcScriptRunner::HandleOOBMessage(IN IwMessagePtr msg)
	{
		// pings
		return LightweightProcess::HandleOOBMessage(msg);
	}

	
	IwScript::IwScript(CLuaVirtualMachine &vm): CLuaScript(vm)
	{
	};

	// When the script calls a class method, this is called
	int 
	IwScript::ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) 
	{
		return 0;
	};

	// When the script function has returns
	void 
	IwScript::HandleReturns (CLuaVirtualMachine& vm, const char *strFunc)
	{

	}
	

}

