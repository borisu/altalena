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

#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "CallWithDirectRtp.h"
#include "LuaTable.h"

/**
@defgroup scripts Scripts
 
@section Fibers Fibers Context Consideration

In ivrworx you write ivr scripts in lua scripting language. For every incoming call, the 'script_file' entry 
in configuration file, denotes the lua script that should be run upon it. 

The most important thing that you should remember is that your scripts are running in fibers and context
switch will occur only when fiber goes into waiting state or exits. There's a major difference between 
running  scripts in different threads and running them in fibers. When working with threads, OS is responsible
for context switch and programmer has no control over when exactly it will happen. With fibers, programmer 
has an explicit control over context switching. This makes writing highly parallel application more easy
as one does not have to lock the resources used by different fibers in the same thread. However, with great
power comes great responsibility. You may only do fast non-blocking operation in fibers. Remember that API
exposed to you by ivrworx library has no really "blocking" calls. For instance, if you use play_file, api the request
for file streaming will be immediately sent, and then internal wait function will be called, which will make 
<A HREF="http://www.cs.kent.ac.uk/projects/ofa/c++csp/doc/index.html">CSP</A> kernel reschedule the process. 
In other words, under the hood, the synchronous API utilizes asynchronous communication with rescheduling between 
sending request and receiving response. 

If you are a developer and you want to extend ivrworx API to do some time consuming operations, say calling 
a web service you have two options.

If you want to implement the API in O(1) threads then you should work a little hard and do the following:-

- Implement different server thread which will receive web service requests invokation from client and 
will actually invokes them.

- Implement proxy call object which will send the request to the server and wait for the response by calling
ivrworx rescheduling API.

- Expose proxy API to the script.

You may look at ivrworx::Call or ivrworx::ImsSession classes for reference implementation.

Another more simple option but which will roughly open new thread for the operation is using embedded <A href="http://luaforge.net/projects/lanes/">lanes 2.0.3</A> library.
It is quite powerful library but as mentioned earlier its greatest disadvantage is opening new thread for the task may
be unacceptable for large enterprise system (and ivrworx is basically designed with such systems in mind). Lanes library
was slightly modified so when waiting for result of asynchronous call it does not hang main thread which reschedules
to another task. So no worries here. Pay attention the ivrworx API is not thread safe and should not be available
from lanes threads - don't use it there!, open new threads only for media unrelated tasks like querying db or running
web service. Of cause you should also be aware of other lanes limitations which are outlined in its documentation. 

This is the small example of two simple functions runnning in  parallel (attached as super script by default)
@code
require "ivrworx"
require "ivrworx_lanes"

local il = ivrworx_lanes

f= ivrworx_lanes.gen( function(n) return 2*n end )

a= f(1)
b= f(2)

ivrworx.loginf( "script>"..a[1])
ivrworx.loginf( "script>"..b[1])
@endcode

Pay attention that 

@section API ivrworx API

The api that ivrworx provides to lua scripts is divided into two groups :-
- Core functions which are bound to C functions. See documentation for ivrworx::IwScript class in order to see documentation for all exposed internal API. 

- Utility functions which are provided as additional lua scripts for example, play_phrase.lua exposes
play_number function which only uses core play_file API. All parsing logic is implemented in lua script itself.
You should inspect lua modules for more documentation regarding the utility functions.

@section Scripts Scripts Structure

Put your lua script inside scripts directory under ivrworx root location. First thing that you should do in your scripts 
is add 'require' keyword in order for lua vm to import functions provided by ivrworx.

@code

-- import core media control function
require "ivrworx";

-- import play_number function
require "play_phrase";
...
@endcode

The next important thing you should know regarding the scripts, is that they are started when call is in offered state. So you have
to give an explicit command to accept or reject the call, e.g. somewhere in beginning of your code you should insert a call to 
ivrworx.answer core API.

@code
require "ivrworx";
require "play_phrase";
...
if lineani["ani"]=="1234" then 
	ivrworx.answer();
else
	ivrworx.hangup();
end
...
@endcode 

@section Tables

ivrworx exposes the following tables that are populated with call related values upon call arrival.

<table border="1">
<tr><th>Value</th><th>Notes</th></tr>
<tr><td>sounds_dir</td><td>contains directory where basic sound files are stored</td>	</tr>
</table>

<table border="1">
<tr> <th>Value</th><th>Notes</th></tr>
<tr> <td>ani</td>  <td>contains ani of incoming call</td>	</tr>
<tr> <td>dnis</td> <td>contains ani of incoming call</td>	</tr>
</table> 

**/
namespace ivrworx
{
	typedef 
		shared_ptr<CLuaVirtualMachine> CLuaVirtualMachinePtr;

