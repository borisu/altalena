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
		_rtspSession(forking),
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
		_rtspSession(forking),
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

		LogDebug("CallWithDirectRtp::AcceptInitialOffer caller connection:" << _callerRtpSession.LocalCnxInfo());

		if (_rtspEnabled == TRUE)
		{
			res = _rtspRtpSession.Allocate();
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;
			}

			res = _rtspSession.Allocate(
				_rtspRtpSession.LocalCnxInfo(),
				RemoteMedia(),
				speech_media_format);
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;
			}

			LogDebug("CallWithDirectRtp::AcceptInitialOffer ims connection:" << _rtspRtpSession.LocalCnxInfo());

		}
	

		if (_mrcpEnabled)
		{
			res = _mrcpRtpSession.Allocate();
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;

			}

			res = _mrcpSession.Allocate(_mrcpRtpSession.LocalCnxInfo(),speech_media_format);
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;

			}

			LogDebug("CallWithDirectRtp::AcceptInitialOffer mrcp connection:" << _mrcpRtpSession.LocalCnxInfo());
		}
		
		
		
		IX_PROFILE_CODE(res = Call::AcceptInitialOffer(
			_callerRtpSession.LocalCnxInfo(),
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
			LogDebug("CallWithRtpManagement::PlayFile - Media streaming disabled for iwh:" << _stackCallHandle )
			return API_FAILURE;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;

		}

		if (_bridgeState != BRIDGE_STATE_IMS)
		{
			_bridgeState = BRIDGE_STATE_IMS;
			_rtspRtpSession.Bridge(_callerRtpSession);
		}

		ApiErrorCode res = _rtspSession.PlayFile(file_name, sync, loop);

		return res;

	}
	ApiErrorCode 
	CallWithRtpManagement::Speak(IN const string &mrcp_body, BOOL sync )
	{
		FUNCTRACKER;

		if (!_mrcpEnabled)
		{
			return API_FEATURE_DISABLED;
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
	CallWithRtpManagement::StopSpeak()
	{
		FUNCTRACKER;

		if (!_mrcpEnabled)
		{
			return API_FEATURE_DISABLED;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}

		ApiErrorCode res = _mrcpSession.StopSpeak();

		return res;

	}

	ApiErrorCode 
	CallWithRtpManagement::StopPlay()
	{
		FUNCTRACKER;

		if (!_rtspEnabled)
		{
			return API_FEATURE_DISABLED;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}

		ApiErrorCode res =  _rtspSession.StopPlay();

		return res;


	}


	void 
	CallWithRtpManagement::UponCallTerminated(IwMessagePtr ptr)
	{
		_rtspSession.InterruptWithHangup();

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

		ApiErrorCode res = API_SUCCESS;

		//
		// Allocate rtp connection of the caller.
		//
		res = _callerRtpSession.Allocate();
		if (IW_FAILURE(res))
		{
			return res;
		}
		LogDebug("CallWithRtpManagement::MakeCall - caller rtp = rtph:" << _callerRtpSession.RtpHandle() 
			<< ", lci:" << _callerRtpSession.LocalCnxInfo());

		//
		// Allocate rtp connection for the media streamer 
		//
		if (_rtspEnabled)
		{
			//
			// rtp
			//
			res = _rtspRtpSession.Allocate();
			if (IW_FAILURE(res))
			{
				return res;
			}
			
			//
			// streamer session
			//
			res = _rtspSession.Allocate(_rtspRtpSession.LocalCnxInfo());
			if (IW_FAILURE(res))
			{
				return res;
			};
			LogDebug("CallWithRtpManagement::MakeCall - streaming session = imsh:" << _rtspSession.SessionHandle()
				<< ", rtsph:" << _rtspRtpSession.RtpHandle()
				<< ", lci:" << _rtspRtpSession.LocalCnxInfo());
		};

		//
		// Allocate rtp connection for speech generator
		//
		if (_mrcpEnabled)
		{
			res = _mrcpRtpSession.Allocate();
			if (IW_FAILURE(res))
			{
				return res;
			}
		};
		
		
		res = Call::MakeCall(destination_uri,_callerRtpSession.LocalCnxInfo());
		if (IW_FAILURE(res))
		{
			return res;
		};

		LogDebug("CallWithRtpManagement::MakeCall - caller remote rtp = iwh:" <<  _stackCallHandle 
			<< ", rci:" << RemoteMedia() 
			<< " codec:" << _acceptedSpeechFormat);


		_callerRtpSession.Modify(RemoteMedia());


#pragma  TODO("Pooling of mrcp connections")

		//
		// allocate mrcp session
		//
		if (_mrcpEnabled)
		{
			res = _mrcpSession.Allocate(_mrcpRtpSession.LocalCnxInfo(),_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				HangupCall();
				return res;
			}

		}

		
		if (_rtspEnabled)
		{
			res = _rtspSession.ModifyConnection(_rtspRtpSession.LocalCnxInfo() ,_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				Call::HangupCall();
				return res;
			};
		}
		
		return API_SUCCESS;

	}

	CallWithRtpManagement::~CallWithRtpManagement(void)
	{
		FUNCTRACKER;

		_rtspSession.TearDown();
	}

}

