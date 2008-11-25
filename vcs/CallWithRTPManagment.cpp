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
#include "CallWithRTPManagment.h"

CallWithRTPManagment::CallWithRTPManagment(
	IN LpHandlePair stack_pair,
	IN LightweightProcess &facade)
:Call(stack_pair,facade),
_callerRtpSession(facade)
{
}

CallWithRTPManagment::~CallWithRTPManagment(void)
{
	FUNCTRACKER;
	_callerRtpSession.CloseRTPConnection();
}

CcuApiErrorCode
CallWithRTPManagment::MakeCall(IN const wstring &destination_uri)
{
	FUNCTRACKER;

	CcuApiErrorCode res = _callerRtpSession.AllocateRTPConnection();
	if (CCU_FAILURE(res))
	{
		LogWarn("Error allocating RTP connection res=[" << res << "]");
		return res;
	}

	res  = Call::MakeCall(destination_uri,_callerRtpSession.LocalMediaData());
	if (CCU_FAILURE(res))
	{
		LogWarn("Error Making call to destination_uri=[" << destination_uri << "],  res=[" << res << "]");
		return res;
	}

	_callerRtpSession.ModifyRTPConnection(RemoteMedia());

	return CCU_API_SUCCESS;
}

CcuApiErrorCode
CallWithRTPManagment::PlayFile(IN const wstring &file_name)
{
	FUNCTRACKER;

	ImsSession ims_session(_parentProcess);

	CcuRtpSession ims_rtp_session(_parentProcess);

	CcuApiErrorCode res = ims_rtp_session.AllocateRTPConnection();
	if (CCU_FAILURE(res))
	{
		LogWarn("Error allocating RTP connection res=[" << res << "]");
		return res;
	}

	res = _callerRtpSession.BridgeRTPConnection(ims_rtp_session);
	if (CCU_FAILURE(res))
	{
		LogWarn("Error bridging RTP connection res=[" << res << "]");
		return res;
	}

	res = ims_session.PlayFile(ims_rtp_session.LocalMediaData(),file_name);
	if (CCU_FAILURE(res))
	{
		LogWarn("Error Playing file=[" << file_name <<"] to=[" << ims_rtp_session.LocalMediaData() << "], res=[" << res << "]");
		return res;
	}

	return CCU_API_SUCCESS;

}
