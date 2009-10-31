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
		public Call
	{
		enum BRIDGE_STATE
		{
			BRIDGE_STATE_NONE,
			BRIDGE_STATE_IMS,
			BRIDGE_STATE_MRCP
		};

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

	protected:

		virtual void UponCallTerminated(IwMessagePtr ptr);

	private:

		Configuration &_conf;

		RtpProxySession _callerRtpSession;

		//
		// rtsp
		//
		BOOL			_rtspEnabled;
		ImsSession		_rtspSession;
		RtpProxySession _rtspRtpSession;

		//
		// mrcp
		//
		BOOL			_mrcpEnabled;
		RtpProxySession _mrcpRtpSession;
		MrcpSession		_mrcpSession;

	
		shared_ptr<MsgCallOfferedReq> _origOffereReq;
		BRIDGE_STATE _bridgeState;

	};

	typedef 
    shared_ptr<CallWithRtpManagement> CallWithRtpManagementPtr;


}
