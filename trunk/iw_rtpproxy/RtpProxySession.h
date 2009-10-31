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

#include "IwUtils.h"
#include "Message.h"

#define RTP_PROXY_Q 10

namespace ivrworx
{
	enum RtpProxyEvents
	{
		MSG_RTP_PROXY_ALLOCATE_REQ = MSG_USER_DEFINED,
		MSG_RTP_PROXY_ACK,
		MSG_RTP_PROXY_NACK,
		MSG_RTP_PROXY_BRIDGE_REQ,
		MSG_RTP_PROXY_MODIFY_REQ,
		MSG_RTP_PROXY_DEALLOCATE_REQ,
	};

	typedef int 
	RtpProxyHandle;

	class RtpProxyMixin 
	{
	public :
		RtpProxyMixin():rtp_proxy_handle(IW_UNDEFINED){};
		RtpProxyHandle rtp_proxy_handle;
		CnxInfo local_media;
		CnxInfo remote_media;
	};		


	class MsgRtpProxyAllocateReq:
		public MsgRequest, 
		public RtpProxyMixin
	{
	public:
		MsgRtpProxyAllocateReq():
		  MsgRequest(MSG_RTP_PROXY_ALLOCATE_REQ, 
			  NAME(MSG_RTP_PROXY_ALLOCATE_REQ)){};

	};

	class MsgRtpProxyDeallocateReq:
		public MsgRequest, 
		public RtpProxyMixin
	{
	public:
		MsgRtpProxyDeallocateReq():
		  MsgRequest(MSG_RTP_PROXY_DEALLOCATE_REQ, 
			  NAME(MSG_RTP_PROXY_DEALLOCATE_REQ)){};

		  CnxInfo remote_cnx_info;

	};

	class MsgRtpProxyBridgeReq:
		public MsgRequest, 
		public RtpProxyMixin
	{
	public:
		MsgRtpProxyBridgeReq():
		  MsgRequest(MSG_RTP_PROXY_BRIDGE_REQ, 
			  NAME(MSG_RTP_PROXY_BRIDGE_REQ)),output_conn(IW_UNDEFINED){};

		RtpProxyHandle output_conn;
	};


	class MsgRtpProxyAck:
		public IwMessage, public RtpProxyMixin
	{
	public:
		MsgRtpProxyAck():
		  IwMessage(MSG_RTP_PROXY_ACK, 
			  NAME(MSG_RTP_PROXY_ACK))
		  {};
	};

	class MsgRtpProxyNack:
		public IwMessage, public RtpProxyMixin
	{
	public:
		MsgRtpProxyNack():
		  IwMessage(MSG_RTP_PROXY_NACK, 
			  NAME(MSG_RTP_PROXY_NACK))
		  {};



	};

	class MsgRtpProxyModifyReq:
		public MsgRequest, 
		public RtpProxyMixin
	{
	public:
		MsgRtpProxyModifyReq():
		  MsgRequest(MSG_RTP_PROXY_MODIFY_REQ, 
			  NAME(MSG_RTP_PROXY_MODIFY_REQ)){};

	};

	class RtpProxySession
	{
	public:

		RtpProxySession(void);

		virtual ~RtpProxySession(void);

		virtual ApiErrorCode Allocate();

		virtual ApiErrorCode Allocate(const CnxInfo &remote_media);

		virtual ApiErrorCode Deallocate();

		virtual ApiErrorCode Modify(const CnxInfo &conn);

		virtual ApiErrorCode Bridge(IN const RtpProxySession &dest);

		virtual CnxInfo LocalCnxInfo();

		virtual RtpProxyHandle RtpHandle();

	private:

		CnxInfo _conn;

		RtpProxyHandle _handle;


	};


}
