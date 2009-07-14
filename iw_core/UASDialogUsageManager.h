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
#include "IwBase.h"
#include "ResipCommon.h"
#include "SipSessionHandlerAdapter.h"
#include "Configuration.h"

using namespace resip;
using namespace boost;

namespace ivrworx
{

	typedef
	map<AppDialogHandle,ServerInviteSessionHandle> DefaultHandlersMap;


	class UASDialogUsageManager:
		public SipSessionHandlerAdapter
	{
	public:
		UASDialogUsageManager(
			IN Configuration &conf,
			IN IwHandlesMap &ccu_handles_map,
			IN ResipDialogHandlesMap &resipHandlesMap,
			IN LightweightProcess &ccu_stack,
			IN DialogUsageManager &dum);

		virtual ~UASDialogUsageManager(void);

		virtual void UponCallOfferedAck(IwMessagePtr req);

		virtual void UponCallOfferedNack(IwMessagePtr req);

		virtual void onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg);

		virtual void onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp);

		virtual void onReceivedRequest(ServerOutOfDialogReqHandle ood, const SipMessage& request);

		/// called when ACK (with out an answer) is received for initial invite (UAS)
		virtual void onConnectedConfirmed(InviteSessionHandle, const SipMessage &msg);

		virtual void CleanUpCall(IN SipDialogContextPtr ctx_ptr);

		virtual void HangupCall(IN SipDialogContextPtr ptr);


	private:

		Configuration &_conf;

		ResipDialogHandlesMap &_resipHandlesMap;

		IwHandlesMap &_refIwHandlesMap;

		LightweightProcess &_sipStack;

		long _sdpVersionCounter;

		DialogUsageManager &_dum;

	};

}






