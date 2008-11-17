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
#include "ccu.h"
#include "ResipCommon.h"
#include "UacSessionHandler.h"


using namespace resip;
using namespace boost;

typedef 
shared_ptr<UACSessionHandler> UACSessionHandlerPtr;


class UACDialogUsageManager :
	public DialogUsageManager,public SipSessionHandlerAdapter
{
public:
	UACDialogUsageManager(
		IN SipStack &sipStack,
		IN CcuMediaData data,
		IN CcuHandlesMap &ccu_handles_map,
		IN LightweightProcess &ccu_stack);

	virtual ~UACDialogUsageManager(void);

	virtual CcuApiErrorCode MakeCall(IN CcuMsgPtr request);

	virtual CcuApiErrorCode HangupCall(SipDialogContextPtr ptr);

	virtual string CreateSdp(IN CcuMediaData &data);

	virtual void onNewSession(
		IN ClientInviteSessionHandle s, 
		IN InviteSession::OfferAnswerType oat, 
		IN const SipMessage& msg);

	virtual void onConnected(
		IN ClientInviteSessionHandle is, 
		IN const SipMessage& msg);
	

	virtual void onTerminated(
		IN InviteSessionHandle, 
		IN InviteSessionHandler::TerminatedReason reason, 
		IN const SipMessage* msg);
	
	virtual void Shutdown();

private:

	NameAddrPtr  _nameAddr;

	CcuHandlesMap &_ccuHandlesMap;

	ResipHandlesMap _resipHandlesMap;

	LightweightProcess &_ccu_stack;

};


