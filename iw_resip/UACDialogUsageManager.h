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

class UACDialogUsageManager :
	public SipSessionHandlerAdapter
{
public:
	UACDialogUsageManager(
		IN ConfigurationPtr conf,
		IN IwHandlesMap &ccu_handles_map,
		IN ResipDialogHandlesMap &resip_handles_map,
		IN DialogUsageManager &dum);

	virtual ~UACDialogUsageManager(void);

	virtual void UponMakeCallReq(IN IwMessagePtr request);

	virtual void UponRegisterReq(IN IwMessagePtr request);

	virtual void UponUnRegisterReq(IN IwMessagePtr request);

	virtual void UponBlindXferReq(IN IwMessagePtr req);

	virtual void UponCallConnected(IN IwMessagePtr request);

	virtual void UponHangupReq(IN IwMessagePtr ptr);

	virtual void UponInfoReq(IN IwMessagePtr ptr);

	virtual void UponSubscribeReq(IN IwMessagePtr ptr);

	virtual void onNewSession(
		IN ClientInviteSessionHandle s, 
		IN InviteSession::OfferAnswerType oat, 
		IN const SipMessage& msg);

	virtual void onConnected(
		IN ClientInviteSessionHandle is, 
		IN const SipMessage& msg);

	/// Received a failure response from UAS
	virtual void onFailure(
		IN ClientInviteSessionHandle, 
		IN const SipMessage& msg);

	virtual void onFailure(
		IN ClientOutOfDialogReqHandle h, 
		IN const SipMessage& errorResponse);

	virtual void CleanUpCall(
		IN SipDialogContextPtr ctx_ptr);

	/// called when INFO message is received 
	//virtual void onInfo(InviteSessionHandle, const SipMessage& msg){ FUNCTRACKER;};

	/// called when response to INFO message is received 
	virtual void onInfoSuccess(
		IN InviteSessionHandle, 
		IN const SipMessage& msg);

	virtual void onInfoFailure(
		IN InviteSessionHandle, 
		IN const SipMessage& msg);

	virtual void onAnswer(
		IN InviteSessionHandle h, 
		IN const SipMessage& msg, 
		IN const Contents& body);

	virtual void onSuccess(
		IN ClientRegistrationHandle h, 
		IN const SipMessage& response);

	virtual void onFailure(
		IN ClientRegistrationHandle h, 
		IN const SipMessage& msg);

	virtual void onRemoved(
		IN ClientRegistrationHandle h, 
		IN const SipMessage& response);

private:

	ConfigurationPtr _conf;

	DialogUsageManager &_dum;

	IwHandlesMap &_iwHandlesMap;

	ResipDialogHandlesMap &_resipHandlesMap;

};


}