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

#pragma once

#include "LuaScript.h"
#include "LuaTable.h"
#include "CallWithRtpManagement.h"
#include "SqliteSession.h"

namespace ivrworx
{

	enum ScriptState
	{
		SCRIPT_STATE_NORMAL,
		SCRIPT_STATE_RUNNING_LONG_OPERATION
	};

/**
Core ivrworx &reg; lua interface for script that handles incoming call.

See @ref sql for more info on sql connectivity and db examples.

**/
class IwScript : 
	public CLuaScript
{
public:

	IwScript(
		IN ScopedForking &forking,
		IN Configuration &conf,
		IN CLuaVirtualMachine &vm);

	~IwScript();

protected:

	virtual int ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) ;

	virtual void HandleReturns (CLuaVirtualMachine& vm, const char *strFunc);

	/**
	ivrworx.wait (timeout) - Waits for [timeout] milliseconds.  

	@return always 0
	**/
	int LuaWait(CLuaVirtualMachine& vm);


	/**
	ivrworx.run(f) - if script needs to perform long blocking operation
	it should use this function. f will be executed in a separate thread,
	while main thread will be rescheduled waiting for an answer. No ivrworx
	media api can be used(!) inside these threads.

	@returns always 0
	**/
	int LuaRun(CLuaVirtualMachine& vm);

	/**
	ivrworx.make_call(f) - makes call to script uri. returns success result
	and handle to the call

	example:

	@codestart
	res,handle = ivrworx.make_call("sip:1234@example.com")
	@codeend

	@returns result of the operation
	**/
	int LuaMakeCall(CLuaVirtualMachine& vm);

	/**
	ivrworx.hangup(handle) - Hangs up the call by its handle (returned in make_call).  

	@return result of the operation
	**/
	int LuaHangupCall(CLuaVirtualMachine& vm);

	
	/**
	ivrworx.play(handle,filename,sync,loop) - Streams filename (wav) to the call specified by handle.  If sync is true the call will ext
	only upon end of streaming. If loop is true the file is being played indefinitely. You cannot set sync and loop
	to true simultaneously.

	@return result of the operation
	**/
	int LuaPlayFile(CLuaVirtualMachine& vm);

	/**
	ivrworx.wait_for_dtmf(handle) returns pair consisting of result and dtmf accepted on the call specified by handle.

	@returns pair of 0 and dtmf digit upon success or other error code and nil in case of error.
	**/
	int LuaWaitForDtmf(CLuaVirtualMachine& vm);

	/**
	ivrworx.send_dtmf(handle) - sends RFC2833to to the call specified by handle.

	@returns pair of 0 and dtmf digit upon success or other error code and nil in case of error.
	**/
	int LuaSendDtmf(CLuaVirtualMachine& vm);

	/**
	ivrworx.blind_xfer (handle,sip_uri) - unattended transfer to [sip_uri] of the call specified by handle..

	@returns pair of 0 and dtmf digit upon success or other error code and nil in case of error.
	**/
	int LuaBlindXfer(CLuaVirtualMachine& vm);

	/**
	ivrworx.wait_till_hangup (handle,sip_uri) - waits for the call specified by handle to hang up.

	@returns pair of 0 and dtmf digit upon success or other error code and nil in case of error.
	**/
	int LuaWaitTillHangup(CLuaVirtualMachine& vm);

	/**
	ivrworx.stop_play (handle) - stops any asynchronous playback on the call specified by handle.

	@returns result of the operation.
	**/
	int LuaStopPlay(CLuaVirtualMachine& vm);


	/**
	ivrworx.speak(handle, mrcp) - speaks mrcp request

	@returns result of the operation.
	**/
	int LuaSpeak(CLuaVirtualMachine& vm);

protected:

	CLuaVirtualMachine &_vmPtr;

	LuaTable _confTable;

	Configuration &_conf;

	ScopedForking &_forking;

	typedef 
	shared_ptr<CallWithRtpManagement> CallPtr;

	typedef 
	map<HandleId,CallPtr> CallMap;

	CallMap _callMap;

	ScriptState _scriptState;

private:

	int _methodBase;

};

class IwCallHandlerScript
	: public IwScript
{
public:

	IwCallHandlerScript(
		IN ScopedForking &forking,
		IN Configuration &conf, 
		IN CLuaVirtualMachine &vm,
		IN CallPtr call);


	/**
	When remote hangup is detected, the script is terminated and on_hangup function is called.
	Common mistake is to define the function at the end of script file. In this case the interpreter
	may not find it. Please define the function at the beginning of script file.

	**/
	void RunOnHangupScript();

protected:

	virtual int ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber);

private:

	int _methodBase;

	LuaTable _lineInTable;

	/**
	ivrworx.accept - Accepts the call. Upon calling this function is script 200 response will be 
	sent to remote party with media proposal. 

	@return 0 if call is suucessfully connected (ACK sent by remote party) or error code otherwise
	**/
	int LuaAnswerCall(CLuaVirtualMachine& vm);

	
	CallWithRtpManagementPtr _incomingCallSession;

};


class script_hangup_exception: 
	public std::exception
{

};

class ProcBlockingOperationRunner
	:public LightweightProcess
{
public:
	ProcBlockingOperationRunner(LpHandlePair pair, CLuaVirtualMachine& vm);

protected:
	void real_run();

protected:

	CLuaVirtualMachine& _vm;

};

}
