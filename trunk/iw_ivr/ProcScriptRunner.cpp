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
#include "SipCallBridge.h"
#include "RTPProxyBridge.h"
#include "ls_sqlite3.h"
#include "LuaRestoreStack.h"
#include "StreamerBridge.h"




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
		IN ConfigurationPtr conf,
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


	ProcScriptRunner::~ProcScriptRunner()
	{
		FUNCTRACKER;
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

			_ctx._forking = &forking;
			_ctx._conf    = _conf;

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

			InitStaticTypes(vm,ivrworx_table,&_ctx);


			_forking = &forking;


			//
			// incoming call case
			//
			if (_initialMsg)
			{
				

				_stackHandle = _initialMsg->stack_call_handle;

				throw;


				
// 				CallWithRtpManagementPtr call_ptr(
// 					new CallWithRtpManagement(
// 					_conf,
// 					forking,
// 					MediaCallSessionPtr(), // temporary!!!!
// 					_initialMsg));
// 
// 				enable_configured_media_formats(_conf,call_ptr);
// 
// 				//
// 				// ivrworx.INCOMING
// 				//
// 				// will be deleted by lua gc
// 				CallBridge *call_bridge = new CallBridge(call_ptr);
// 				
// 				Luna<CallBridge>::RegisterObject(vm,call_bridge,ivrworx_table.TableRef(),"INCOMING");


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

