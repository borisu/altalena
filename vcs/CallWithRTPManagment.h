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

#include "call.h"
#include "CcuRTPSession.h"
#include "ImsSession.h"

using namespace ivrworx;

class CallWithRTPManagment :
	public Call
{
public:

	CallWithRTPManagment(
		IN LpHandlePair stack_pair, 
		IN LightweightProcess &facade);

	CallWithRTPManagment(
		IN LpHandlePair _stackPair, 
		IN int call_handle,
		IN CnxInfo offered_media,
		IN LightweightProcess &facade);

	virtual ~CallWithRTPManagment(void);

	IxApiErrorCode MakeCall(
		IN const wstring &destination_uri);

	IxApiErrorCode PlayFile(
		IN const wstring &file_name);

	IxApiErrorCode AcceptCall();

private:

	CcuRtpSession _callerRtpSession;

	CcuRtpSession _imsRtpSession;

	ImsSession _imsSession;

};
