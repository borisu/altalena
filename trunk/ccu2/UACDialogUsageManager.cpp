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

#include "StdAfx.h"
#include "Ccu.h"
#include "LightweightProcess.h"
#include "ResipCommon.h"
#include "UacDialogUsageManager.h"
#include "UACUserProfile.h"
#include "UACAppDialogSet.h"
#include "CcuLogger.h"



#pragma warning( push )
#pragma warning( disable : 4267 )

UACDialogUsageManager::UACDialogUsageManager(
	IN SipStack &sipStack,
	IN CcuMediaData data,
	IN CcuHandlesMap &ccu_handles_map,
	IN LightweightProcess &ccu_stack):
DialogUsageManager(sipStack),
_ccuHandlesMap(ccu_handles_map),
_ccu_stack(ccu_stack)
{
	string uasUri = "sip:VCS@" + data.iptos();
	_nameAddr	= NameAddrPtr(new NameAddr(uasUri.c_str()));
	
	
	SharedPtr<MasterProfile> uacMasterProfile = 
		SharedPtr<MasterProfile>(new MasterProfile());
	auto_ptr<ClientAuthManager> uacAuth(new ClientAuthManager);

	setMasterProfile(uacMasterProfile);
	getMasterProfile()->setDefaultFrom(*_nameAddr);
	getMasterProfile()->setDefaultRegistrationTime(70);


	setClientAuthManager(uacAuth);

	
	setInviteSessionHandler(this);
	setClientRegistrationHandler(this);
	addOutOfDialogHandler(OPTIONS,this);

	
}

#pragma TODO ("Transfer to SIP common and make return sdp instead of string")
string
UACDialogUsageManager::CreateSdp(IN CcuMediaData &data)
{

	string empty_str = "";
	string sdpStr = empty_str + "v=0\r\n"
		"o=1900 369696545 369696545 IN IP4  " + data.iptos() + "\r\n"
		"s=CCU\r\n"
		"c=IN IP4 "  + data.iptos() + "\r\n"
		"t=0 0\r\n"
		"m=audio " + data.porttos() + " RTP/AVP 8 101\r\n"
		"a=rtpmap:8 pcma/8000\r\n"
		"a=rtpmap:101 telephone-event/8000\r\n"
		"a=fmtp:101 0-15\r\n";

	return sdpStr;

}

CcuApiErrorCode 
UACDialogUsageManager::HangupCall(SipDialogContextPtr ptr)
{
	FUNCTRACKER;

	

	_ccuHandlesMap.erase(ptr->stack_handle);
	_resipHandlesMap.erase(ptr->uac_invite_handle->getAppDialog());
	ptr->uac_invite_handle->end();

	return CCU_API_SUCCESS;

}


CcuApiErrorCode 
UACDialogUsageManager::MakeCall(IN CcuMsgPtr ptr)
{
	FUNCTRACKER;

	try {

		CcuMsgMakeCallReq *req  = 
			boost::shared_dynamic_cast<CcuMsgMakeCallReq>(ptr).get();

		//
		// prepare headers
		//
		NameAddr name_addr(WStringToString(req->destination_uri).c_str());
		SharedPtr<UserProfile> user_profile;
		if (!req->on_behalf_of.empty())
		{
			user_profile = SharedPtr<UserProfile>(new UACUserProfile(WStringToString(req->on_behalf_of)));
		} else
		{
			user_profile = SharedPtr<UserProfile>(getMasterProfile());
		}
		

		//
		// prepare sdp
		//
		string sdpStr = CreateSdp(req->local_media);

		Data data_buffer(sdpStr.c_str());
		HeaderFieldValue header(data_buffer.data(), data_buffer.size());
		Mime type("application", "sdp");
		SdpContents sdp(&header, type);

		
		//
		// create context
		//
		SipDialogContextPtr ctx_ptr = 
			SipDialogContextPtr(new SipDialogContext());

		UACAppDialogSet * uac_dialog_set = new UACAppDialogSet(*this,ctx_ptr);
		SharedPtr<SipMessage> session = 
			makeInviteSession(
			name_addr, 
			user_profile,
			&sdp, 
			uac_dialog_set); 


		send(session);

		
		ctx_ptr->stack_handle = GenerateSipHandle();
		ctx_ptr->transaction_type = CCU_UAC;
		ctx_ptr->call_handler_inbound = req->call_handler_inbound;
		ctx_ptr->orig_request = ptr;

		return CCU_API_SUCCESS;

	} catch (BaseException &e)
	{
		LogDebug("Caught exception while trying to make call=[" << e.getMessage() <<"]");

		return CCU_API_FAILURE;
		
	}
}

void 
UACDialogUsageManager::onNewSession(
									IN ClientInviteSessionHandle s, 
									IN InviteSession::OfferAnswerType oat, 
									IN const SipMessage& msg)
{
	
	
	SipDialogContextPtr ctx_ptr = ((UACAppDialogSet*)(s->getAppDialogSet().get()))->_ptr;
	ctx_ptr->uac_invite_handle = s;
	

	//
	// put them in maps
	//
	_resipHandlesMap[s->getAppDialog()]= ctx_ptr;
	_ccuHandlesMap[ctx_ptr->stack_handle]= ctx_ptr;

}

void 
UACDialogUsageManager::onTerminated(IN InviteSessionHandle is , IN InviteSessionHandler::TerminatedReason reason, IN const SipMessage* msg) 
{
	FUNCTRACKER;

	LogInfo("The resip dialog handle=[" << is.getId() <<"] >>terminated<< with reason=[" << reason <<"]." );

	SipDialogContextPtr ctx_ptr = ((UACAppDialogSet*)(is->getAppDialogSet().get()))->_ptr;

	
	CcuMsgHangupCallReq *hang_up_evt = 
		new CcuMsgHangupCallReq();
	hang_up_evt->stack_call_handle = ctx_ptr->stack_handle;

	ctx_ptr->call_handler_inbound->Send(hang_up_evt);

	//
	// remove from maps
	//
	_resipHandlesMap.erase(is->getAppDialog());
	_ccuHandlesMap.erase(ctx_ptr->stack_handle);

}

void 
UACDialogUsageManager::onConnected(IN ClientInviteSessionHandle is, IN const SipMessage& msg)
{
	FUNCTRACKER;

	ResipHandlesMap::iterator iter = _resipHandlesMap.find(is->getAppDialog());

	SipDialogContextPtr ctx_ptr = (*iter).second;

	_ccu_stack.SendResponse(
		ctx_ptr->orig_request,
		new CcuMsgMakeCallAck(ctx_ptr->stack_handle));
}

void 
UACDialogUsageManager::Shutdown()
{
	forceShutdown(NULL);
}

UACDialogUsageManager::~UACDialogUsageManager(void)
{
}


#pragma warning( pop )