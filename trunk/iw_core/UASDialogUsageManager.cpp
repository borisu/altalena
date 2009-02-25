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
#include "Logger.h"
#include "Profiler.h"

namespace ivrworx
{



	UASDialogUsageManager::UASDialogUsageManager(
		IN Configuration &conf,
		IN SipStack &resip_stack, 
		IN IwHandlesMap &handles_map,
		IN LightweightProcess &stack):
	DialogUsageManager(resip_stack),
		_conf(conf),
		_refIwHandlesMap(handles_map),
		_sipStack(stack)
	{

		_sdpVersionCounter = ::GetTickCount();

		CnxInfo ipAddr = conf.IvrCnxInfo();

		string uasUri = "sip:" + conf.From() + "@" + ipAddr.ipporttos();
		_uasAor		= NameAddrPtr(new NameAddr(uasUri.c_str()));

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
		UASDialogUsageManager::UponCallOfferedNack(IwMessagePtr req)
	{
		FUNCTRACKER;

		shared_ptr<MsgCallOfferedNack> ack = 
			dynamic_pointer_cast<MsgCallOfferedNack>(req);

		IwHandlesMap::iterator iter = _refIwHandlesMap.find(ack->stack_call_handle);
		if (iter == _refIwHandlesMap.end())
		{
			LogWarn("ix handle=[" << ack->stack_call_handle<< "] not found. Has caller disconnected already?");
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;
		LogDebug("Ix rejected the call eith ix handle=[" << ctx_ptr->stack_handle << "].");

		CleanUpCall(ctx_ptr);

		IX_PROFILE_CODE(ctx_ptr->uas_invite_handle->end());

	}

	void 
		UASDialogUsageManager::UponCallOfferedAck(IwMessagePtr req)
	{

		shared_ptr<MsgCalOfferedlAck> ack = 
			dynamic_pointer_cast<MsgCalOfferedlAck>(req);

		IwHandlesMap::iterator iter = _refIwHandlesMap.find(ack->stack_call_handle);
		if (iter == _refIwHandlesMap.end())
		{
			LogWarn("call handle " << ack->stack_call_handle<< " not found (caller disconnected?).");
			return;
		}

		SipDialogContextPtr ptr = (*iter).second;
		ptr->last_user_request = req;

		if (ack->accepted_codecs.empty())
		{
			LogWarn("No accepted codec found, call handle = " << ack->stack_call_handle);
			HangupCall(ptr);
		}

		SdpContents sdp;

		unsigned long tm = ::GetTickCount();
		unsigned long sessionId((unsigned long) tm);


		SdpContents::Session::Origin origin("-", sessionId, sessionId, SdpContents::IP4, ack->local_media.iptoa());
		SdpContents::Session session(0, origin, "ivrworks session");

		session.connection() = SdpContents::Session::Connection(SdpContents::IP4, ack->local_media.iptoa());
		session.addTime(SdpContents::Session::Time(tm, 0));

		SdpContents::Session::Medium medium("audio", ack->local_media.port_ho(), 0, "RTP/AVP");

		for (MediaFormatsList::iterator iter = ack->accepted_codecs.begin(); 
			iter != ack->accepted_codecs.end(); iter ++)	
		{

			MediaFormat &media_format = *iter;

			medium.addFormat(media_format.sdp_mapping_tos().c_str());

			string rtpmap = media_format.sdp_mapping_tos() + " " + media_format.sdp_name_tos() + "/" + media_format.sampling_rate_tos();
			medium.addAttribute("rtpmap", rtpmap.c_str());
		}

		session.addMedium(medium);
		sdp.session() = session;

		Data encoded(Data::from(sdp));
		// 
		// 	HeaderFieldValue hfv(encoded.c_str(),encoded.size());
		// 	Mime type("application", "sdp");
		// 	SdpContents sdp(&hfv, type);


		IX_PROFILE_CODE(ptr->uas_invite_handle.get()->provideAnswer(sdp));
		IX_PROFILE_CODE(ptr->uas_invite_handle.get()->accept());
	}


	void 
		UASDialogUsageManager::onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
	{
		FUNCTRACKER;

		sis->provisional(100);
		sis->provisional(180);

		// prepare dialog context
		SipDialogContextPtr ctx_ptr(new SipDialogContext());
		ctx_ptr->transaction_type = TXN_TYPE_UAS;
		ctx_ptr->uas_invite_handle = sis;
		ctx_ptr->stack_handle = GenerateSipHandle();

		_resipHandlesMap[sis->getAppDialog()]= ctx_ptr;
		_refIwHandlesMap[ctx_ptr->stack_handle]= ctx_ptr;

		LogDebug("New session created - stack ix handle=[" <<  ctx_ptr->stack_handle  << "], sip callid=[" << sis->getCallId().c_str() << "], resip handle=[" << sis.getId() << "]");

	}

	void 
		UASDialogUsageManager::CleanUpCall(IN SipDialogContextPtr ctx_ptr)
	{
		_refIwHandlesMap.erase(ctx_ptr->stack_handle);
		_resipHandlesMap.erase(ctx_ptr->uas_invite_handle->getAppDialog());
	}


	void 
		UASDialogUsageManager::onTerminated(InviteSessionHandle handle , InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
	{
		FUNCTRACKER;


		IwStackHandle ixhandle = IW_UNDEFINED;
		ResipDialogHandlesMap::iterator iter  = _resipHandlesMap.find(handle->getAppDialog());
		if (iter != _resipHandlesMap.end())
		{
			// early termination
			SipDialogContextPtr ctx_ptr = (*iter).second;
			ixhandle = ctx_ptr->stack_handle;
			ctx_ptr->call_handler_inbound->Send(new MsgCallHangupEvt());

			CleanUpCall(ctx_ptr);

		}


		LogDebug("Call terminated - stack ix handle=[" <<  ixhandle  << "], sip callid=[" << handle->getCallId().c_str() << "], resip handle=[" << handle.getId() << "]");

	}

	void 
		UASDialogUsageManager::onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)      
	{
		FUNCTRACKER;

		ResipDialogHandlesMap::iterator ctx_iter  = _resipHandlesMap.find(is->getAppDialog());
		if (ctx_iter == _resipHandlesMap.end())
		{
			LogCrit("'Offered' without created context, handle=[" << is.getId() << "]");
			throw;
		}
		SipDialogContextPtr ctx_ptr = (*ctx_iter).second;


		LogDebug("Call offered - stack ix handle=[" <<  ctx_ptr->stack_handle  << "], sip callid=[" << is->getCallId().c_str() << "], resip handle=[" << is.getId() << "]");

		const SdpContents::Session &s = sdp.session();
		const Data &addr_data = s.connection().getAddress();
		const string addr = addr_data.c_str();

		if (s.media().empty())
		{
			LogWarn("Empty medias list - stack ix handle=[" <<  ctx_ptr->stack_handle  << "], sip callid=[" << is->getCallId().c_str() << "], resip handle=[" << is.getId() << "]");
			HangupCall(ctx_ptr);
			return;
		}

		// currently we support single audio argument
		list<SdpContents::Session::Medium>::const_iterator iter = s.media().begin();
		for (;iter != s.media().end();iter++)
		{
			const SdpContents::Session::Medium &curr_medium = (*iter);
			if (_stricmp("audio",curr_medium.name().c_str()) == 0)
			{
				break;
			}
		}

		if (iter == s.media().end())
		{
			LogWarn("Not found audio connection - stack ix handle=[" <<  ctx_ptr->stack_handle  << "], sip callid=[" << is->getCallId().c_str() << "], resip handle=[" << is.getId() << "]");
			HangupCall(ctx_ptr);
			return;
		}

		const SdpContents::Session::Medium &medium = *iter;

		int port =	medium.port();

		DECLARE_NAMED_HANDLE_PAIR(call_handler_pair);

		MsgCallOfferedReq *offered = new MsgCallOfferedReq();
		offered->remote_media		= CnxInfo(addr,port);
		offered->stack_call_handle	= ctx_ptr->stack_handle;
		offered->call_handler_inbound = call_handler_pair;


		// send list of codecs to the main process
		const list<Codec> &offered_codecs = medium.codecs();
		for (list<Codec>::const_iterator codec_iter = offered_codecs.begin(); 
			codec_iter != offered_codecs.end(); codec_iter++)
		{

			offered->offered_codecs.push_front(
				MediaFormat(
				codec_iter->getName().c_str(),
				codec_iter->getRate(),
				codec_iter->payloadType()));
		}


		_sipStack._outbound->Send(offered);

		ctx_ptr->call_handler_inbound = call_handler_pair.inbound;



	}


	void 
		UASDialogUsageManager::onConnected(InviteSessionHandle is, const SipMessage& msg)
	{
		FUNCTRACKER;

		ResipDialogHandlesMap::iterator ctx_iter  = _resipHandlesMap.find(is->getAppDialog());
		if (ctx_iter == _resipHandlesMap.end())
		{
			LogCrit("'Connected' without context, handle=[" << is.getId() << "]");
			is->end();
			return;
		}

		SipDialogContextPtr ctx_ptr = (*ctx_iter).second;

		MsgNewCallConnected *conn_msg = 
			new MsgNewCallConnected();

		_sipStack.SendResponse(
			ctx_ptr->last_user_request, 
			conn_msg);

		LogDebug("Call connected - stack ix handle=[" <<  ctx_ptr->stack_handle  << "], sip callid =[" << is->getCallId().c_str() << "], resip handle=[" << is.getId() << "]");

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


	ApiErrorCode 
		UASDialogUsageManager::HangupCall(SipDialogContextPtr ptr)
	{
		FUNCTRACKER;


		_refIwHandlesMap.erase(ptr->stack_handle);
		_resipHandlesMap.erase(ptr->uas_invite_handle->getAppDialog());
		ptr->uas_invite_handle->end();

		return API_SUCCESS;

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

}