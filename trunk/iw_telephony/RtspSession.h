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

#include "Telephony.h"

namespace ivrworx
{
	enum RtspEvents
	{
		MSG_RTSP_SETUP_SESSION_REQ = RTSP_MSG_BASE,
		MSG_RTSP_SETUP_SESSION_ACK,
		MSG_RTSP_SETUP_SESSION_NACK,
		MSG_RTSP_PLAY_REQ ,
		MSG_RTSP_PLAY_ACK,
		MSG_RTSP_PLAY_NACK,
		MSG_RTSP_PAUSE_REQ,
		MSG_RTSP_PAUSE_ACK,
		MSG_RTSP_PAUSE_NACK,
		MSG_RTSP_TEARDOWN_REQ,
		MSG_RTSP_TEARDOWN_ACK,
		MSG_RTSP_TEARDOWN_NACK
	};

	typedef int
	RtspHandle; 

	class IW_TELEPHONY_API  RtspMixin
	{
	public:

		RtspMixin():
		  rtsp_handle(IW_UNDEFINED),
		  start_time(0.0),
		  duration(0.0),
		  scale(1.0)
		{

		}

		RtspHandle rtsp_handle;

		string request_url;

		AbstractOffer offer;

		double start_time;

		double duration;

		double scale;

		

	};

//--- Setup session

	class IW_TELEPHONY_API  MsgRtspSetupSessionReq
		: public RtspMixin,
		public MsgRequest
	{
	public:
		MsgRtspSetupSessionReq(): 
			MsgRequest(MSG_RTSP_SETUP_SESSION_REQ,NAME(MSG_RTSP_SETUP_SESSION_REQ)){}
	};

	class IW_TELEPHONY_API  MsgRtspSetupSessionAck
		: public RtspMixin, public MsgRequest
	{
	public:
		MsgRtspSetupSessionAck(): 
		  MsgRequest(MSG_RTSP_SETUP_SESSION_ACK,NAME(MSG_RTSP_SETUP_SESSION_ACK)){}
	};


	class IW_TELEPHONY_API MsgRtspSetupSessionNack
		: public RtspMixin,public MsgRequest
	{
	public:
		MsgRtspSetupSessionNack(): 
		  MsgRequest(MSG_RTSP_SETUP_SESSION_NACK,NAME(MSG_RTSP_SETUP_SESSION_NACK)){}
	};

//--- Play

	class IW_TELEPHONY_API MsgRtspPlayReq
		: public RtspMixin,
		public MsgRequest
	{
	public:
		MsgRtspPlayReq(): 
		  MsgRequest(MSG_RTSP_PLAY_REQ,NAME(MSG_RTSP_PLAY_REQ)){}
	};

	class IW_TELEPHONY_API MsgRtspPlayAck
		: public RtspMixin, public MsgRequest
	{
	public:
		MsgRtspPlayAck(): 
		  MsgRequest(MSG_RTSP_PLAY_ACK,NAME(MSG_RTSP_PLAY_ACK)){}
	};


	class IW_TELEPHONY_API MsgRtspPlayNack
		: public RtspMixin,public MsgRequest
	{
	public:
		MsgRtspPlayNack(): 
		  MsgRequest(MSG_RTSP_PLAY_NACK,NAME(MSG_RTSP_PLAY_NACK)){}
	};

//--- Pause

	class IW_TELEPHONY_API MsgRtspPauseReq
		: public RtspMixin,
		public MsgRequest
	{
	public:
		MsgRtspPauseReq(): 
		  MsgRequest(MSG_RTSP_PAUSE_REQ,NAME(MSG_RTSP_PAUSE_REQ)){}
	};

	class IW_TELEPHONY_API MsgRtspPauseAck
		: public RtspMixin, public MsgRequest
	{
	public:
		MsgRtspPauseAck(): 
		  MsgRequest(MSG_RTSP_PAUSE_ACK,NAME(MSG_RTSP_PAUSE_ACK)){}
	};

	class IW_TELEPHONY_API MsgRtspPauseNack
		: public RtspMixin, public MsgRequest
	{
	public:
		MsgRtspPauseNack(): 
		  MsgRequest(MSG_RTSP_PAUSE_NACK,NAME(MSG_RTSP_PAUSE_NACK)){}
	};

// -- Tear Down

	class IW_TELEPHONY_API MsgRtspTearDownReq
		: public RtspMixin,public MsgRequest
	{
	public:
		MsgRtspTearDownReq(): 
		  MsgRequest(MSG_RTSP_TEARDOWN_REQ,NAME(MSG_RTSP_TEARDOWN_REQ)){}
	};

	class IW_TELEPHONY_API MsgRtspTearDownAck
		: public RtspMixin, public MsgRequest
	{
	public:
		MsgRtspTearDownAck(): 
		  MsgRequest(MSG_RTSP_TEARDOWN_ACK,NAME(MSG_RTSP_TEARDOWN_ACK)){}
	};


	class IW_TELEPHONY_API MsgRtspTearDownNack
		: public RtspMixin,public MsgRequest
	{
	public:
		MsgRtspTearDownNack(): 
		  MsgRequest(MSG_RTSP_TEARDOWN_NACK,NAME(MSG_RTSP_TEARDOWN_NACK)){}
	};


	class IW_TELEPHONY_API RtspSession
	{
	public:

		RtspSession(ScopedForking &forking, ConfigurationPtr conf,HandleId rtspHandleId);

		virtual ~RtspSession(void);
	
		virtual ApiErrorCode Setup(IN const string &rtsp_url, IN const AbstractOffer &offer);

		virtual ApiErrorCode Play(double start_time = 0.0, double duration = 0.0 , double scale = 1.0);

		virtual ApiErrorCode Pause();

		virtual ApiErrorCode TearDown();

		virtual RtspHandle SessionHandle();

	protected:

		RtspHandle _rtspHandle;

		HandleId _rtspServiceHandleId;

		ScopedForking &_forking;

		ConfigurationPtr _conf;

	};

}
