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
#include "CallWithRtpManagement.h"
#include "LuaTable.h"
#include "IwScriptApi.h"

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

Another more simple option but which will roughly open new thread for the operation ivrworx.run. This function
will open a background thread for executing lua code while rescheduling the main thread.

This is the small example:

@code
require "ivrworx"

f= ivrworx.run( function() sql_conn = open_sql_connection() end )

@endcode

Pay attention that ivrwox api cannot be used within such a block.

@section API ivrworx API

The api that ivrworx provides to lua scripts is divided into two groups :-
- Core functions which are bound to C functions. See documentation for ivrworx::IwScript class in order to see documentation for all exposed internal API. 
You may also look at @ref sql for info on working with sqlite db.

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
Table conf:-
<table border="1">
<tr><th>Value</th><th>Notes</th></tr>
<tr><td>sounds_dir</td><td>contains directory where basic sound files are stored</td>	</tr>
</table>

Table linein:-
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
			IN const string &script_name,
			IN const char *precompiled_buffer,
			IN size_t buffer_size,
			IN shared_ptr<MsgCallOfferedReq> msg, 
			IN LpHandlePair stack_pair, 
			IN LpHandlePair pair);

		~ProcScriptRunner();

		virtual void real_run();

		virtual BOOL RunScript(IwScript &script);

		virtual BOOL HandleOOBMessage(IN IwMessagePtr msg);

		static int LuaWait(lua_State *L);

	private:

		shared_ptr<MsgCallOfferedReq> _initialMsg;

		LpHandlePair _stackPair;

		Configuration &_conf;

		int _stackHandle;

		const string _scriptName;

		const char *_precompiledBuffer;

		size_t _bufferSize;

	};


	
}
