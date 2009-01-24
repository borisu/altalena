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
#include "Ccu.h"
#include "CcuLogger.h"
#include "ProcTestStub.h"

ProcTestStub::ProcTestStub(LpHandlePair pair):
LightweightProcess(pair,__FUNCTIONW__),
_numOfMessages(0),
_shutdownFlag(false),
_exitMsg(CCU_MSG_UNKNOWN)
{
	FUNCTRACKER;

}

ProcTestStub::ProcTestStub():
LightweightProcess(HANDLE_PAIR,__FUNCTIONW__),
_numOfMessages(0),
_shutdownFlag(false),
_exitMsg(CCU_MSG_UNKNOWN)
{
	FUNCTRACKER;

}

ProcTestStub::~ProcTestStub(void)
{
	FUNCTRACKER;

}

BOOL 
ProcTestStub::HandleOOBMessage(CcuMsgPtr msg)
{
	LightweightProcess::HandleOOBMessage(msg);
	return TRUE;
}

void
ProcTestStub::real_run()
{
	FUNCTRACKER;

	while(1)
	{
		CcuMsgPtr ptr = _inbound->Wait();

		LogInfo("TEST RCV msg=[" << ptr->message_id_str << "]");

		switch(ptr->message_id  )
		{
		case CCU_MSG_PING:
			{
				LightweightProcess::HandleOOBMessage(ptr);
				break;
			}
		default:
			{

			}

		}
		_numOfMessages++;

		if (ptr->message_id == _exitMsg)
		{
			LogInfo("Received exit message");
			break;
		}
	}	

	LogDebug("Exit...");

}
