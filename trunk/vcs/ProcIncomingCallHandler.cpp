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
#include "ProcIncomingCallHandler.h"
#include "CcuRTPSession.h"

ProcIncomingCallHandler::ProcIncomingCallHandler(
	IN LpHandlePair pair, 
	IN LpHandlePair stack)
:LightweightProcess(pair,__FUNCTIONW__),
_stackPair(stack)
{

}

ProcIncomingCallHandler::~ProcIncomingCallHandler(void)
{
}

void ProcIncomingCallHandler::real_run()
{
	
	FUNCTRACKER;

	CcuApiErrorCode res = CCU_API_SUCCESS;

	//
	// allocate local connection
	//
	CcuRtpSession local_session(*this);

	res = local_session.AllocateRTPConnection(_incomingCall->RemoteMedia());
	if (CCU_FAILURE(res))
	{
		LogWarn(">>Failed<< to Allocate RTP connection");
		_incomingCall->RejectCall();
		return;
	}

	res = _incomingCall->AcceptCall(local_session.LocalMediaData());
	if (CCU_FAILURE(res))
	{
		LogWarn(">>Failed<< to Accept RTP connection");
		return;
	}
		

	::Sleep(INFINITE);


}
