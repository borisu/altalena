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
		IN Configuration &conf,
		IN IwHandlesMap &ccu_handles_map,
		IN ResipDialogHandlesMap &resip_handles_map,
		IN DialogUsageManager &dum);

	virtual ~UACDialogUsageManager(void);

	virtual void UponMakeCallReq(IN IwMessagePtr request);

	virtual void UponBlindXferReq(IN IwMessagePtr req);

	virtual void UponMakeCallAckReq(IN IwMessagePtr request);

	virtual void UponHangupReq(IN IwMessagePtr ptr);

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

	virtual void CleanUpCall(
		IN SipDialogContextPtr ctx_ptr);
	
private:

	Configuration &_conf;

	DialogUsageManager &_dum;

	IwHandlesMap &_iwHandlesMap;

	ResipDialogHandlesMap &_resipHandlesMap;

};


}