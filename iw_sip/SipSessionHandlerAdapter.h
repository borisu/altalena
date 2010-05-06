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

#include "Logger.h"

using namespace std;
using namespace resip;
using namespace ivrworx;

class SipSessionHandlerAdapter:
	public InviteSessionHandler, 
	public ClientSubscriptionHandler,
	public ClientRegistrationHandler,
	public OutOfDialogHandler
{
public:

	SipSessionHandlerAdapter(void):InviteSessionHandler(true){};

	virtual ~SipSessionHandlerAdapter(void){};

	virtual void onSuccess(ClientRegistrationHandle h, const SipMessage& response){FUNCTRACKER;};

	virtual void onFailure(ClientRegistrationHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onRemoved(ClientRegistrationHandle, const SipMessage& response){FUNCTRACKER;};

	virtual int onRequestRetry(ClientRegistrationHandle, int retrySeconds, const SipMessage& response)
	{
		FUNCTRACKER;
		return -1;
	};

	virtual void onNewSession(ClientInviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg){FUNCTRACKER;};

	virtual void onNewSession(ServerInviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg){FUNCTRACKER;};

	virtual void onFailure(ClientInviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onProvisional(ClientInviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onConnected(ClientInviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onStaleCallTimeout(ClientInviteSessionHandle handle){FUNCTRACKER;};

	virtual void onConnected(InviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onRedirected(ClientInviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onTerminated(InviteSessionHandle, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg){FUNCTRACKER;};

	virtual void onAnswer(InviteSessionHandle, const SipMessage& msg, const SdpContents& sdp){FUNCTRACKER;};

	virtual void onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp){FUNCTRACKER;};      

	virtual void onEarlyMedia(ClientInviteSessionHandle, const SipMessage& msg, const SdpContents& sdp){FUNCTRACKER;};

	virtual void onOfferRequired(InviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onOfferRejected(InviteSessionHandle, const SipMessage* msg){FUNCTRACKER;};

	virtual void onDialogModified(InviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg){FUNCTRACKER;};

	virtual void onRefer(InviteSessionHandle, ServerSubscriptionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onReferAccepted(InviteSessionHandle, ClientSubscriptionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onReferRejected(InviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onReferNoSub(InviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onInfo(InviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onInfoSuccess(InviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onInfoFailure(InviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onMessage(InviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onMessageSuccess(InviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onMessageFailure(InviteSessionHandle, const SipMessage& msg){FUNCTRACKER;};

	virtual void onForkDestroyed(ClientInviteSessionHandle){FUNCTRACKER;};

	// Out-of-Dialog Callbacks
	virtual void onSuccess(ClientOutOfDialogReqHandle, const SipMessage& successResponse){FUNCTRACKER;};

	virtual void onFailure(ClientOutOfDialogReqHandle, const SipMessage& errorResponse){FUNCTRACKER;};

	virtual void onReceivedRequest(ServerOutOfDialogReqHandle ood, const SipMessage& request){FUNCTRACKER;};

	// Subscription callbacks
	// Client must call acceptUpdate or rejectUpdate for any onUpdateFoo
	virtual void onUpdatePending(ClientSubscriptionHandle, const SipMessage& notify, bool outOfOrder){FUNCTRACKER;};

	virtual void onUpdateActive(ClientSubscriptionHandle, const SipMessage& notify, bool outOfOrder){FUNCTRACKER;};

	//unknown Subscription-State value
	virtual void onUpdateExtension(ClientSubscriptionHandle, const SipMessage& notify, bool outOfOrder){FUNCTRACKER;};

	virtual int onRequestRetry(ClientSubscriptionHandle, int retrySeconds, const SipMessage& notify){ return -1;};

	//subscription can be ended through a notify or a failure response.
	virtual void onTerminated(ClientSubscriptionHandle, const SipMessage& msg){FUNCTRACKER;};   

	//not sure if this has any value.
	virtual void onNewSubscription(ClientSubscriptionHandle, const SipMessage& notify){FUNCTRACKER;};

	virtual void onTerminated(resip::ClientSubscriptionHandle,const resip::SipMessage *){FUNCTRACKER;};


};

