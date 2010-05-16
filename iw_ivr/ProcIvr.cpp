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
#include "ProcIvr.h"
#include "CallWithRtpManagement.h"
#include "ProcScriptRunner.h"
#include "LocalProcessRegistrar.h"
#include "ProcHandleWaiter.h"
#include "LuaUtils.h"



#define IVR_Q 0

using namespace boost::assign;

namespace ivrworx
{

ProcIvr::ProcIvr(IN LpHandlePair pair, IN Configuration &conf)
:LightweightProcess(pair,IVR_Q,	"ProcIvr"),
_conf(conf),
_precompiledBuffer(NULL),
_superSize(0),
_precompiledBuffer_Super(NULL),
_scriptSize(0),
_waitingForSuperCompletion(FALSE)
{
	_sipStackData = CnxInfo(
		convert_hname_to_addrin(_conf.GetString("ivr_sip_host").c_str()),
		_conf.GetInt("ivr_sip_port"));
}



ProcIvr::~ProcIvr(void)
{
	if (_precompiledBuffer_Super != NULL) ::free(_precompiledBuffer_Super);
	if (_precompiledBuffer != NULL) ::free(_precompiledBuffer);
}

void
ProcIvr::real_run()
{

	FUNCTRACKER;
	
	IwMessagePtr event;

	string super_script = _conf.GetString("super_script");

	//
	// Precompile files
	//
	if (_conf.GetBool("precompile"))
	{

		ApiErrorCode res = 
			LuaUtils::Precompile(_conf.GetString("script_file"),&_precompiledBuffer, &_scriptSize);

		if (IW_FAILURE(res))
		{
			LogCrit("ProcIvr::real_run - Cannot precompile script file res:" << res);
			throw critical_exception("Cannot precompile script");
		}

		if (super_script.empty() != false)
		{
			res = 
				LuaUtils::Precompile(super_script,&_precompiledBuffer_Super, &_superSize);
			if (IW_FAILURE(res))
			{
				LogCrit("ProcIvr::real_run - Cannot precompile super script file res:" << res);
				throw critical_exception("Cannot precompile super script");
			} //if failure precompiling
		}//if super exists
	}// if precompilation needed

	START_FORKING_REGION;

	

	LogInfo("Ivr process started successfully");
	I_AM_READY;

	//
	// Run super script
	//
	DECLARE_NAMED_HANDLE_PAIR(super_script_handle);
	DECLARE_NAMED_HANDLE_PAIR(spawn_handle);

	if (!super_script.empty())
	{
		AddShutdownListener(super_script_handle,_inbound);

		ProcScriptRunner *super_script_proc = 
			new ProcScriptRunner(
			_conf,								// configuration
			super_script,						// script name
			_precompiledBuffer_Super,			// precompiled buffer
			_superSize,							// size of precompiled buffer
			shared_ptr<MsgCallOfferedReq>(),	// initial incoming message
			_pair,								// handle used to send "spawn" messages
			super_script_handle					// handle created by stack for events
			);

		if (_conf.GetString("super_mode") == "sync" || 
			_conf.GetBool("ivr_enabled") == FALSE)
		{
			csp::RunInThisThread(super_script_proc);
		} 
		else
		{
			_waitingForSuperCompletion = TRUE;
			FORK_IN_THIS_THREAD(super_script_proc);
		}

	}

	if (_conf.GetBool("ivr_enabled") == FALSE)
	{
		Shutdown(Seconds(5),stack_pair);
		LogInfo("ivr_enabled:false... exiting");
		return;
	}
	

	HandlesVector list = list_of(stack_pair.outbound)(_inbound)(spawn_handle.outbound);

	//
	// Message Loop
	// 
	BOOL shutdown_flag = FALSE;
	while(shutdown_flag == FALSE)
	{
		IX_PROFILE_CHECK_INTERVAL(10000);
		
		int index = -1;
		ApiErrorCode err_code = 
			SelectFromChannels(
			list,
			Seconds(60), 
			index, 
			event);

		switch (err_code)
		{
		case API_TIMEOUT:
			{
				LogInfo("Ivr keep alive.");
				continue;
			}
		case API_SUCCESS:
			{
				break;
			}
		default:
			{
				LogCrit("ProcIvr::real_run - Unknown error code. Exiting");
				throw critical_exception("ProcIvr::real_run - Unknown error code. Exiting");
			}
		}

		
		switch (index)
		{
		case 0:
			{
				shutdown_flag = ProcessStackMessage(event, forking);
				if (shutdown_flag == TRUE)
				{
					LogWarn("Sip stack process terminated unexpectedly. Waiting for all calls to finish and exiting Ivr process.");
				}
				break;
			}
		case 1:
			{
				
				shutdown_flag = ProcessInboundMessage(event, forking);
				if (shutdown_flag == TRUE)
				{
					Shutdown(Time(Seconds(5)),super_script_handle);
					Shutdown(Time(Seconds(5)),stack_pair);
				}
				break;
			}
		case 2:
			{
				ProcessSpawnMessage(event, spawn_handle, forking);
				break;
			}
		default:
			{
				LogCrit("Unknown handle signaled");
				throw critical_exception("Unknown handle signaled");
			}
		}

	}

	END_FORKING_REGION;

	if (event != NULL_MSG && 
		event->message_id == MSG_PROC_SHUTDOWN_REQ)
	{

		SendResponse(event,new MsgShutdownAck());
	}


}

BOOL 
ProcIvr::ProcessSpawnMessage(IN IwMessagePtr event, IN LpHandlePair spawn_pair, IN ScopedForking &forking)
{
   FUNCTRACKER;

   shared_ptr<MsgIvrStartScriptReq> req = 
	   dynamic_pointer_cast<MsgIvrStartScriptReq> (event);

	switch (event->message_id)
	{
	case MSG_IVR_START_SCRIPT_REQ:
		{
			DECLARE_NAMED_HANDLE_PAIR(script_runner_handle);

			FORK_IN_THIS_THREAD(
				new ProcScriptRunner(
				_conf,					// configuration
				req,					// script name
				spawn_pair,				// on this handle "spawn" requests will be recieved
				script_runner_handle	// handle created by stack for events
				));

			return FALSE;
		}
	default:
		{
			BOOL oob_res = HandleOOBMessage(event);
			if (oob_res = FALSE)
			{
				LogCrit("ProcIvr::ProcessSpawnMessage - Unknown message received id=[" << event->message_id_str << "]");
				throw;
			}
		}
	}

	return FALSE;

}

BOOL 
ProcIvr::ProcessInboundMessage(IN IwMessagePtr event, IN ScopedForking &forking)
{
	FUNCTRACKER;
	switch (event->message_id)
	{
	case MSG_PROC_SHUTDOWN_EVT:
		{
			// currently it can be only from super script, so no checking.
			_waitingForSuperCompletion = FALSE;
			return FALSE;
		}
	case MSG_PROC_SHUTDOWN_REQ:
		{
			return TRUE;
		}
	default:
		{
			BOOL oob_res = HandleOOBMessage(event);
			if (oob_res = FALSE)
			{
				LogCrit("Unknown message received id=[" << event->message_id_str << "]");
				throw;
			}
		}
	}

	return FALSE;

}

BOOL 
ProcIvr::ProcessStackMessage(IN IwMessagePtr ptr, IN ScopedForking &forking)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case MSG_CALL_OFFERED:
		{

			shared_ptr<MsgCallOfferedReq> call_offered = 
				shared_polymorphic_cast<MsgCallOfferedReq> (ptr);

			// still waiting for super to finish
			if (_waitingForSuperCompletion == TRUE)
			{
				LogWarn("Receive CallOfferedMsg while super script is running in sync mode. Rejecting the call iwh:" << call_offered->stack_call_handle);
				SendResponse(ptr, new MsgCallOfferedNack());
				return FALSE;
			}


			DECLARE_NAMED_HANDLE_PAIR(script_runner_handle);

			FORK_IN_THIS_THREAD(
					new ProcScriptRunner(
						_conf,							// configuration
						_conf.GetString("script_file"),	// script name
						_precompiledBuffer,				// precompiled buffer
						_scriptSize,					// size of precompiled buffer
						call_offered,					// initial incoming message
						_pair,							// handle used to send "spawn" messages
						script_runner_handle			// handle created by stack for events
				));

			return FALSE;

		}
	
	case MSG_PROC_SHUTDOWN_EVT:
		{
			LogWarn("Detected Sip process shutdown. Exiting.");
			return TRUE;
			break;
		}
	default:
		{
			LogWarn("Unknown message from stack " << ptr->message_id_str);
			return FALSE;
		}
	}

	return FALSE;

}

}



