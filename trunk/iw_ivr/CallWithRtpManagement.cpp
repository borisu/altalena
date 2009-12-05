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
		_imsEnabled(FALSE),
		_imsSession(forking),
		_mrcpSession(forking),
		_rtspSession(forking,conf),
		_ringTimeout(Seconds(60))
	{
		_mrcpEnabled = _conf.GetBool("mrcp_enabled");
		_imsEnabled  = _conf.GetBool("ims_enabled");
		_rtspEnabled = _conf.GetBool("rtsp_enabled");
		_ringTimeout = Seconds(_conf.GetInt("ring_timeout"));
		
	}

	CallWithRtpManagement::CallWithRtpManagement(
		IN Configuration &conf,
		IN ScopedForking &forking,
		IN shared_ptr<MsgCallOfferedReq> offered_msg)
		:Call(forking,offered_msg),
		_conf(conf),
		_mrcpEnabled(FALSE),
		_imsEnabled(FALSE),
		_origOffereReq(offered_msg),
		_imsSession(forking),
		_mrcpSession(forking),
		_rtspSession(forking,conf),
		_ringTimeout(Seconds(60))
	{
		_mrcpEnabled = _conf.GetBool("mrcp_enabled");
		_imsEnabled = _conf.GetBool("ims_enabled");
		_ringTimeout = Seconds(_conf.GetInt("ring_timeout"));
		_rtspEnabled = _conf.GetBool("rtsp_enabled");
		
		RemoteMedia(offered_msg->remote_media);
	}

	ApiErrorCode 
	CallWithRtpManagement::AcceptInitialOffer()
	{
		FUNCTRACKER;

		if (_callState != CALL_STATE_INITIAL_OFFERED)
		{
			LogWarn("CallWithRtpManagement::AcceptInitialOffer -  wrong call state:" << _callState << ", iwh:" << _stackCallHandle);
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


		LogDebug("CallWithRtpManagement::AcceptInitialOffer -  *** allocating caller rtp ***");
		res = _callerRtpSession.Allocate(_origOffereReq->remote_media, speech_media_format);
		if (IW_FAILURE(res))
		{
			RejectCall();
			return res;

		}

		LogDebug("CallWithRtpManagement::AcceptInitialOffer - caller rtp = rtph:" << _callerRtpSession.RtpHandle() 
			<< ", lci:" << _callerRtpSession.LocalCnxInfo());


		//
		// Allocate rtsp connection
		//
		if (_rtspEnabled)
		{
			//
			// rtp
			//
			LogDebug("CallWithRtpManagement::AcceptInitialOffer - *** allocating rtsp session ***");
			res = _rtspRtpSession.Allocate(CnxInfo::UNKNOWN,_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				return res;
			}
		}

	

		if (_imsEnabled == TRUE)
		{
			//
			// rtp
			//
			LogDebug("CallWithRtpManagement::AcceptInitialOffer - *** allocating streamer rtp and session ***");
			res = _imsRtpSession.Allocate(CnxInfo::UNKNOWN,speech_media_format);
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;
			}

			//
			// streamer session
			//
			res = _imsSession.Allocate(
				_imsRtpSession.LocalCnxInfo(),
				speech_media_format);
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;
			}

			LogDebug("CallWithRtpManagement::AcceptInitialOffer - streaming info = imsh:" << _imsSession.SessionHandle()
				<< ", associated imsh:" << _imsRtpSession.RtpHandle());

		}
	

		//
		// allocate mrcp session
		//
		if (_mrcpEnabled)
		{
			LogDebug("CallWithRtpManagement::AcceptInitialOffer *** allocating mrcp rtp connection and session ***");
			res = _mrcpRtpSession.Allocate(CnxInfo::UNKNOWN,speech_media_format);
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;

			}

			res = _mrcpSession.Allocate(
				_mrcpRtpSession.LocalCnxInfo(),
				speech_media_format);
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;

			}

			LogDebug("CallWithRtpManagement::AcceptInitialOffer - mrcp info = mrcph:" << _mrcpSession.SessionHandle()
				<< ", associated imsh:" << _mrcpRtpSession.RtpHandle());
		}
		
		
		
		res = Call::AcceptInitialOffer(
			_callerRtpSession.LocalCnxInfo(),
			accepted_media_formats,
			speech_media_format);


		if (IW_SUCCESS(res))
		{
			LogDebug("CallWithRtpManagement::AcceptInitialOffer - ims("  <<  _imsSession.SessionHandle() << ")---> rtp(" << _imsRtpSession.RtpHandle() << ") + ");
			LogDebug("CallWithRtpManagement::AcceptInitialOffer - rtp("  <<  _callerRtpSession.RtpHandle()  << ")---> caller(" <<  StackCallHandle() << ") " << RemoteMedia());
			LogDebug("CallWithRtpManagement::AcceptInitialOffer - mrcp(" <<  _mrcpSession.SessionHandle() << ")---> rtp(" << _mrcpRtpSession.RtpHandle() << ") + ");
			LogDebug("CallWithRtpManagement::AcceptInitialOffer - rtsp(" <<  _rtspSession.SessionHandle() << ")---> rtp(" << _rtspRtpSession.RtpHandle() << ") + ");
		}
		

		return res;

	}

	ApiErrorCode 
	CallWithRtpManagement::PlayFile(IN const string &file_name, IN BOOL sync, IN BOOL loop)
	{
		FUNCTRACKER;

		if (!_imsEnabled)
		{
			LogDebug("CallWithRtpManagement::PlayFile - Media streaming disabled for iwh:" << _stackCallHandle )
			return API_FAILURE;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;

		}

		_imsSession.StopPlay();
		
		ApiErrorCode res = _imsRtpSession.Bridge(_callerRtpSession);
		if (IW_FAILURE(res))
		{
			return res;
		}
	
		

		res = _imsSession.PlayFile(file_name, sync, loop);

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

		_mrcpSession.StopSpeak();

		ApiErrorCode res = _mrcpRtpSession.Bridge(_callerRtpSession);
		if (IW_FAILURE(res))
		{
			return res;
		}

		res = _mrcpSession.Speak(mrcp_body, sync);
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

		if (!_imsEnabled)
		{
			return API_FEATURE_DISABLED;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}

		ApiErrorCode res =  _imsSession.StopPlay();

		return res;


	}


	void 
	CallWithRtpManagement::UponCallTerminated(IwMessagePtr ptr)
	{
		_imsSession.InterruptWithHangup();
		_mrcpSession.InterruptWithHangup();

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

		LogDebug("CallWithRtpManagement::MakeCall *** allocating caller rtp ***");

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

		
		
		LogDebug("CallWithRtpManagement::MakeCall *** calling ***");
		res = Call::MakeCall(destination_uri,_callerRtpSession.LocalCnxInfo(),_ringTimeout);
		if (IW_FAILURE(res))
		{
			return res;
		};

		res = _callerRtpSession.Modify(RemoteMedia(),_acceptedSpeechFormat);
		if (IW_FAILURE(res))
		{
			return res;
		};

		LogDebug("CallWithRtpManagement::MakeCall - caller remote rtp = iwh:" <<  _stackCallHandle 
			<< ", rci:" << RemoteMedia() 
			<< " codec:" << _acceptedSpeechFormat);

		//
		// Allocate rtsp connection
		//
		if (_rtspEnabled)
		{
			//
			// rtp
			//
			LogDebug("CallWithRtpManagement::MakeCall *** allocating rtsp session ***");
			res = _rtspRtpSession.Allocate(CnxInfo::UNKNOWN,_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				return res;
			}
		}



		//
		// Allocate ims session
		//
		if (_imsEnabled)
		{
			//
			// rtp
			//
			LogDebug("CallWithRtpManagement::MakeCall *** allocating streamer rtp and session ***");
			res = _imsRtpSession.Allocate(CnxInfo::UNKNOWN,_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				return res;
			}

			//
			// streamer session
			//
			res = _imsSession.Allocate(
				_imsRtpSession.LocalCnxInfo(), 
				_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				return res;
			};

			LogDebug("CallWithRtpManagement::MakeCall - streaming info = imsh:" << _imsSession.SessionHandle()
				<< ", associated imsh:" << _imsRtpSession.RtpHandle());
		};



		//
		// allocate mrcp session
		//
		if (_mrcpEnabled)
		{
			LogDebug("CallWithRtpManagement::MakeCall *** allocating mrcp rtp connection and session ***");
			res = _mrcpRtpSession.Allocate(CnxInfo::UNKNOWN,_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				return res;
			}

			res = _mrcpSession.Allocate(_mrcpRtpSession.LocalCnxInfo(),_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				return res;
			}

			LogDebug("CallWithRtpManagement::MakeCall - mrcp info = mrcph:" << _mrcpSession.SessionHandle()
				<< ", associated imsh:" << _mrcpRtpSession.RtpHandle());
			

		}


		LogDebug("CallWithRtpManagement::MakeCall - ims("  <<  _imsSession.SessionHandle() << ")---> rtp(" << _imsRtpSession.RtpHandle() << ") + ");
		LogDebug("CallWithRtpManagement::MakeCall - rtp("  <<  _callerRtpSession.RtpHandle()  << ")---> caller(" <<  StackCallHandle() << ") " << RemoteMedia());
		LogDebug("CallWithRtpManagement::MakeCall - mrcp(" <<  _mrcpSession.SessionHandle() << ")---> rtp(" << _mrcpRtpSession.RtpHandle() << ") + ");
		LogDebug("CallWithRtpManagement::MakeCall - rtsp(" <<  _rtspSession.SessionHandle() << ")---> rtp(" << _rtspRtpSession.RtpHandle() << ") + ");
		
		return API_SUCCESS;

	}

	ApiErrorCode
	CallWithRtpManagement::RtspSetup(IN const string &rtsp_url)
	{
		if (!_rtspEnabled)
		{
			return API_FEATURE_DISABLED;
		}

		if (_acceptedSpeechFormat == MediaFormat::UNKNOWN)
		{
			return API_WRONG_STATE;
		}

		LogDebug("CallWithRtpManagement::RtspSetup - rtsp_url:" << rtsp_url << ", media format:" << _acceptedSpeechFormat);

		ApiErrorCode res = _rtspSession.Setup(rtsp_url,_acceptedSpeechFormat);
		return res;

	}

	ApiErrorCode
	CallWithRtpManagement::RtspPlay(IN double start_time /* = 0.0 */, IN double end_time /* = 0.0 */, IN float scale /* = 1.0f */)
	{
		if (!_rtspEnabled)
		{
			return API_FEATURE_DISABLED;
		}

		if (_acceptedSpeechFormat == MediaFormat::UNKNOWN)
		{
			return API_WRONG_STATE;
		}

		LogDebug("CallWithRtpManagement::RtspPlay - start_time:" << start_time << ", end_time:" << end_time << ", scale:" << scale);

		ApiErrorCode res = _rtspSession.Play(start_time,end_time,scale);
		return res;

	}


	ApiErrorCode
	CallWithRtpManagement::RtspPause()
	{
		if (!_rtspEnabled)
		{
			return API_FEATURE_DISABLED;
		}

		if (_acceptedSpeechFormat == MediaFormat::UNKNOWN)
		{
			return API_WRONG_STATE;
		}

		LogDebug("CallWithRtpManagement::RtspPause");

		ApiErrorCode res = _rtspSession.Pause();
		return res;

	}

	ApiErrorCode
	CallWithRtpManagement::RtspTearDown()
	{
		if (!_rtspEnabled)
		{
			return API_FEATURE_DISABLED;
		}

		if (_acceptedSpeechFormat == MediaFormat::UNKNOWN)
		{
			return API_WRONG_STATE;
		}

		LogDebug("CallWithRtpManagement::RtspTearDown");

		ApiErrorCode res = _rtspSession.TearDown();
		return res;

	}

	CallWithRtpManagement::~CallWithRtpManagement(void)
	{
		FUNCTRACKER;

		//
		// destroy rtp connections
		//
		_imsSession.TearDown();
		_mrcpSession.TearDown();
		_rtspSession.TearDown();
		_callerRtpSession.TearDown();

		//
		// destroy sessions themselves
		//
		_mrcpRtpSession.TearDown();
		_imsRtpSession.TearDown();
		_rtspSession.TearDown();

	}

}

