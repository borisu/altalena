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
#include "ls_sqlite3.h"

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


	ProcScriptRunner::ProcScriptRunner(
		IN Configuration &conf,
		IN const string &script_name,
		IN const char *precompiled_buffer,
		IN size_t buffer_size,
		IN shared_ptr<MsgCallOfferedReq> msg, 
		IN LpHandlePair stack_pair, 
		IN LpHandlePair pair)
		:LightweightProcess(pair,"IvrScript"),
		_conf(conf),
		_initialMsg(msg),
		_stackPair(stack_pair),
		_scriptName(script_name),
		_precompiledBuffer(precompiled_buffer),
		_bufferSize(buffer_size),
		_forking(NULL)
	{
		FUNCTRACKER;
	}

	ProcScriptRunner::~ProcScriptRunner()
	{
		FUNCTRACKER;

	}

	int
	ProcScriptRunner::LuaRun(lua_State * state)
	{
		
	
		DECLARE_NAMED_HANDLE_PAIR(runner_pair);
		int err = 0;
		csp::Run(new ProcBlockingOperationRunner(runner_pair,state,err));

		err ? lua_pushnumber (state, API_FAILURE): lua_pushnumber (state, API_SUCCESS);

		return 1;
	}

	int
	ProcScriptRunner::LuaCreateCall(lua_State *state)
	{
		FUNCTRACKER;

		lua_pushstring(state, IW_SCRIPT_RUNNER_ID);
		lua_gettable(state, LUA_REGISTRYINDEX);

		ProcScriptRunner *runner = 
			(ProcScriptRunner*)lua_touserdata(state,-1);

		lua_pop(state,1);

		if (runner == NULL || runner->_forking == NULL)
		{
			LogWarn("ProcScriptRunner::LuaCreateCall - Cannot find object1");
			return 0;
		}

		CallWithRtpManagementPtr call_ptr 
			(new CallWithRtpManagement(runner->_conf, *runner->_forking));

		const MediaFormatsPtrList &codecs_list = runner->_conf.MediaFormats();
		for (MediaFormatsPtrList::const_iterator iter = codecs_list.begin(); iter != codecs_list.end(); iter++)
		{
			call_ptr->EnableMediaFormat(**iter);
		}

		Luna<CallBridge>::PushObject(state,new CallBridge(call_ptr));

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
			ivrworx_table.AddFunction("run",ProcScriptRunner::LuaRun);
			ivrworx_table.AddFunction("createcall",ProcScriptRunner::LuaCreateCall);


			
			
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
				CallWithRtpManagementPtr call_session(
					new CallWithRtpManagement(
					_conf,
					forking,
					_initialMsg));


				const MediaFormatsPtrList &codecs_list = _conf.MediaFormats();
				for (MediaFormatsPtrList::const_iterator iter = codecs_list.begin(); iter != codecs_list.end(); iter++)
				{
					call_session->EnableMediaFormat(**iter);
				}


				//
				// ivrworx.INCOMING
				//
				// will be deleted by lua gc
				CallBridge *call_bridge = new CallBridge(call_session);
				
				Luna<CallBridge>::RegisterObject(vm,call_bridge,ivrworx_table.TableRef(),"INCOMING");


				// compile the script if needed
				IwScript script(vm);
				RunScript(script);
				


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
			LogWarn("Exception while running script:" << _conf.ScriptFile() <<", e:" << e.what() << ", iwh:" << iwh);
		}

		LogDebug("script:" << _conf.ScriptFile() << ", iwh:" << iwh << " completed.") ;
		
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

