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
#include "LpHandle.h"
#include "Call.h"


using namespace boost;
using namespace resip;
using namespace std;

typedef 
shared_ptr<NameAddr> NameAddrPtr;

typedef
shared_ptr<Data> DataPtr;

typedef
shared_ptr<HeaderFieldValue> HeaderFieldValuePtr;

typedef
shared_ptr<SdpContents> SdpContentsPtr;

wostream& 
operator <<(wostream &ostream, const Data &data);

wostream& 
operator <<(wostream& strm,  const Message& msg);

typedef 
int CcuStackHandle;

CcuStackHandle 
GenerateSipHandle();

enum TransactionType
{
	CCU_UAS,
	CCU_UAC
};

struct SipDialogContext :
	public boost::noncopyable
{
	SipDialogContext();

	TransactionType transaction_type;

	ServerInviteSessionHandle uas_invite_handle;

	ClientInviteSessionHandle uac_invite_handle;

	LpHandlePtr call_handler_inbound;

	// used to send responses as it is 
	// temporary process used to run transaction
	//
	CcuMsgPtr last_user_request;

	CcuStackHandle stack_handle;
};


typedef 
shared_ptr<SipDialogContext> SipDialogContextPtr;

typedef 
map<CcuStackHandle,SipDialogContextPtr> CcuHandlesMap;

typedef 
map<AppDialogHandle,SipDialogContextPtr> ResipDialogHandlesMap;


CnxInfo 
CreateMediaData(const SdpContents& sdp);





