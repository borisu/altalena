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
#include "UASAppDialogSet.h"
#include "UASDialogUsageManager.h"
#include "UASAppDialogSetFactory.h"
#include "Logger.h"


namespace ivrworx
{
	UASDialogUsageManager::UASDialogUsageManager(
		IN Configuration &conf,
		IN IwHandlesMap &handles_map,
		IN ResipDialogHandlesMap &resipHandlesMap,
		IN LpHandlePtr events_handle,
		IN DialogUsageManager &dum):
		_conf(conf),
		_resipHandlesMap(resipHandlesMap),
		_refIwHandlesMap(handles_map),
		_eventsHandle(events_handle),
		_dum(dum)
	{

		_sdpVersionCounter = ::GetTickCount();

		
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
			LogWarn("UASDialogUsageManager::UponCallOfferedNack -  iwh:" << ack->stack_call_handle<< " not found. Has caller disconnected already?");
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;
		LogDebug("UASDialogUsageManager::UponCallOfferedNack -  " << LogHandleState(ctx_ptr, ctx_ptr->invite_handle));

		CleanUpCall(ctx_ptr);

		ctx_ptr->uas_invite_handle->end();

	}

	void 
	UASDialogUsageManager::UponCallOfferedAck(IwMessagePtr req)
	{

		FUNCTRACKER;

		shared_ptr<MsgCalOfferedAck> ack = 
			dynamic_pointer_cast<MsgCalOfferedAck>(req);

		IwHandlesMap::iterator iter = _refIwHandlesMap.find(ack->stack_call_handle);
		if (iter == _refIwHandlesMap.end())
		{
			LogWarn("UASDialogUsageManager::UponCallOfferedAck -  iwh:" << ack->stack_call_handle<< " not found. Has caller disconnected already?");
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;
		if (ack->accepted_codecs.empty())
		{
			LogWarn("UASDialogUsageManager::UponCallOfferedAck -  No accepted codec found " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			HangupCall(ctx_ptr);
		}

		LogDebug("UASDialogUsageManager::UponCallOfferedAck -  " << LogHandleState(ctx_ptr, ctx_ptr->invite_handle));

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

		((UASAppDialogSet*)(ctx_ptr->uas_invite_handle->getAppDialogSet().get()))->_makeCallAck
			= req;
		

		ctx_ptr->uas_invite_handle.get()->provideAnswer(sdp);
		ctx_ptr->uas_invite_handle.get()->accept();
	}


	void 
	UASDialogUsageManager::onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
	{
		FUNCTRACKER;

		sis->provisional(100);
		sis->provisional(180);

		// prepare dialog context
		SipDialogContextPtr ctx_ptr(new SipDialogContext());
		ctx_ptr->uas_invite_handle = sis;
		ctx_ptr->stack_handle = GenerateCallHandle();

		_resipHandlesMap[sis->getAppDialog()]= ctx_ptr;
		_refIwHandlesMap[ctx_ptr->stack_handle]= ctx_ptr;

		LogDebug("UASDialogUsageManager::onNewSession - " << LogHandleState(ctx_ptr,ctx_ptr->uas_invite_handle));

	}

	void 
	UASDialogUsageManager::CleanUpCall(IN SipDialogContextPtr ctx_ptr)
	{
		FUNCTRACKER;

		_refIwHandlesMap.erase(ctx_ptr->stack_handle);
		_resipHandlesMap.erase(ctx_ptr->uas_invite_handle->getAppDialog());
	}

#pragma TODO("UASDialogUsageManager::onOffer takes 7 ms - SDP parsing is taking long (almost 4 ms)")

	void 
	UASDialogUsageManager::onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)      
	{
		FUNCTRACKER;

		ResipDialogHandlesMap::iterator ctx_iter  = _resipHandlesMap.find(is->getAppDialog());
		if (ctx_iter == _resipHandlesMap.end())
		{
			LogCrit("UASDialogUsageManager::onOffer - without created context, rsh:" << is.getId());
			throw;
		}

		SipDialogContextPtr ctx_ptr = (*ctx_iter).second;
		ctx_ptr->invite_handle = is;

		LogDebug("UASDialogUsageManager::onOffer - " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));

		const SdpContents::Session &s = sdp.session();
		const Data &addr_data = s.connection().getAddress();
		const string &addr = addr_data.c_str();


		// keep alive session with the same sdp version number
		if (is->isAccepted() && 
			(sdp.session().origin().getVersion() == s.origin().getVersion()))
		{
			LogDebug("UASDialogUsageManager::onOffer - Session timer keep-alive " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
 			is->provideAnswer(is->getLocalSdp());
			return;
		}

		
		if (s.media().empty())
		{
			LogWarn("UASDialogUsageManager::onOffer - Empty medias list " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
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
			LogWarn("UASDialogUsageManager::onOffer - Not found audio connection " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			HangupCall(ctx_ptr);
			return;
		}

		const SdpContents::Session::Medium &medium = *iter;

		int port =	medium.port();

		DECLARE_NAMED_HANDLE_PAIR(call_handler_pair);

		
		// keep alive or hold and reinvite are not accepted currently
		if (is->isAccepted()		  ||
			addr == "0.0.0.0"		  || 
			medium.exists("sendonly") || 
			medium.exists("inactive"))
		{
			LogWarn("UASDialogUsageManager::onOffer - In-dialog offer is not supported, rejecting offer" << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			is->reject(488);
			return;
		}

		MsgCallOfferedReq *offered = new MsgCallOfferedReq();
		offered->remote_media		= CnxInfo(addr,port);
		offered->stack_call_handle	= ctx_ptr->stack_handle;
		offered->call_handler_inbound = call_handler_pair;


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
			_eventsHandle->Send(offered);
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


		GetCurrLightWeightProc()->SendResponse(
			((UASAppDialogSet *)is->getAppDialogSet().get())->_makeCallAck, 
			conn_msg);

		LogDebug("onConnectedConfirmed:: " << LogHandleState(ctx_ptr,is));

	}
	

	void 
	UASDialogUsageManager::HangupCall(SipDialogContextPtr ctx_ptr)
	{
		FUNCTRACKER;

		LogDebug("HangupCall:: " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));

		_refIwHandlesMap.erase(ctx_ptr->stack_handle);
		_resipHandlesMap.erase(ctx_ptr->uas_invite_handle->getAppDialog());
		ctx_ptr->uas_invite_handle->getAppDialogSet()->end();

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