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

using namespace std;
using namespace resip;

class SipSessionHandlerAdapter:
	public InviteSessionHandler, 
	public ClientSubscriptionHandler,
	public ClientRegistrationHandler,
	public OutOfDialogHandler
{
public:

	SipSessionHandlerAdapter(void){};

	virtual ~SipSessionHandlerAdapter(void){};

	virtual void onSuccess(ClientRegistrationHandle h, const SipMessage& response){};

	virtual void onFailure(ClientRegistrationHandle, const SipMessage& msg){};

	virtual void onRemoved(ClientRegistrationHandle, const SipMessage& response){};

	virtual int onRequestRetry(ClientRegistrationHandle, int retrySeconds, const SipMessage& response)
	{
		return -1;
	};

	virtual void onNewSession(ClientInviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg){};

	virtual void onNewSession(ServerInviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg){};

	virtual void onFailure(ClientInviteSessionHandle, const SipMessage& msg){};

	virtual void onProvisional(ClientInviteSessionHandle, const SipMessage& msg){};

	virtual void onConnected(ClientInviteSessionHandle, const SipMessage& msg){};

	virtual void onStaleCallTimeout(ClientInviteSessionHandle handle){};

	virtual void onConnected(InviteSessionHandle, const SipMessage& msg){};

	virtual void onRedirected(ClientInviteSessionHandle, const SipMessage& msg){};

	virtual void onTerminated(InviteSessionHandle, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg){};

	virtual void onAnswer(InviteSessionHandle, const SipMessage& msg, const SdpContents& sdp){};

	virtual void onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp){};      

	virtual void onEarlyMedia(ClientInviteSessionHandle, const SipMessage& msg, const SdpContents& sdp){};

	virtual void onOfferRequired(InviteSessionHandle, const SipMessage& msg){};

	virtual void onOfferRejected(InviteSessionHandle, const SipMessage* msg){};

	virtual void onDialogModified(InviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg){};

	virtual void onRefer(InviteSessionHandle, ServerSubscriptionHandle, const SipMessage& msg){};

	virtual void onReferAccepted(InviteSessionHandle, ClientSubscriptionHandle, const SipMessage& msg){};

	virtual void onReferRejected(InviteSessionHandle, const SipMessage& msg){};

	virtual void onReferNoSub(InviteSessionHandle, const SipMessage& msg){};

	virtual void onInfo(InviteSessionHandle, const SipMessage& msg){};

	virtual void onInfoSuccess(InviteSessionHandle, const SipMessage& msg){};

	virtual void onInfoFailure(InviteSessionHandle, const SipMessage& msg){};

	virtual void onMessage(InviteSessionHandle, const SipMessage& msg){};

	virtual void onMessageSuccess(InviteSessionHandle, const SipMessage& msg){};

	virtual void onMessageFailure(InviteSessionHandle, const SipMessage& msg){};

	virtual void onForkDestroyed(ClientInviteSessionHandle){};

	// Out-of-Dialog Callbacks
	virtual void onSuccess(ClientOutOfDialogReqHandle, const SipMessage& successResponse){};

	virtual void onFailure(ClientOutOfDialogReqHandle, const SipMessage& errorResponse){};

	virtual void onReceivedRequest(ServerOutOfDialogReqHandle ood, const SipMessage& request){};

	// Subscription callbacks
	// Client must call acceptUpdate or rejectUpdate for any onUpdateFoo
	virtual void onUpdatePending(ClientSubscriptionHandle, const SipMessage& notify, bool outOfOrder){};

	virtual void onUpdateActive(ClientSubscriptionHandle, const SipMessage& notify, bool outOfOrder){};

	//unknown Subscription-State value
	virtual void onUpdateExtension(ClientSubscriptionHandle, const SipMessage& notify, bool outOfOrder){};

	virtual int onRequestRetry(ClientSubscriptionHandle, int retrySeconds, const SipMessage& notify){ return -1;};

	//subscription can be ended through a notify or a failure response.
	virtual void onTerminated(ClientSubscriptionHandle, const SipMessage& msg){};   

	//not sure if this has any value.
	virtual void onNewSubscription(ClientSubscriptionHandle, const SipMessage& notify){};


};

