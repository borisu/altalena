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
#include "ScriptSession.h"
#include "CcuLogger.h"

ScriptSession::ScriptSession(IN LpHandlePair scriptRunnerPair, 
							 IN LightweightProcess &parentProc):
_scriptRunnerPair(scriptRunnerPair),
_parentProcess(parentProc)
{
}

ScriptSession::~ScriptSession(void)
{
}


CcuApiErrorCode 
ScriptSession::StartScript(wstring buffer)
{
	FUNCTRACKER;

	CcuMsgPtr response = CCU_NULL_MSG;

	CcuMsgStartScript *start_script_msg = new CcuMsgStartScript();
	start_script_msg->script_buffer = buffer;
	
	EventsSet responsesSet;
	responsesSet.insert(CCU_MSG_ACK);

	CcuApiErrorCode res = _parentProcess.DoRequestResponseTransaction(
		_scriptRunnerPair.inbound,
		CcuMsgPtr(start_script_msg),
		responsesSet,
		response,
		MilliSeconds(_parentProcess.TransactionTimeout()),
		L"Start Script TXN");

	return res;

}
