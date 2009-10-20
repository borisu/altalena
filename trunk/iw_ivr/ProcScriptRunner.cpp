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
#include "IwScriptApi.h"
#include "BridgeMacros.h"
#include "LoggerBridge.h"
#include "ConfBridge.h"
#include "CallBridge.h"
#include "ls_sqlite3.h"

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
		_bufferSize(buffer_size)
	{
		FUNCTRACKER;
	}

	ProcScriptRunner::~ProcScriptRunner()
	{
		FUNCTRACKER;

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
			CLuaVirtualMachine vm;
			IX_PROFILE_CODE(vm.InitialiseVM());
			if (vm.Ok() == false)
			{
				LogCrit("Couldn't initialize lua vm");
				throw;
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

			//
			// register sql library
			//
			if (!luaopen_luasql_sqlite3(vm))
			{
				LogWarn("ProcScriptRunner::real_run - cannot register sql library");
				return;
			}


			START_FORKING_REGION;
	
			if (_initialMsg)
			{
				_stackHandle = _initialMsg->stack_call_handle;
				CallWithRtpManagementPtr call_session (
					new CallWithRtpManagement(
					_conf,
					forking,
					_initialMsg));

				// the script should be terminated if caller hanged up
				call_session->SetEventListener(MSG_CALL_HANG_UP_EVT,_inbound);


				const MediaFormatsPtrList &codecs_list = _conf.MediaFormats();
				for (MediaFormatsPtrList::const_iterator iter = codecs_list.begin(); iter != codecs_list.end(); iter++)
				{
					call_session->EnableMediaFormat(**iter);
				}


				//
				// ivrworx.INCOMING
				//
				CallBridge call_bridge(call_session.get());
				Luna<CallBridge>::RegisterType(vm,TRUE);
				Luna<CallBridge>::RegisterObject(vm,&call_bridge,ivrworx_table.TableRef(),"INCOMING");


				// compile the script if needed
				IwCallHandlerScript script(forking, _conf,vm,call_session);
				try 
				{
					RunScript(script);
				}
				catch (script_hangup_exception)
				{
					script.RunOnHangupScript();
				}


			} 
			else 
			{
				IwScript script(forking,_conf,vm);
				RunScript(script);

			}

			
			END_FORKING_REGION
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

}

