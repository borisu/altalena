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
	RtpProxySession::RtpProxySession(void):
	_handle(IW_UNDEFINED)
	{
	}

	RtpProxySession::~RtpProxySession(void)
	{
		Deallocate();
	}

	ApiErrorCode 
	RtpProxySession::Allocate()
	{
		FUNCTRACKER;

		return Allocate(CnxInfo());
	}

	ApiErrorCode 
	RtpProxySession::Allocate(const CnxInfo &remote_media)
	{
		FUNCTRACKER;

		if (_handle != IW_UNDEFINED)
		{
			return API_FAILURE;
		}

		DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

		MsgRtpProxyAllocateReq *msg = new MsgRtpProxyAllocateReq();
		msg->remote_media = remote_media;
		

		IwMessagePtr response = NULL_MSG;
		ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
			RTP_PROXY_Q,
			IwMessagePtr(msg),
			response,
			MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
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
				_conn = ack->local_media;

				LogDebug("RtpProxySession::Allocate allocated conn:" << _handle << " conn:" << _conn.ipporttos());
				
				return API_SUCCESS;
			}
		
		default:
			{
				return API_FAILURE;
			}
		}

	}

	ApiErrorCode 
	RtpProxySession::Deallocate()
	{
		FUNCTRACKER;

		if (_handle == IW_UNDEFINED)
		{
			return API_SUCCESS;
		}

		MsgRtpProxyDeallocateReq *req = 
			new MsgRtpProxyDeallocateReq();

		req->rtp_proxy_handle = _handle;

		GetCurrLightWeightProc()->SendMessage(RTP_PROXY_Q,IwMessagePtr(req));

		return API_SUCCESS;
	}

	ApiErrorCode 
	RtpProxySession::Modify(const CnxInfo &conn)
	{
		if (_handle == IW_UNDEFINED)
		{
			return API_WRONG_STATE;
		}

		MsgRtpProxyModifyReq *req = 
			new MsgRtpProxyModifyReq();

		req->rtp_proxy_handle = _handle;
		req->remote_media  = conn;
		GetCurrLightWeightProc()->SendMessage(RTP_PROXY_Q,IwMessagePtr(req));

		return API_SUCCESS;

	}

	ApiErrorCode 
	RtpProxySession::Bridge(IN const RtpProxySession &dest)
	{
		FUNCTRACKER;

		LogDebug("Bridge src:" << _handle << ", dst:" << dest._handle);

		if (_handle == IW_UNDEFINED)
		{
			return API_WRONG_STATE;
		}

		DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

		MsgRtpProxyBridgeReq *msg = new MsgRtpProxyBridgeReq();
		msg->output_conn = dest._handle;
		msg->rtp_proxy_handle = _handle;


		IwMessagePtr response = NULL_MSG;
		ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
			RTP_PROXY_Q,
			IwMessagePtr(msg),
			response,
			MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
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

				return API_SUCCESS;
			}

		default:
			{
				return API_FAILURE;
			}
		}


	}

	CnxInfo 
	RtpProxySession::LocalCnxInfo()
	{
		return _conn;
	}

}
