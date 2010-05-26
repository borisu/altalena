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

namespace ivrworx
{
	class CallWithRtpManagement 
	{
	public:

		CallWithRtpManagement(
			IN Configuration &conf,
			IN ScopedForking &forking,
			IN MediaCallSessionPtr media_session);

		CallWithRtpManagement(
			IN Configuration &conf,
			IN ScopedForking &forking,
			IN MediaCallSessionPtr media_session,
			IN shared_ptr<MsgCallOfferedReq> offered_msg);

		virtual ~CallWithRtpManagement(void);

		virtual ApiErrorCode Init();

		virtual ApiErrorCode AcceptInitialOffer();

		virtual ApiErrorCode Speak(IN const string &mrcp_body, BOOL sync = FALSE );

		virtual ApiErrorCode PlayFile(IN const string &file_name, IN BOOL sync = FALSE, IN BOOL loop = FALSE);

		virtual ApiErrorCode StopPlay();

		virtual ApiErrorCode StopSpeak();

		virtual ApiErrorCode MakeCall(IN const string &destination_uri);

		virtual ApiErrorCode RtspSetup(IN const string &rtsp_url);

		virtual ApiErrorCode RtspPlay(IN double start_time = 0.0, IN double duration = 0.0, IN float scale = 1.0f);

		virtual ApiErrorCode RtspPause();

		virtual ApiErrorCode RtspTearDown();

		virtual int	StackCallHandle() const;

		virtual ApiErrorCode BlindXfer(IN const string &destination_uri);

		virtual ApiErrorCode WaitForDtmf(OUT string &signal, IN const Time timeout);

		virtual void WaitTillHangup();

		virtual const string& Dnis();

		virtual const string& Ani();

		virtual ApiErrorCode HangupCall();

		virtual void CleanDtmfBuffer();

		virtual const MediaFormat& AcceptedSpeechCodec();

		virtual void EnableMediaFormat(IN const MediaFormat& media_format);

		virtual ApiErrorCode SendInfo(const string &body, const string &type);
	
	protected:

		virtual void UponCallTerminated(IwMessagePtr ptr);

	private:

		Configuration &_conf;

		shared_ptr<RtpProxySession> _callerRtpSession;

		// 
		// live555rtsp
		//
		BOOL								_rtspEnabled;
		shared_ptr<Live555RtspSession>		_rtspSession;
		shared_ptr<RtpProxySession>			_rtspRtpSession;


		//
		// m2ims
		//
		BOOL							_imsEnabled;
		shared_ptr<StreamingSession>	_imsSession;
		shared_ptr<RtpProxySession>		_imsRtpSession;

		//
		// unimrcp
		//
		BOOL							_mrcpEnabled;
		shared_ptr<MrcpSession>			_mrcpSession;
		shared_ptr<RtpProxySession>		_mrcpRtpSession;

	
		shared_ptr<MsgCallOfferedReq>		_origOffereReq;

		MediaCallSessionPtr					_callerMediaCall;

		csp::Time _ringTimeout;

	};

	typedef 
    shared_ptr<CallWithRtpManagement> CallWithRtpManagementPtr;


}
