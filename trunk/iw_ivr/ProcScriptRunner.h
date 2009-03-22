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


namespace ivrworx
{
	typedef 
		shared_ptr<CLuaVirtualMachine> CLuaVirtualMachinePtr;

	class ProcScriptRunner
		: public LightweightProcess

	{
	public:

		ProcScriptRunner(
			IN Configuration &conf,
			IN shared_ptr<MsgCallOfferedReq> msg, 
			IN LpHandlePair stack_pair, 
			IN LpHandlePair pair);

		~ProcScriptRunner();

		virtual void real_run();

		virtual BOOL HandleOOBMessage(IN IwMessagePtr msg);

	private:

		shared_ptr<MsgCallOfferedReq> _initialMsg;

		LpHandlePair _stackPair;

		Configuration &_conf;

		int _stackHandle;

	};

	class IwScript : 
		public CLuaScript
	{
	public:

		IwScript(
			IN Configuration &conf, 
			IN CLuaVirtualMachine &vm, 
			IN CallWithDirectRtp &call);

		~IwScript();

	private:

		// When the script calls a class method, this is called
		virtual int ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) ;

		// When the script function has returns
		virtual void HandleReturns (CLuaVirtualMachine& vm, const char *strFunc);

		int LuaAnswerCall(CLuaVirtualMachine& vm);

		int LuaHangupCall(CLuaVirtualMachine& vm);

		int LuaWait(CLuaVirtualMachine& vm);

		int LuaPlay(CLuaVirtualMachine& vm);

		int LuaWaitForDtmf(CLuaVirtualMachine& vm);

		int LuaSendDtmf(CLuaVirtualMachine& vm);

		int LuaBlindXfer(CLuaVirtualMachine& vm);

		CallWithDirectRtp &_callSession;

		int _methodBase;

		CLuaVirtualMachine &_vmPtr;

		LuaTable _confTable;

		LuaTable _lineInTable;

		Configuration &_conf;

	};


}
