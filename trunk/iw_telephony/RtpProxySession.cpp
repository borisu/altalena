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
#include "RtpProxySession.h"


namespace ivrworx
{
	RtpProxySession::RtpProxySession(HandleId handle_id):
	_rtpProxyHandleId(handle_id),
	_handle(IW_UNDEFINED),
	_bridgedHandle(IW_UNDEFINED)
	{
	}

	RtpProxySession::~RtpProxySession(void)
	{
		TearDown();
	}

	ApiErrorCode 
	RtpProxySession::Allocate()
	{
		FUNCTRACKER;

		return Allocate(AbstractOffer());
	}

	ApiErrorCode 
	RtpProxySession::Allocate(const AbstractOffer &remoteOffer)
	{
		FUNCTRACKER;

		if (_handle != IW_UNDEFINED)
		{
			return API_FAILURE;
		}

		DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

		MsgRtpProxyAllocateReq *msg = new MsgRtpProxyAllocateReq();
		msg->offer = remoteOffer;
		
		

		IwMessagePtr response = NULL_MSG;
		ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
			_rtpProxyHandleId,
			IwMessagePtr(msg),
			response,
			MilliSeconds(GetCurrRunningContext()->TransactionTimeout()),
			"Allocate RTP Connection TXN");

		if (res != API_SUCCESS)
		{
			LogWarn("Error allocating rtp connection " << res);
			return res;
		}

		switch (response->message_id)
		{
		case MSG_RTP_PROXY_ACK:
			{
				shared_ptr<MsgRtpProxyAck> ack 
					= dynamic_pointer_cast<MsgRtpProxyAck> (response);

				_handle = ack->rtp_proxy_handle;
				_localOffer = ack->offer;

				LogDebug("RtpProxySession::Allocate allocated rtpid:" << _handle);
				
				return API_SUCCESS;
			}
		
		default:
			{
				LogDebug("RtpProxySession::Allocate failed");
				return API_FAILURE;
			}
		}

	}

	ApiErrorCode 
	RtpProxySession::TearDown()
	{
		FUNCTRACKER;

		if (_handle == IW_UNDEFINED)
		{
			return API_SUCCESS;
		}

		MsgRtpProxyDeallocateReq *req = 
			new MsgRtpProxyDeallocateReq();

		req->rtp_proxy_handle = _handle;

		GetCurrRunningContext()->SendMessage(_rtpProxyHandleId,IwMessagePtr(req));

		_handle = IW_UNDEFINED;
		_bridgedHandle = IW_UNDEFINED;
		
		return API_SUCCESS;
	}

	ApiErrorCode 
	RtpProxySession::Modify(const AbstractOffer &remoteOffer)
	{
		LogDebug("RtpProxySession::Modify  rtph:" << _handle);

		if (_handle == IW_UNDEFINED)
		{
			return API_WRONG_STATE;
		}

		MsgRtpProxyModifyReq *req = 
			new MsgRtpProxyModifyReq();

		req->rtp_proxy_handle = _handle;
		req->offer	  = remoteOffer;
		

		IwMessagePtr response = NULL_MSG;
		GetCurrRunningContext()->DoRequestResponseTransaction(
			_rtpProxyHandleId,
			IwMessagePtr(req),
			response,
			MilliSeconds(GetCurrRunningContext()->TransactionTimeout()),
			"Modify RTP Connection TXN");

		switch (response->message_id)
		{
		case MSG_RTP_PROXY_ACK:
			{
				shared_ptr<MsgRtpProxyAck> ack 
					= dynamic_pointer_cast<MsgRtpProxyAck> (response);

				return API_SUCCESS;
			}

		default:
			{
				return API_FAILURE;
			}
		};

	}

	RtpProxyHandle 
	RtpProxySession::RtpHandle()
	{
		return _handle;
	}

	ApiErrorCode 
	RtpProxySession::Bridge(IN const RtpProxySession &dest)
	{
		FUNCTRACKER;

		LogDebug("RtpProxySession::Bridge src:" << _handle << ", dst:" << dest._handle);

		if (_handle == IW_UNDEFINED)
		{
			return API_WRONG_STATE;
		}

		_bridgedHandle = IW_UNDEFINED;

		DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

		MsgRtpProxyBridgeReq *msg = new MsgRtpProxyBridgeReq();
		msg->output_conn = dest._handle;
		msg->rtp_proxy_handle = _handle;


		IwMessagePtr response = NULL_MSG;
		ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
			_rtpProxyHandleId,
			IwMessagePtr(msg),
			response,
			MilliSeconds(GetCurrRunningContext()->TransactionTimeout()),
			"Bridge RTP Connection TXN");

		if (res != API_SUCCESS)
		{
			LogWarn("Error bridging rtp connection " << res);
			return res;
		}

		switch (response->message_id)
		{
		case MSG_RTP_PROXY_ACK:
			{
				shared_ptr<MsgRtpProxyAck> ack 
					= dynamic_pointer_cast<MsgRtpProxyAck> (response);

				_bridgedHandle = dest._handle;

				return API_SUCCESS;
			}

		default:
			{
				return API_FAILURE;
			}
		}


	}

	AbstractOffer 
	RtpProxySession::LocalOffer()
	{
		return _localOffer;
	}

	AbstractOffer 
	RtpProxySession::RemoteOffer()
	{
		return _remoteOffer;
	}

}
