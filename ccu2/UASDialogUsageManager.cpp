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
#include "LightweightProcess.h"
#include "ResipCommon.h"
#include "UASDialogUsageManager.h"
#include "UASAppDialogSetFactory.h"
#include "Call.h"
#include "CcuLogger.h"



UASDialogUsageManager::UASDialogUsageManager(
	IN SipStack &resip_stack, 
	IN CnxInfo ip_addr,
	IN ICallHandlerCreatorPtr call_handler_creator,
	IN CcuHandlesMap &ccu_handles_map,
	IN LightweightProcess &ccu_stack):
DialogUsageManager(resip_stack),
_ipAddr(ip_addr),
_callHandlerCreator(call_handler_creator),
_ccuHandlesMap(ccu_handles_map),
_ccu_stack(ccu_stack)
{
	string uasUri = "sip:VCS@" + _ipAddr.ipporttos();
	_uasAor		= NameAddrPtr(new NameAddr(uasUri.c_str()));

	addTransport(UDP,_ipAddr.port_ho());

	_uasMasterProfile = SharedPtr<MasterProfile>(new MasterProfile());
	setMasterProfile(_uasMasterProfile);

	auto_ptr<ClientAuthManager> _uasAuth (new ClientAuthManager());
	setClientAuthManager(_uasAuth);
	_uasAuth.release();


	getMasterProfile()->setDefaultFrom(*_uasAor);
	getMasterProfile()->setDefaultRegistrationTime(70);

	time_t bHangupAt = 0;
	
	setClientRegistrationHandler(this);
	setInviteSessionHandler(this);
	addOutOfDialogHandler(OPTIONS, this);


	auto_ptr<AppDialogSetFactory> uas_dsf(new UASAppDialogSetFactory());
	setAppDialogSetFactory(uas_dsf);
	uas_dsf.release();
}

UASDialogUsageManager::~UASDialogUsageManager(void)
{
}

void 
UASDialogUsageManager::UponCallOfferedAck(CcuMsgPtr req, SipDialogContextPtr ptr)
{
	
	shared_ptr<CcuMsgCalOfferedlAck> ack = 
		dynamic_pointer_cast<CcuMsgCalOfferedlAck>(req);
	
	
	string sdpStr = CreateSdp(ack->local_media);

	Data data_buffer(sdpStr.c_str());

#pragma warning (push )
#pragma warning( disable : 4267)
	HeaderFieldValue header(data_buffer.data(), data_buffer.size());
#pragma warning (pop)

	Mime type("application", "sdp");
	SdpContents sdp(&header, type);

	ptr->uas_invite_handle.get()->provideAnswer(sdp);
	ptr->uas_invite_handle.get()->accept();
}

string
UASDialogUsageManager::CreateSdp(CnxInfo &data)
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


void 
UASDialogUsageManager::onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
	FUNCTRACKER;
	LogDebug(">>New UAS SIP Session<< msg:-\n" << msg <<"\n");

	sis->provisional(180);

	if (_callHandlerCreator.get() == NULL)
	{
		_defaultHandlersMap[sis->getAppDialog()] = sis;
		return;
	}

	SipDialogContextPtr ctx_ptr(new SipDialogContext());
	ctx_ptr->uas_invite_handle = sis;
	ctx_ptr->stack_handle = GenerateSipHandle();

	_resipHandlesMap[sis->getAppDialog()]= ctx_ptr;
	_ccuHandlesMap[ctx_ptr->stack_handle]= ctx_ptr;


}

#pragma TODO("Handle early and late termination")

void 
UASDialogUsageManager::onTerminated(InviteSessionHandle handle , InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
{
	FUNCTRACKER;
	LogDebug(" Call on handle=[" << handle.getId() <<"] is terminated with reason=["  <<  reason << "]");

	if (_callHandlerCreator.get() == NULL)
	{
		_defaultHandlersMap.erase(handle->getAppDialog());
		return;
	}

}

void 
UASDialogUsageManager::onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)      
{
	FUNCTRACKER;

	if (_callHandlerCreator.get() == NULL)
	{
		ServerInviteSessionHandle sis = _defaultHandlersMap[is->getAppDialog()];
		// default port
		string sdpStr = CreateSdp(CnxInfo(_ipAddr.inaddr(), 60555));

		Data data_buffer(sdpStr.c_str());

#pragma warning (push )
#pragma warning( disable : 4267)
		HeaderFieldValue header(data_buffer.data(), data_buffer.size());
#pragma warning (pop)

		Mime type("application", "sdp");
		SdpContents sdp(&header, type);

		sis->provideAnswer(sdp);
		sis->accept();
		
		return;
	}

	ResipHandlesMap::iterator iter  = _resipHandlesMap.find(is->getAppDialog());
	SipDialogContextPtr ctx_ptr = (*iter).second;
	
	const SdpContents::Session &s = sdp.session();
	const Data &addr_data = s.connection().getAddress();
	const string addr = addr_data.c_str();

	const SdpContents::Session::Medium &medium = s.media().front();
	int port = medium.port();

	
	LpHandlePair pair = _callHandlerCreator->CreateCallHandler(
		_ccu_stack._pair,
		ctx_ptr->stack_handle,
		CnxInfo(addr,port));

	ctx_ptr->call_handler_inbound = pair.inbound;

}


void 
UASDialogUsageManager::onConnected(InviteSessionHandle is, const SipMessage& msg)
{
	FUNCTRACKER;

	ResipHandlesMap::iterator iter = _resipHandlesMap.find(is->getAppDialog());

	SipDialogContextPtr ctx_ptr = (*iter).second;

	ctx_ptr->call_handler_inbound->Send(new CcuMsgNewCallConnected());

}

void 
UASDialogUsageManager::onOfferRequired(InviteSessionHandle is, const SipMessage& msg)
{
	FUNCTRACKER;

	// 	LogDebug( ": InviteSession-onOfferRequired - " << msg.brief());
	// 	is->provideOffer(*_sdp);
}

void 
UASDialogUsageManager::onAnswer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)      
{
	LogDebug(  ": InviteSession-onAnswer(SDP)");
	// 	if(*pHangupAt == 0)
	// 	{
	// 		is->provideOffer(sdp);
	// 		*pHangupAt = time(NULL) + 5;
	// 	}
}


CcuApiErrorCode 
UASDialogUsageManager::HangupCall(SipDialogContextPtr ptr)
{
	FUNCTRACKER;


	_ccuHandlesMap.erase(ptr->stack_handle);
	_resipHandlesMap.erase(ptr->uas_invite_handle->getAppDialog());
	ptr->uas_invite_handle->end();

	return CCU_API_SUCCESS;

}



void 
UASDialogUsageManager::onReceivedRequest(ServerOutOfDialogReqHandle ood, const SipMessage& request)
{
	FUNCTRACKER;

	switch(request.header(h_CSeq).method())
	{
	case OPTIONS:
		{
			SharedPtr<SipMessage> resp = ood->reject(405);
			ood->send(resp);
			break;
		}
	default:
		{
			LogInfo("Unknown out-of-dialog request");
		}
	}

}