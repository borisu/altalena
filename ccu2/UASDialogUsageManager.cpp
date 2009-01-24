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
	IN CcuConfiguration &conf,
	IN SipStack &resip_stack, 
	IN CcuHandlesMap &ccu_handles_map,
	IN LightweightProcess &ccu_stack):
DialogUsageManager(resip_stack),
_conf(conf),
_refCcuHandlesMap(ccu_handles_map),
_ccu_stack(ccu_stack)
{
	
	_sdpVersionCounter = ::GetTickCount();

	CnxInfo ipAddr = conf.VcsCnxInfo();

	wstring uasUri = L"sip:" + conf.From() + L"@" + ipAddr.ipporttows();
	_uasAor		= NameAddrPtr(new NameAddr(WStringToString(uasUri).c_str()));

	addTransport(UDP,ipAddr.port_ho());
	addTransport(TCP,ipAddr.port_ho());

	_uasMasterProfile = SharedPtr<MasterProfile>(new MasterProfile());
	setMasterProfile(_uasMasterProfile);

	auto_ptr<ClientAuthManager> _uasAuth (new ClientAuthManager());
	setClientAuthManager(_uasAuth);
	_uasAuth.release();


	getMasterProfile()->setDefaultFrom(*_uasAor);
	getMasterProfile()->setDefaultRegistrationTime(70);

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
UASDialogUsageManager::UponCallOfferedNack(CcuMsgPtr req)
{
	shared_ptr<CcuMsgCallOfferedNack> ack = 
		dynamic_pointer_cast<CcuMsgCallOfferedNack>(req);

	CcuHandlesMap::iterator iter = _refCcuHandlesMap.find(ack->stack_call_handle);
	if (iter == _refCcuHandlesMap.end())
	{
		LogWarn("Handle=[" << ack->stack_call_handle<< "] >>not found<<. Has caller disconnected???");
		return;
	}

	SipDialogContextPtr ptr = (*iter).second;

	LogDebug(">>Closing<< call handle=[" << ptr->stack_handle << "]");

	_refCcuHandlesMap.erase(ptr->stack_handle);
	
	ptr->uas_invite_handle->end();

}

void 
UASDialogUsageManager::UponCallOfferedAck(CcuMsgPtr req, SipDialogContextPtr ptr)
{
	
	shared_ptr<CcuMsgCalOfferedlAck> ack = 
		dynamic_pointer_cast<CcuMsgCalOfferedlAck>(req);
	
	
	string sdpStr = 
		CreateSdp(ack->local_media);

	Data data_buffer(sdpStr.c_str());

	HeaderFieldValue header(data_buffer.data(), (unsigned int)data_buffer.size());

	Mime type("application", "sdp");
	SdpContents sdp(&header, type);
	

	ptr->uas_invite_handle.get()->provideAnswer(sdp);
	ptr->uas_invite_handle.get()->accept();
}


string
UASDialogUsageManager::CreateSdp(CnxInfo &offered_sdp)
{


	char session_id_buf[CCU_MAX_LONG_LENGTH];
	_ltoa_s(::GetTickCount(),session_id_buf,CCU_MAX_LONG_LENGTH,10);

	char version_buf[CCU_MAX_LONG_LENGTH];
	_ltoa_s(_sdpVersionCounter++,version_buf,CCU_MAX_LONG_LENGTH,10);

	
	string sdp_str = 
		//v=0
		"v=0\r\n"
		//o=<username> <session id> <version> <network type> <address type> <address>
		"o=myivr " +  string(session_id_buf) +  " " + string(version_buf) + " IN IP4  " + offered_sdp.iptos() + "\r\n"
		//s=<session name>
		"s=myivr\r\n"
		// c=<network type> <address type> <connection address>
		"c=IN IP4 "  + offered_sdp.iptos() + "\r\n"
		// t=<start time>  <stop time>
		"t=0 0\r\n";


	string codecs_list_str =	"m=audio " + offered_sdp.porttos() + " RTP/AVP ";
	
	string sdp_map;
	
	for (CodecsList::const_iterator iter = _conf.CodecList().begin(); 
		iter != _conf.CodecList().end();
		iter++)
	{
		codecs_list_str += " " + (*iter)->sdp_mapping_tos();
		sdp_map += (*iter)->get_sdp_a();
		
	}

	sdp_str += codecs_list_str + "\r\n";

	sdp_str += sdp_map;

	return sdp_str;

}


void 
UASDialogUsageManager::onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
	FUNCTRACKER;
	LogDebug(">>New UAS SIP Session<< msg:-\n" << msg <<"\n");

	sis->provisional(180);

	SipDialogContextPtr ctx_ptr(new SipDialogContext());
	ctx_ptr->uas_invite_handle = sis;
	ctx_ptr->stack_handle = GenerateSipHandle();

	_resipHandlesMap[sis->getAppDialog()]= ctx_ptr;
	_refCcuHandlesMap[ctx_ptr->stack_handle]= ctx_ptr;


}

#pragma TODO("Handle early and late termination")

void 
UASDialogUsageManager::onTerminated(InviteSessionHandle handle , InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
{
	FUNCTRACKER;
	LogDebug(" Call on handle=[" << handle.getId() <<"] is terminated with reason=["  <<  reason << "]");

}

void 
UASDialogUsageManager::onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)      
{
	FUNCTRACKER;

	ResipHandlesMap::iterator iter  = _resipHandlesMap.find(is->getAppDialog());
	SipDialogContextPtr ctx_ptr = (*iter).second;
	
	const SdpContents::Session &s = sdp.session();
	const Data &addr_data = s.connection().getAddress();
	const string addr = addr_data.c_str();

	const SdpContents::Session::Medium &medium = s.media().front();
	int port = medium.port();
	
	
	DECLARE_NAMED_HANDLE_PAIR(call_handler_pair);

	CcuMsgCallOffered *offered = new CcuMsgCallOffered();
	offered->remote_media = CnxInfo(addr,port);
	offered->stack_call_handle = ctx_ptr->stack_handle;

	_ccu_stack._outbound->Send(offered);


	ctx_ptr->call_handler_inbound = call_handler_pair.inbound;

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


	_refCcuHandlesMap.erase(ptr->stack_handle);
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