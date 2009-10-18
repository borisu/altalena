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
#include "CallWithRtpManagement.h"

using namespace boost;

namespace ivrworx
{
	CallWithRtpManagement::CallWithRtpManagement(
		IN Configuration &conf,
		IN ScopedForking &forking)
		:Call(forking),
		_conf(conf),
		_mrcpEnabled(FALSE),
		_rtspEnabled(FALSE),
		_imsSession(forking),
		_bridgeState(BRIDGE_STATE_NONE),
		_mrcpSession(forking)
	{
		_mrcpEnabled = _conf.GetBool("mrcp_enabled");
		_rtspEnabled = _conf.GetBool("rtsp_enabled");
		
	}

	CallWithRtpManagement::CallWithRtpManagement(
		IN Configuration &conf,
		IN ScopedForking &forking,
		IN shared_ptr<MsgCallOfferedReq> offered_msg)
		:Call(forking,offered_msg),
		_conf(conf),
		_mrcpEnabled(FALSE),
		_rtspEnabled(FALSE),
		_origOffereReq(offered_msg),
		_imsSession(forking),
		_bridgeState(BRIDGE_STATE_NONE),
		_mrcpSession(forking)
	{
		_mrcpEnabled = _conf.GetBool("mrcp_enabled");
		_rtspEnabled = _conf.GetBool("rtsp_enabled");
		
		RemoteMedia(offered_msg->remote_media);
	}

	ApiErrorCode 
	CallWithRtpManagement::AcceptInitialOffer()
	{
		FUNCTRACKER;

		if (_callState != CALL_STATE_INITIAL_OFFERED)
		{
			LogDebug("CallWithDirectRtp::AcceptInitialOffer wrong call state:" << _callState << ", iwh:" << _stackCallHandle);
			return API_WRONG_STATE;
		}
		
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


		res = _callerRtpSession.Allocate(_origOffereReq->remote_media);
		if (IW_FAILURE(res))
		{
			RejectCall();
			return res;

		}

		LogDebug("CallWithDirectRtp::AcceptInitialOffer caller connection:" << _callerRtpSession.GetLocalInfo());

		if (_rtspEnabled == TRUE)
		{
			res = _imsRtpSession.Allocate();
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;

			}

			LogDebug("CallWithDirectRtp::AcceptInitialOffer ims connection:" << _imsRtpSession.GetLocalInfo());

		}
	

		if (_mrcpEnabled)
		{
			res = _mrcpRtpSession.Allocate();
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;

			}

			LogDebug("CallWithDirectRtp::AcceptInitialOffer mrcp connection:" << _mrcpRtpSession.GetLocalInfo());
		}
		
		
		
		IX_PROFILE_CODE(res = Call::AcceptInitialOffer(
			_callerRtpSession.GetLocalInfo(),
			accepted_media_formats,
			speech_media_format));

		return res;

	}

	ApiErrorCode 
	CallWithRtpManagement::PlayFile(IN const string &file_name, IN BOOL sync, IN BOOL loop)
	{
		FUNCTRACKER;

		if (!_rtspEnabled)
		{
			return API_FAILURE;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;

		}

		if (_bridgeState != BRIDGE_STATE_IMS)
		{
			_bridgeState = BRIDGE_STATE_IMS;
			_imsRtpSession.Bridge(_callerRtpSession);
		}

		ApiErrorCode res = _imsSession.PlayFile(file_name, sync, loop,TRUE);

		return res;

	}
	ApiErrorCode 
	CallWithRtpManagement::Speak(IN const string &mrcp_body, BOOL sync )
	{
		FUNCTRACKER;

		if (!_rtspEnabled)
		{
			return API_FAILURE;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;

		}

		if (_bridgeState != BRIDGE_STATE_MRCP)
		{
			_bridgeState = BRIDGE_STATE_MRCP;
			_mrcpRtpSession.Bridge(_callerRtpSession);
		}

		ApiErrorCode res = _mrcpSession.Speak(mrcp_body, sync);

		return res;


	}

	ApiErrorCode 
	CallWithRtpManagement::StopPlay()
	{
		FUNCTRACKER;

		if (!_rtspEnabled)
		{
			return API_FAILURE;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}

		ApiErrorCode res = API_SUCCESS;

		switch (_bridgeState)
		{
		case BRIDGE_STATE_IMS:
			{
				res = _imsSession.StopPlay();
				break;
			}
		case BRIDGE_STATE_MRCP:
			{
				res = _mrcpSession.StopSpeak();
				break;
			}
		default:
			{

			}
		}

	
		return res;


	}


	ApiErrorCode 
	CallWithRtpManagement::SendRfc2833Dtmf(IN char dtmf)
	{
		FUNCTRACKER;

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}


		ApiErrorCode res = _imsSession.SendDtmf(dtmf);
		return res;

	}

	void 
	CallWithRtpManagement::UponCallTerminated(IwMessagePtr ptr)
	{
		_imsSession.InterruptWithHangup();

		Call::UponCallTerminated(ptr);
	}

	ApiErrorCode 
	CallWithRtpManagement::MakeCall(IN const string &destination_uri)
	{
		
		FUNCTRACKER;
		if (_callState != CALL_STATE_UNKNOWN)
		{
			return API_WRONG_STATE;
		}

		// allocate dummy session to save bind time for future
		ApiErrorCode res = API_SUCCESS;
		IX_PROFILE_CODE(res = _imsSession.Allocate());
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
		IX_PROFILE_CODE(res = _imsSession.ModifyConnection(_remoteMedia ,_acceptedSpeechFormat));
		if (IW_FAILURE(res))
		{
			Call::HangupCall();
			return res;
		};

		return API_SUCCESS;

	}

	CallWithRtpManagement::~CallWithRtpManagement(void)
	{
		FUNCTRACKER;

		_imsSession.TearDown();
	}

}

