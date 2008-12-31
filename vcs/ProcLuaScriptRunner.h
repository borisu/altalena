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
#include "lightweightprocess.h"
#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "CallWithRTPManagment.h"


class ProcLuaScriptRunner :
	public LightweightProcess
{
public:
	ProcLuaScriptRunner(IN LpHandlePair pair, IN LpHandlePair stack_handle);
	virtual ~ProcLuaScriptRunner(void);

	void real_run();
private:

	CLuaVirtualMachine _vm;

	LpHandlePair _stackHandle;

	void StartScript(CcuMsgPtr start_script_msg);
	

};

class CallFlowScript
	: public CLuaScript
{
public:

	CallFlowScript(IN CLuaVirtualMachine &vm, IN CallWithRTPManagment &call_session);

	~CallFlowScript();

	// When the script calls a class method, this is called
	virtual int ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) ;

	// When the script function has returns
	virtual void HandleReturns (CLuaVirtualMachine& vm, const char *strFunc);

private:

	int AnswerCall(CLuaVirtualMachine& vm);

	CallWithRTPManagment &_callSession;

	int _methodBase;

};
