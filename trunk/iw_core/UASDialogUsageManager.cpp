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
UASDialogUsageManager::UponCallOfferedAck(CcuMsgPtr req)
{
	
	shared_ptr<CcuMsgCalOfferedlAck> ack = 
		dynamic_pointer_cast<CcuMsgCalOfferedlAck>(req);

	CcuHandlesMap::iterator iter = _refCcuHandlesMap.find(ack->stack_call_handle);
	if (iter == _refCcuHandlesMap.end())
	{
		LogWarn("Handle=[" << ack->stack_call_handle<< "] >>not found<<. Has caller disconnected???");
		return;
	}

	SipDialogContextPtr ptr = (*iter).second;
	ptr->last_user_request = req;
	
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
#define IX_MSX_SDP_SIZE		1024

	static char sdp_buf[IX_MSX_SDP_SIZE];
	sdp_buf[0] = '\0';

	char temp_buf[CCU_MAX_LONG_LENGTH];
	temp_buf[0] = '\0';

	int length_counter=0;

#define STRCAT_COUNT(BUF,S) length_counter+=(int)::strlen((S));\
	::strcat_s((BUF), IX_MSX_SDP_SIZE - length_counter, (S));
	
	//v=0\r\n
	STRCAT_COUNT(sdp_buf, "v=0\r\n");

	//o=<username>_
	STRCAT_COUNT(sdp_buf, "o=ivrworx ");
	//<session id>_ 
	_ltoa_s(::GetTickCount(),temp_buf,CCU_MAX_LONG_LENGTH,10);
	STRCAT_COUNT(sdp_buf, temp_buf);
	STRCAT_COUNT(sdp_buf, " ");
	//<version>_ 
	_ltoa_s(_sdpVersionCounter++,temp_buf,CCU_MAX_LONG_LENGTH,10);
	STRCAT_COUNT(sdp_buf, temp_buf);
	//<network type>_
	STRCAT_COUNT(sdp_buf," IN IP4 ");
	//<address>\r\n
	STRCAT_COUNT(sdp_buf,offered_sdp.iptoa());
	STRCAT_COUNT(sdp_buf,"\r\n");

	//s=<session name>\r\n
	STRCAT_COUNT(sdp_buf,"s=myivr\r\n");	
		
	// c=<network type> <address type>_
	STRCAT_COUNT(sdp_buf,"c=IN IP4 ")  ;

	// <connection address>\r\n
	STRCAT_COUNT(sdp_buf,offered_sdp.iptoa())
	STRCAT_COUNT(sdp_buf,"\r\n")

	// t=<start time>  <stop time>\r\n
	STRCAT_COUNT(sdp_buf,"t=0 0\r\n");

	// m=audio_
	STRCAT_COUNT(sdp_buf,"m=audio ");
	STRCAT_COUNT(sdp_buf,offered_sdp.porttos().c_str());

	//_RTP/AVP
	STRCAT_COUNT(sdp_buf," RTP/AVP");
	
	
	
	for (CodecsList::const_iterator iter = _conf.CodecList().begin(); 
		iter != _conf.CodecList().end();
		iter++)
	{
		STRCAT_COUNT(sdp_buf," "); 
		STRCAT_COUNT(sdp_buf,(*iter)->sdp_mapping_tos().c_str());
	}

	STRCAT_COUNT(sdp_buf,"\r\n"); 
	for (CodecsList::const_iterator iter = _conf.CodecList().begin(); 
		iter != _conf.CodecList().end();
		iter++)
	{
		STRCAT_COUNT(sdp_buf,(*iter)->get_sdp_a().c_str());
	}


	STRCAT_COUNT(sdp_buf,"\r\n"); 
	cout << endl << sdp_buf << endl;

#undef IX_MSX_SDP_SIZE
#undef STRCAT_COUNT
	
	return sdp_buf;

}


void 
UASDialogUsageManager::onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
	FUNCTRACKER;
	
	sis->provisional(180);

	// prepare dialog context
	SipDialogContextPtr ctx_ptr(new SipDialogContext());
	ctx_ptr->transaction_type = CCU_UAS;
	ctx_ptr->uas_invite_handle = sis;
	ctx_ptr->stack_handle = GenerateSipHandle();

	_resipHandlesMap[sis->getAppDialog()]= ctx_ptr;
	_refCcuHandlesMap[ctx_ptr->stack_handle]= ctx_ptr;

	LogDebug(">>New UAS SIP Session<< ix stack handle=[" <<  ctx_ptr->stack_handle  << "] msg:-\n" << msg <<"\n");

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

	ResipDialogHandlesMap::iterator iter  = _resipHandlesMap.find(is->getAppDialog());
	SipDialogContextPtr ctx_ptr = (*iter).second;
	
	const SdpContents::Session &s = sdp.session();
	const Data &addr_data = s.connection().getAddress();
	const string addr = addr_data.c_str();

	const SdpContents::Session::Medium &medium = s.media().front();
	int port = medium.port();
	
	
	DECLARE_NAMED_HANDLE_PAIR(call_handler_pair);

	CcuMsgCallOfferedReq *offered = new CcuMsgCallOfferedReq();
	offered->remote_media = CnxInfo(addr,port);
	offered->stack_call_handle = ctx_ptr->stack_handle;

	_ccu_stack._outbound->Send(offered);


	ctx_ptr->call_handler_inbound = call_handler_pair.inbound;

}


void 
UASDialogUsageManager::onConnected(InviteSessionHandle is, const SipMessage& msg)
{
	FUNCTRACKER;

	ResipDialogHandlesMap::iterator iter = _resipHandlesMap.find(is->getAppDialog());
	if (iter == _resipHandlesMap.end())
	{
		LogWarn("Resip dialog handle=[" << is->getAppDialog().getId() << "] >>not found<<. Has user disconnected???");
		return;
	}

	SipDialogContextPtr ctx_ptr = (*iter).second;
	_ccu_stack.SendResponse(
		ctx_ptr->last_user_request, 
		new CcuMsgNewCallConnected());
	
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