	/**
	Main Ivr process implementation
	**/
	class ProcScriptRunner
		: public LightweightProcess

	{
	public:

		ProcScriptRunner(
			IN Configuration &conf,
			IN shared_ptr<MsgCallOfferedReq> msg, 
			IN LpHandlePair stack_pair, 
			IN LpHandlePair pair);

		ProcScriptRunner(
			IN Configuration &conf,
			IN LpHandlePair stack_pair, 
			IN LpHandlePair pair);

		~ProcScriptRunner();

		virtual void real_run();

		virtual BOOL HandleOOBMessage(IN IwMessagePtr msg);

	private:

		virtual void RunStandAlone();

		virtual void RunIncomingCallHandler();

		shared_ptr<MsgCallOfferedReq> _initialMsg;

		LpHandlePair _stackPair;

		Configuration &_conf;

		int _stackHandle;

	};


	/**
	Core ivrworx &reg; lua interface for script that handles incoming call.
	**/
	class IwScript : 
		public CLuaScript
	{
	public:

		IwScript(
			IN Configuration &conf, 
			IN CLuaVirtualMachine &vm);

		~IwScript();

	protected:

		virtual int ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) ;

		virtual void HandleReturns (CLuaVirtualMachine& vm, const char *strFunc);

		/**
		ivrworx.wait timeout) - Waits for [timeout] milliseconds.  

		@return always 0
		**/
		int LuaWait(CLuaVirtualMachine& vm);

		/**
		ivrworx.log(log_level,log) - script logging.
		
		@returns always 0
		**/
		int LuaLog(CLuaVirtualMachine& vm);

		int _methodBase;

		CLuaVirtualMachine &_vmPtr;

		LuaTable _confTable;

		LuaTable _lanesTable;

		Configuration &_conf;

	};

	class IwCallHandlerScript
		: public IwScript
	{
	public:

		IwCallHandlerScript(
			IN Configuration &conf, 
			IN CLuaVirtualMachine &vm, 
			IN CallWithDirectRtp &call);


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

		/**
		ivrworx.hangup - Hangs up the call.  

		@return always 0
		**/
		int LuaHangupCall(CLuaVirtualMachine& vm);

		/**
		ivrworx.play(filename,sync,loop) - Streams filename (wav) to the caller.  If sync is true the call will ext
		only upon end of streaming. If loop is true the file is being played indefinitely. You cannot set sync and loop
		to true simultaneously.

		@return 0 upon success.
		**/
		int LuaPlay(CLuaVirtualMachine& vm);

		/**
		ivrworx.wait_for_dtmf returns pair consisting of result and dtmf accepted

		@returns pair of 0 and dtmf digit upon success or other error code and nil in case of error.
		**/
		int LuaWaitForDtmf(CLuaVirtualMachine& vm);

		/**
		ivrworx.send_dtmf - sends RFC2833to caller.

		@returns pair of 0 and dtmf digit upon success or other error code and nil in case of error.
		**/
		int LuaSendDtmf(CLuaVirtualMachine& vm);

		/**
		ivrworx.blind_xfer (sip_uri) - unattended transfer to [sip_uri].

		@returns pair of 0 and dtmf digit upon success or other error code and nil in case of error.
		**/
		int LuaBlindXfer(CLuaVirtualMachine& vm);

		/**
		ivrworx.blind_xfer (sip_uri) - unattended transfer to [sip_uri].

		@returns pair of 0 and dtmf digit upon success or other error code and nil in case of error.
		**/
		int LuaWaitTillHangup(CLuaVirtualMachine& vm);

		CallWithDirectRtp &_callSession;

	};


	class script_hangup_exception: 
		public std::exception
	{

	};

}
