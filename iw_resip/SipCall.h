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

	enum SipCallEvts
	{
		SIP_CALL_INFO_REQ = MSG_CALL_LAST_ONE + 1,
		SIP_CALL_INFO_ACK,
		SIP_CALL_INFO_NACK

	};

	class MsgSipCallInfoReq:
		public MsgVoipCallMixin, public MsgRequest, public MsgCallOfferedMixin
	{
	public:
		MsgSipCallInfoReq():
		  MsgRequest(SIP_CALL_INFO_REQ, 
			  NAME(SIP_CALL_INFO_REQ)){}

	};

	class MsgSipCallInfoAck:
		public MsgVoipCallMixin, public MsgResponse, public MsgCallOfferedMixin
	{
	public:
		MsgSipCallInfoAck():
		  MsgResponse(SIP_CALL_INFO_ACK, 
			  NAME(SIP_CALL_INFO_ACK)){}

	};

	class MsgSipCallInfoNack:
		public MsgVoipCallMixin, public MsgResponse, public MsgCallOfferedMixin
	{
	public:
		MsgSipCallInfoNack():
		  MsgResponse(SIP_CALL_INFO_NACK, 
			  NAME(SIP_CALL_INFO_NACK)){}

	};


	class SipMediaCall : 
		public MediaCallSession
	{
	public:

		SipMediaCall(IN ScopedForking &forking,IN HandleId handle_id);

		SipMediaCall(IN ScopedForking &forking,
			 IN shared_ptr<MsgCallOfferedReq> offered_msg);

		virtual ApiErrorCode SendInfo(const string &body, const string &type);

		virtual void UponActiveObjectEvent(IwMessagePtr ptr);

		virtual ~SipMediaCall(void);

	private:

		LpHandlePtr _infosChannel;

	};


}




