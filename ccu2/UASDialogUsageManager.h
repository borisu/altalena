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
#include "Ccu.h"
#include "ResipCommon.h"
#include "SipSessionHandlerAdapter.h"
#include "CcuConfiguration.h"

using namespace resip;
using namespace boost;

typedef
map<AppDialogHandle,ServerInviteSessionHandle> DefaultHandlersMap;


class UASDialogUsageManager:
	public DialogUsageManager, 
	public SipSessionHandlerAdapter
{
public:
	UASDialogUsageManager(
		IN CcuConfiguration &conf,
		IN SipStack &resip_stack, 
		IN CcuHandlesMap &ccu_handles_map,
		IN LightweightProcess &ccu_stack);

	virtual ~UASDialogUsageManager(void);

	virtual void UponCallOfferedAck(IxMsgPtr req);

	virtual void UponCallOfferedNack(IxMsgPtr req);

	virtual void onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg);

	virtual void onTerminated(InviteSessionHandle, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg);

	virtual void onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp);

	virtual void onOfferRequired(InviteSessionHandle is, const SipMessage& msg);

	virtual void onAnswer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp); 

	virtual IxApiErrorCode HangupCall(SipDialogContextPtr ptr);

	virtual void onReceivedRequest(ServerOutOfDialogReqHandle ood, const SipMessage& request);

	virtual void onConnected(InviteSessionHandle, const SipMessage& msg);

	virtual string CreateSdp(CnxInfo &offered_sdp, const MediaFormat &codec);

	virtual void CleanUpCall(IN SipDialogContextPtr ctx_ptr);

private:

	CcuConfiguration &_conf;
	
	SharedPtr<MasterProfile> _uasMasterProfile;

	NameAddrPtr _uasAor;

	ResipDialogHandlesMap _resipHandlesMap;

	CcuHandlesMap &_refCcuHandlesMap;

	LightweightProcess &_ccu_stack;

	long _sdpVersionCounter;

};







