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
#include "RtspSession.h"


namespace ivrworx
{
	RtspSession::RtspSession(ScopedForking &forking, ConfigurationPtr conf,HandleId rtspServiceHandleId):
	_forking(forking),
	_rtspHandle(IW_UNDEFINED),
	_conf(conf),
	_rtspServiceHandleId(rtspServiceHandleId)
	{
		
		
	}

	RtspSession::~RtspSession(void)
	{
		FUNCTRACKER;

		
	}

	RtspHandle
	RtspSession::SessionHandle()
	{
		return _rtspHandle;
	}


	ApiErrorCode 
	RtspSession::Setup(IN const string &rtsp_url, IN const AbstractOffer &offer)
	{

		FUNCTRACKER;

		LogDebug("RtspSession::Setup - rtsp_url:" << rtsp_url << ", rtsh:" <<  _rtspHandle);

		MsgRtspSetupSessionReq *msg = new MsgRtspSetupSessionReq();
		msg->offer = offer;
		
		IwMessagePtr response = NULL_MSG;
		ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
			_rtspServiceHandleId,
			IwMessagePtr(msg),
			response,
			Seconds(10),
			"RTSP Setup TXN");

		if (res != API_SUCCESS)
		{
			LogWarn("RtspSession::Setup - Error allocating RTSP session" << res);
			return res;
		}

		switch (response->message_id)
		{
		case MSG_RTSP_SETUP_SESSION_ACK:
			{


				shared_ptr<MsgRtspSetupSessionAck> ack = 
					shared_polymorphic_cast<MsgRtspSetupSessionAck>(response);

				_rtspHandle	= ack->rtsp_handle;

				LogDebug("RtspSession::Setup - RTSP session allocated successfully, rtsph:" << _rtspHandle);

				break;

			}
		case MSG_RTSP_SETUP_SESSION_NACK:
			{
				LogDebug("Error allocating Rtsp session.");
				res = API_SERVER_FAILURE;
				break;
			}
		default:
			{
				throw;
			}
		}

		return res;

	}

	ApiErrorCode 
	RtspSession::Play(double start_time /* = 0.0 */, double duration /* = 0.0  */, double scale /* = 1.0 */)
	{
		LogDebug("RtspSession::Play - start_time:" << start_time  << ", duration:"  << duration << ", scale: scale" << ", rtsh:" <<  _rtspHandle);

		if (_rtspHandle == IW_UNDEFINED)
		{
			return API_FAILURE;
		}

		MsgRtspPlayReq *msg = new MsgRtspPlayReq();
		msg->start_time = start_time;
		msg->duration = duration;
		msg->scale =  scale;

		IwMessagePtr response = NULL_MSG;
		ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
			_rtspServiceHandleId,
			IwMessagePtr(msg),
			response,
			MilliSeconds(GetCurrRunningContext()->TransactionTimeout()),
			"RTSP Play TXN");

		if (res != API_SUCCESS)
		{
			LogWarn("RtspSession::Play - Error playing RTSP session" << res);
			return res;
		}

		switch (response->message_id)
		{
		case MSG_RTSP_PLAY_ACK:
			{
				break;
			}
		case MSG_RTSP_SETUP_SESSION_NACK:
			{
				LogWarn("RtspSession::Play - Error allocating Rtsp session.");
				res = API_SERVER_FAILURE;
				break;
			}
		default:
			{
				throw;
			}
		}

		return res;
	}

	ApiErrorCode 
	RtspSession::Pause()
	{
		LogDebug("RtspSession::Pause rtsh:" <<  _rtspHandle);

		if (_rtspHandle == IW_UNDEFINED)
		{
			return API_FAILURE;
		}

		MsgRtspPauseReq *msg = new MsgRtspPauseReq();
		
		IwMessagePtr response = NULL_MSG;
		ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
			_rtspServiceHandleId,
			IwMessagePtr(msg),
			response,
			MilliSeconds(GetCurrRunningContext()->TransactionTimeout()),
			"RTSP Pause TXN");

		if (res != API_SUCCESS)
		{
			LogWarn("RtspSession::Pause - Error playing RTSP session" << res);
			return res;
		}

		switch (response->message_id)
		{
		case MSG_RTSP_PAUSE_ACK:
			{
				break;
			}
		case MSG_RTSP_PAUSE_NACK:
			{
				LogWarn("RtspSession::Pause - Error allocating Rtsp session.");
				res = API_SERVER_FAILURE;
				break;
			}
		default:
			{
				throw;
			}
		}

		return res;
	}

	ApiErrorCode 
	RtspSession::TearDown()
	{
		FUNCTRACKER;

		LogDebug("RtspSession::TearDown rtsph:" <<  _rtspHandle);

		if (_rtspHandle == IW_UNDEFINED)
		{
			return API_FAILURE;
		}

		_rtspHandle = IW_UNDEFINED;

		GetCurrRunningContext()->SendMessage(_rtspServiceHandleId, 
			IwMessagePtr(new MsgRtspTearDownReq()));

		return API_SUCCESS;

	}


}
