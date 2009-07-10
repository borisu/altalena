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
	ProcScriptRunner::RunSuperScript()
	{
		FUNCTRACKER;

		const string &script_name = _conf.SuperScript();

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

			

			START_FORKING_REGION;

			// compile the script if needed
			IwScript script(forking,_conf,vm);

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
			CLuaVirtualMachine vm;
			IX_PROFILE_CODE(vm.InitialiseVM());

			if (vm.Ok() == false)
			{
				LogCrit("Couldn't initialize lua vm");
				throw;
			}

			CLuaDebugger debugger(vm);

			_stackHandle = _initialMsg->stack_call_handle;

			
			START_FORKING_REGION;
			CallWithDirectRtp call_session(
				forking,
				_initialMsg);

			// the script should be terminated if caller hanged up
			call_session.SetEventListener(MSG_CALL_HANG_UP_EVT,_inbound);


			const MediaFormatsPtrList &codecs_list = _conf.MediaFormats();
			for (MediaFormatsPtrList::const_iterator iter = codecs_list.begin(); iter != codecs_list.end(); iter++)
			{
				call_session.EnableMediaFormat(**iter);
			}

			
			

			// compile the script if needed
			IwCallHandlerScript script(forking, _conf,vm,call_session);

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
			RunSuperScript();
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

}

