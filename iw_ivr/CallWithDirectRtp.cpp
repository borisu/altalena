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
		IN ScopedForking &forking)
		:Call(forking),
		_imsSession(forking)
	{

		
	}

	CallWithDirectRtp::CallWithDirectRtp(
		IN ScopedForking &forking,
		IN shared_ptr<MsgCallOfferedReq> offered_msg)
		:Call(forking,offered_msg),
		_origOffereReq(offered_msg),
		_imsSession(forking)
	{
		
		RemoteMedia(offered_msg->remote_media);
	}

	ApiErrorCode 
	CallWithDirectRtp::AcceptInitialOffer()
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		MediaFormatsList accepted_media_formats;
		MediaFormat speech_media_format = MediaFormat::PCMU;

		ApiErrorCode res = this->NegotiateMediaFormats(
			_origOffereReq->offered_codecs, 
			accepted_media_formats, 
			speech_media_format);
		if (IW_FAILURE(res))
		{
			RejectCall();
			return res;
		};

		res = _imsSession.AllocateIMSConnection(RemoteMedia(),speech_media_format);
		if (IW_FAILURE(res))
		{
			RejectCall();
			return res;
		};

		res = Call::AcceptInitialOffer(
			_imsSession.ImsMediaData(),
			accepted_media_formats,
			speech_media_format);

		return res;

	}

	ApiErrorCode 
	CallWithDirectRtp::PlayFile(IN const string &file_name, IN BOOL sync, IN BOOL loop)
	{
		FUNCTRACKER;

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}

		ApiErrorCode res = _imsSession.PlayFile(file_name, sync, loop,TRUE);

		return res;

	}

	ApiErrorCode 
	CallWithDirectRtp::StopPlay()
	{
		FUNCTRACKER;

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}

		ApiErrorCode res = _imsSession.StopPlay();

		return res;


	}

	ApiErrorCode 
	CallWithDirectRtp::WaitForDtmf(OUT int &dtmf, IN Time timeout)
	{
		FUNCTRACKER;

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}

		ApiErrorCode res = _imsSession.WaitForDtmf(dtmf, timeout);

		return res;

	}


	ApiErrorCode 
	CallWithDirectRtp::SendRfc2833Dtmf(IN char dtmf)
	{
		FUNCTRACKER;

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}


		ApiErrorCode res = _imsSession.SendDtmf(dtmf);
		return res;

	}

	ApiErrorCode 
	CallWithDirectRtp::MakeCall(IN const string &destination_uri)
	{
		
		FUNCTRACKER;
		if (_callState != CALL_STATE_UNKNOWN)
		{
			return API_WRONG_STATE;
		}

		// allocate dummy session to save bind time for future
		ApiErrorCode res = _imsSession.AllocateIMSConnection();
		if (IW_FAILURE(res))
		{
			return res;
		};
		
		res = Call::MakeCall(destination_uri,_imsSession.ImsMediaData());
		if (IW_FAILURE(res))
		{
			return res;
		};

		// allocate dummy session to save bind time for future
		res = _imsSession.ModifyConnection(_remoteMedia ,_acceptedSpeechFormat);
		if (IW_FAILURE(res))
		{
			Call::HangupCall();
			return res;
		};

		return API_SUCCESS;

	}

	CallWithDirectRtp::~CallWithDirectRtp(void)
	{
		FUNCTRACKER;

		_imsSession.TearDown();
	}

}

