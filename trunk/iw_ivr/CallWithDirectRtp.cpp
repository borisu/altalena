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
#include "CallWithDirectRtp.h"

using namespace boost;

namespace ivrworx
{
	CallWithDirectRtp::CallWithDirectRtp(
		IN LpHandlePair stack_pair,
		IN ScopedForking &forking,
		IN shared_ptr<MsgCallOfferedReq> offered_msg)
		:Call(stack_pair,forking,offered_msg),
		_origOffereReq(offered_msg),
		_imsSession(forking)
	{
		
		RemoteMedia(offered_msg->remote_media);
	}

	ApiErrorCode CallWithDirectRtp::AcceptCall()
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		MediaFormatsList accepted_media_formats;
		ApiErrorCode res = this->NegotiateMediaFormats(_origOffereReq->offered_codecs, accepted_media_formats);
		if (IW_FAILURE(res))
		{
			RejectCall();
			return res;
		};

		res = _imsSession.AllocateIMSConnection(RemoteMedia(),_acceptedSpeechFormat);
		if (IW_FAILURE(res))
		{
			RejectCall();
			return res;
		};

		res = Call::AcceptCall(_imsSession.ImsMediaData(),accepted_media_formats);
		return res;

	}

	ApiErrorCode 
	CallWithDirectRtp::PlayFile(IN const string &file_name, IN BOOL sync, IN BOOL loop)
	{
		FUNCTRACKER;

		ApiErrorCode res = _imsSession.PlayFile(file_name, sync, loop,TRUE);

		return res;

	}

	ApiErrorCode 
	CallWithDirectRtp::WaitForDtmf(OUT int &dtmf, IN Time timeout)
	{
		FUNCTRACKER;

		ApiErrorCode res = _imsSession.WaitForDtmf(dtmf, timeout);

		return res;

	}

	CallWithDirectRtp::~CallWithDirectRtp(void)
	{
		FUNCTRACKER;

		_imsSession.TearDown();
	}

}

