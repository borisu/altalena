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
	class CallWithRtpManagement :
		public SipCall
	{
		

	public:

		CallWithRtpManagement(
			IN Configuration &conf,
			IN ScopedForking &forking);

		CallWithRtpManagement(
			IN Configuration &conf,
			IN ScopedForking &forking,
			IN shared_ptr<MsgCallOfferedReq> offered_msg);

		virtual ~CallWithRtpManagement(void);

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

		

	protected:

		virtual void UponCallTerminated(IwMessagePtr ptr);

	private:

		Configuration &_conf;

		RtpProxySession _callerRtpSession;

		// 
		// rtsp
		//
		BOOL			_rtspEnabled;
		RtspSession		_rtspSession;
		RtpProxySession _rtspRtpSession;


		//
		// ims
		//
		BOOL			_imsEnabled;
		ImsSession		_imsSession;
		RtpProxySession _imsRtpSession;

		//
		// mrcp
		//
		BOOL			_mrcpEnabled;
		MrcpSession		_mrcpSession;
		RtpProxySession _mrcpRtpSession;

	
		shared_ptr<MsgCallOfferedReq> _origOffereReq;

		csp::Time _ringTimeout;

	};

	typedef 
    shared_ptr<CallWithRtpManagement> CallWithRtpManagementPtr;


}
