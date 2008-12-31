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

#include "LightweightProcess.h"
#include "CcuConfiguration.h"
#include "Call.h"
#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "CallWithRTPManagment.h"


using namespace std;

class ProcVcs :
	public LightweightProcess
{

public:

#pragma TODO("Leave only ctor that receives configuration object as a parameter")

	ProcVcs(IN LpHandlePair pair, IN CnxInfo sip_stack_media);

	ProcVcs(IN LpHandlePair pair, IN CcuConfiguration &conf);

	virtual void real_run();

	virtual ~ProcVcs(void);

protected:

	BOOL ProcessStackMessage(
		IN CcuMsgPtr event,
		IN ScopedForking &forking
		);

	BOOL ProcessInboundMessage(
		IN CcuMsgPtr event,
		IN ScopedForking &forking
		);

	void StartScript(
		IN CcuMsgPtr msg);

private:

	LpHandlePair _stackPair;

	CnxInfo _sipStackData;

	CcuConfiguration *_conf;

	CLuaVirtualMachine _vm;

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

