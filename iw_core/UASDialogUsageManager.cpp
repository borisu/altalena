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

	
	template<class T> string LogHandleState(SipDialogContextPtr ptr, T is)
	{

		stringstream stream;
		stream << "iwh:" << (ptr.get() == NULL ? -1 : ptr->stack_handle) << ", rsh:"  << is.getId() <<  ", callid:" << is->getCallId().c_str() << ", state:";

		if (is->isAccepted())
		{
			stream << "accepted;";
		}
		if (is->isConnected())
		{
			stream << "connected;";
		}
		if (is->isEarly())
		{
			stream << "early;";
		}
		if (is->isTerminated())
		{
			stream << "terminated;";
		}

		return stream.str();

	}
	
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
		addClientSubscriptionHandler("refer",this);
		
		addOutOfDialogHandler(OPTIONS, this);


		auto_ptr<AppDialogSetFactory> uas_dsf(new UASAppDialogSetFactory());
		setAppDialogSetFactory(uas_dsf);
		uas_dsf.release();

		LogInfo("UAS started on " << ipAddr.ipporttos());
	}

	UASDialogUsageManager::~UASDialogUsageManager(void)
	{
	}

	void
    UASDialogUsageManager::UponBlindXferReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		shared_ptr<MsgCallBlindXferReq> xfer_req = 
			dynamic_pointer_cast<MsgCallBlindXferReq>(req);

		IwHandlesMap::iterator iter = _refIwHandlesMap.find(xfer_req->stack_call_handle);
		if (iter == _refIwHandlesMap.end())
		{
			LogWarn("UponBlindXferReq:: iwh:" << xfer_req->stack_call_handle << " not found. Has caller disconnected already?");
			GetCurrLightWeightProc()->SendResponse(
				req,
				new MsgCallBlindXferNack());
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;
		InviteSessionHandle invite_handle = ctx_ptr->invite_handle;

		if (invite_handle->isConnected() == false)
		{
			LogWarn("Cannot xfer in not connected state, " << LogHandleState(ctx_ptr, invite_handle))
			GetCurrLightWeightProc()->SendResponse(
				req,
				new MsgCallBlindXferNack());

			return;
		}

		
		LogDebug("UponBlindXferReq:: dst:" << xfer_req->destination_uri << ", " << LogHandleState(ctx_ptr, invite_handle));

		NameAddr name_addr(xfer_req->destination_uri.c_str());
		invite_handle->refer(name_addr,false);

		GetCurrLightWeightProc()->SendResponse(
			req,
			new MsgCallBlindXferAck());

		invite_handle->end();

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
			LogWarn("UponCallOfferedNack:: iwh:" << ack->stack_call_handle<< " not found. Has caller disconnected already?");
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;
		LogDebug("UponCallOfferedNack:: " << LogHandleState(ctx_ptr, ctx_ptr->invite_handle));

		CleanUpCall(ctx_ptr);

		IX_PROFILE_CODE(ctx_ptr->uas_invite_handle->end());

	}

	void 
	UASDialogUsageManager::UponCallOfferedAck(IwMessagePtr req)
	{

		shared_ptr<MsgCalOfferedAck> ack = 
			dynamic_pointer_cast<MsgCalOfferedAck>(req);

		IwHandlesMap::iterator iter = _refIwHandlesMap.find(ack->stack_call_handle);
		if (iter == _refIwHandlesMap.end())
		{
			LogWarn("UponCallOfferedAck:: iwh:" << ack->stack_call_handle<< " not found. Has caller disconnected already?");
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;
		ctx_ptr->last_user_request = req;

		if (ack->accepted_codecs.empty())
		{
			LogWarn("UponCallOfferedAck:: No accepted codec found " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			HangupCall(ctx_ptr);
		}

		LogDebug("UponCallOfferedAck:: " << LogHandleState(ctx_ptr, ctx_ptr->invite_handle));

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

		
		if (ack->invite_type == OFFER_TYPE_HOLD)
		{
			medium.addAttribute("inactive");
		}
		

		session.addMedium(medium);
		sdp.session() = session;

		Data encoded(Data::from(sdp));
		

		IX_PROFILE_CODE(ctx_ptr->uas_invite_handle.get()->provideAnswer(sdp));
		IX_PROFILE_CODE(ctx_ptr->uas_invite_handle.get()->accept());
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

		LogDebug("onNewSession:: " << LogHandleState(ctx_ptr,ctx_ptr->uas_invite_handle));

	}

	void 
	UASDialogUsageManager::CleanUpCall(IN SipDialogContextPtr ctx_ptr)
	{
		FUNCTRACKER;

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

			LogDebug("onTerminated:: " << LogHandleState(ctx_ptr,handle));
			CleanUpCall(ctx_ptr);

		} 
		else
		{
			LogDebug("onTerminated:: " << LogHandleState(SipDialogContextPtr((SipDialogContext*)NULL),handle));
		}

	}

	void 
	UASDialogUsageManager::onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)      
	{
		FUNCTRACKER;

		ResipDialogHandlesMap::iterator ctx_iter  = _resipHandlesMap.find(is->getAppDialog());
		if (ctx_iter == _resipHandlesMap.end())
		{
			LogCrit("onOffer:: without created context, rsh:" << is.getId());
			throw;
		}

		SipDialogContextPtr ctx_ptr = (*ctx_iter).second;
		ctx_ptr->invite_handle = is;

		LogDebug("onOffer::" << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));


		const SdpContents::Session &s = sdp.session();
		const Data &addr_data = s.connection().getAddress();
		const string addr = addr_data.c_str();

		
		if (s.media().empty())
		{
			LogWarn("onOffer:: Empty medias list " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			HangupCall(ctx_ptr);
			return;
		}

		// currently we only "audio" conversation
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
			LogWarn("onOffer:: Not found audio connection " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
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

		// keep alive or hold and reinvite are not accepted currently
		if (is->isAccepted()		  ||
			addr == "0.0.0.0"		  || 
			medium.exists("sendonly") || 
			medium.exists("inactive"))
		{
			LogWarn("onOffer:: In-dialog offer is not supported, rejecting offer" << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			is->reject(488);
			return;
		}

		const Uri &to_uri = msg.header(h_To).uri();
		offered->dnis = to_uri.user().c_str();

		const Uri &from_uri = msg.header(h_From).uri();
		offered->ani = from_uri.user().c_str();


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


		if (offered->is_indialog || 
			offered->invite_type == ivrworx::OFFER_TYPE_HOLD ||
			offered->invite_type == ivrworx::OFFER_TYPE_RESUME)
		{
			ctx_ptr->call_handler_inbound->Send(offered);
		} 
		else
		{
			_sipStack._outbound->Send(offered);
		}
		

		ctx_ptr->call_handler_inbound = call_handler_pair.inbound;


	}

	void
	UASDialogUsageManager::onConnectedConfirmed(InviteSessionHandle is, const SipMessage &msg)
	{
		FUNCTRACKER;

		ResipDialogHandlesMap::iterator ctx_iter  = _resipHandlesMap.find(is->getAppDialog());
		if (ctx_iter == _resipHandlesMap.end())
		{
			LogCrit("onConnectedConfirmed:: without context, rsh:" << is.getId());
			is->end();
			return;
		}

		SipDialogContextPtr ctx_ptr = (*ctx_iter).second;
		ctx_ptr->invite_handle = is;

		MsgNewCallConnected *conn_msg = 
			new MsgNewCallConnected();

		_sipStack.SendResponse(
			ctx_ptr->last_user_request, 
			conn_msg);

		LogDebug("onConnectedConfirmed:: " << LogHandleState(ctx_ptr,is));

	}
	


	void 
	UASDialogUsageManager::onConnected(InviteSessionHandle is, const SipMessage& msg)
	{
		FUNCTRACKER;
	
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
	UASDialogUsageManager::HangupCall(SipDialogContextPtr ctx_ptr)
	{
		FUNCTRACKER;

		LogDebug("HangupCall:: " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));

		_refIwHandlesMap.erase(ctx_ptr->stack_handle);
		_resipHandlesMap.erase(ctx_ptr->uas_invite_handle->getAppDialog());
		ctx_ptr->uas_invite_handle->end();

		return API_SUCCESS;
	}

	void 
	UASDialogUsageManager::onUpdatePending(ClientSubscriptionHandle sh, const SipMessage& notify, bool outOfOrder)
	{
		FUNCTRACKER;

// 		ResipDialogHandlesMap::iterator ctx_iter  = _resipHandlesMap.find(sh->getAppDialog());
// 		if (ctx_iter == _resipHandlesMap.end())
// 		{
// 			LogCrit("Received UPDATE (pending) without application context, rsh:" << sh.getId());
// 			sh->rejectUpdate();
// 			sh->end();
// 			return;
// 		}

		
	}

	void 
	UASDialogUsageManager::onUpdateActive(ClientSubscriptionHandle sh, const SipMessage& notify, bool outOfOrder)
	{
// 		ResipDialogHandlesMap::iterator ctx_iter  = _resipHandlesMap.find(sh->getAppDialog());
// 		if (ctx_iter == _resipHandlesMap.end())
// 		{
// 			LogCrit("Received UPDATE (active) without application context, rsh:" << sh.getId());
// 			sh->rejectUpdate();
// 			sh->end();
// 			return;
// 		}

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