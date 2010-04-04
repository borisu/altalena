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
#include "IwBase.h"
#include "LightweightProcess.h"
#include "ResipCommon.h"
#include "UacDialogUsageManager.h"
#include "UACAppDialogSet.h"
#include "Logger.h"


namespace ivrworx
{

#pragma warning( push )
#pragma warning( disable : 4267 )

	UACDialogUsageManager::UACDialogUsageManager(
		IN Configuration &conf,
		IN IwHandlesMap &ccu_handles_map,
		IN ResipDialogHandlesMap &resip_handles_map,
		IN DialogUsageManager &dum):
		_conf(conf),
		_dum(dum),
		_iwHandlesMap(ccu_handles_map),
		_resipHandlesMap(resip_handles_map)
	{
		FUNCTRACKER;

	}

	void 
	UACDialogUsageManager::UponHangupReq(IN IwMessagePtr ptr)
	{
		FUNCTRACKER;

		shared_ptr<MsgHangupCallReq> hangup_msg = 
			dynamic_pointer_cast<MsgHangupCallReq>(ptr);

		IwStackHandle handle = hangup_msg->stack_call_handle;

		LogDebug("UACDialogUsageManager::UponHangupReq iwh:" << hangup_msg->stack_call_handle);

		IwHandlesMap::iterator iter = _iwHandlesMap.find(handle);
		if (iter == _iwHandlesMap.end())
		{
			LogDebug("UACDialogUsageManager::UponHangupReq - the call iwh:" << hangup_msg->stack_call_handle << " already hanged up.");
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;

		_iwHandlesMap.erase(ctx_ptr->stack_handle);
		if (ctx_ptr->uac_invite_handle.isValid())
		{
			_resipHandlesMap.erase(ctx_ptr->uac_invite_handle->getAppDialog());
			ctx_ptr->uac_invite_handle->getAppDialogSet()->end();
		}

		if (ctx_ptr->uas_invite_handle.isValid())
		{
			_resipHandlesMap.erase(ctx_ptr->uas_invite_handle->getAppDialog());
			ctx_ptr->uas_invite_handle->getAppDialogSet()->end();
		}
		

	}

	void 
	UACDialogUsageManager::CleanUpCall(IN SipDialogContextPtr ctx_ptr)
	{
		FUNCTRACKER;

		_iwHandlesMap.erase(ctx_ptr->stack_handle);
		if (ctx_ptr->uac_invite_handle.isValid())
		{
			_resipHandlesMap.erase(ctx_ptr->uac_invite_handle->getAppDialog());
			ctx_ptr->uac_invite_handle->getAppDialogSet()->end();
		}
		if (ctx_ptr->uas_invite_handle.isValid())
		{
			_resipHandlesMap.erase(ctx_ptr->uas_invite_handle->getAppDialog());
			ctx_ptr->uas_invite_handle->getAppDialogSet()->end();
		}
		
	}

	void 
	UACDialogUsageManager::UponMakeCallAckReq(IN IwMessagePtr ptr)
	{
		FUNCTRACKER;
	
		shared_ptr<MsgMakeCallAckReq> ack_req  = 
			shared_dynamic_cast<MsgMakeCallAckReq>(ptr);

		IwHandlesMap::iterator iter = 
			_iwHandlesMap.find(ack_req->stack_call_handle);

		if (iter == _iwHandlesMap.end())
		{
			LogWarn("UACDialogUsageManager::UponMakeCallAckReq - non-existent call iwh:" << ack_req->stack_call_handle);
			return;
		}

		//
		//
		//

		SipDialogContextPtr ctx_ptr = (*iter).second;

		if ( ctx_ptr->uac_invite_handle->isConnected() == false)
		{
			LogCrit("UACDialogUsageManager::UponMakeCallAckReq - no support for sdp in ACK");
			throw;
		}


	}

	

	#pragma TODO ("UACDialogUsageManager::UponMakeCallReq takes 20 ms - SDP preparation should be replaced with simple string concatenation")

	void 
	UACDialogUsageManager::UponMakeCallReq(IN IwMessagePtr ptr)
	{
		FUNCTRACKER;

		try {

			MsgMakeCallReq *req  = 
				shared_dynamic_cast<MsgMakeCallReq>(ptr).get();

			//
			// prepare headers
			//
			NameAddr name_addr(req->destination_uri.c_str());
			SharedPtr<UserProfile> user_profile;
			user_profile = SharedPtr<UserProfile>(_dum.getMasterProfile());
			
			//
			// prepare sdp
			//
			SdpContents sdp;

			unsigned long tm = ::GetTickCount();
			unsigned long sessionId((unsigned long) tm);

			SdpContents::Session::Origin origin(
				"-", 
				sessionId, 
				sessionId, 
				SdpContents::IP4, 
				req->local_media.iptoa());

			SdpContents::Session session(0, origin, "ivrworks session");

			session.connection() = 
				SdpContents::Session::Connection(SdpContents::IP4, req->local_media.iptoa());
			session.addTime(SdpContents::Session::Time(tm, 0));

			SdpContents::Session::Medium medium("audio", req->local_media.port_ho(), 0, "RTP/AVP");

			
			
			const MediaFormat *dtmf_format = NULL;

			string str_buffer(1024,'\0');
			

			ListOfAny codecs_list;
			_conf.GetArray("codecs",codecs_list);

			for (ListOfAny::iterator conf_iter = codecs_list.begin(); 
				conf_iter != codecs_list.end(); 
				conf_iter++)
			{

				string conf_codec_name =  any_cast<string>(*conf_iter);
				MediaFormat media_format = MediaFormat::GetMediaFormat(conf_codec_name);
			
				if (MediaFormat::GetMediaType(media_format.sdp_name_tos()) == MediaFormat::MediaType_DTMF)
				{
					dtmf_format = &media_format;
					continue;
				}

				medium.addFormat(media_format.sdp_mapping_tos().c_str());

				str_buffer.clear();
				str_buffer += media_format.sdp_mapping_tos() ;
				str_buffer +=" "; 
				str_buffer += media_format.sdp_name_tos(); 
				str_buffer += "/"; 
				str_buffer += media_format.sampling_rate_tos();

				medium.addAttribute("rtpmap", str_buffer.c_str());

			}

			

			// dtmf format always last
			if (dtmf_format != NULL)
			{
				medium.addFormat(dtmf_format->sdp_mapping_tos().c_str());

				str_buffer.clear();
				str_buffer += dtmf_format->sdp_mapping_tos();
				str_buffer += " "; 
				str_buffer += dtmf_format->sdp_name_tos();
				str_buffer += "/";
				str_buffer += dtmf_format->sampling_rate_tos();
				medium.addAttribute("rtpmap", str_buffer.c_str());


				str_buffer.clear();
				str_buffer += dtmf_format->sdp_mapping_tos();
				str_buffer += " 0-16";

				medium.addAttribute("fmtp", (str_buffer).c_str());
			}


			medium.addAttribute("sendrecv");
			session.addMedium(medium);
			
			sdp.session() = session;

			Data encoded(Data::from(sdp));


			//
			// create context
			//
			SipDialogContextPtr ctx_ptr = 
				SipDialogContextPtr(new SipDialogContext());
			

			UACAppDialogSet * uac_dialog_set = new UACAppDialogSet(_dum,ctx_ptr,ptr);
			SharedPtr<SipMessage> invite_session; 
			invite_session = _dum.makeInviteSession(
				name_addr, 
				user_profile,
				&sdp, 
				uac_dialog_set); 

			
			_dum.send(invite_session);

			
			ctx_ptr->stack_handle = req->stack_call_handle == IW_UNDEFINED ? 
				GenerateSipHandle() : req->stack_call_handle;

			ctx_ptr->call_handler_inbound = req->call_handler_inbound;

			LogDebug("UACDialogUsageManager::UponMakeCallReq - INVITE iwh:" << ctx_ptr->stack_handle)

	
		} 
		catch (BaseException &e)
		{
			LogWarn("Exception while issuing invite:" << e.getMessage().c_str());
			GetCurrLightWeightProc()->SendResponse(ptr, new MsgMakeCallNack());
		}
	}

	void 
	UACDialogUsageManager::onNewSession(
		IN ClientInviteSessionHandle s, 
		IN InviteSession::OfferAnswerType oat, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;


		SipDialogContextPtr ctx_ptr = ((UACAppDialogSet*)(s->getAppDialogSet().get()))->_ptr;
		ctx_ptr->uac_invite_handle = s;

		
		ctx_ptr->invite_handle =_dum.findInviteSession(s->getAppDialog()->getDialogId());
		if (!ctx_ptr->invite_handle.isValid())
		{
			LogWarn("UACDialogUsageManager::onNewSession - Invalid invite handle");
		}

		
		//
		// put them in maps
		//
		_resipHandlesMap[s->getAppDialog()]= ctx_ptr;
		_iwHandlesMap[ctx_ptr->stack_handle]= ctx_ptr;

	}

	void
	UACDialogUsageManager::UponBlindXferReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		shared_ptr<MsgCallBlindXferReq> xfer_req = 
			dynamic_pointer_cast<MsgCallBlindXferReq>(req);

		IwHandlesMap::iterator iter = _iwHandlesMap.find(xfer_req->stack_call_handle);
		if (iter == _iwHandlesMap.end())
		{
			LogWarn("UACDialogUsageManager::UponBlindXferReq - iwh:" << xfer_req->stack_call_handle << " not found. Has caller disconnected already?");
			GetCurrLightWeightProc()->SendResponse(
				req,
				new MsgCallBlindXferNack());
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;

		InviteSessionHandle invite_handle_to_replace = ctx_ptr->invite_handle;
		

		if (!invite_handle_to_replace.isValid() || invite_handle_to_replace->isConnected() == false)
		{
			LogWarn("UACDialogUsageManager::UponBlindXferReq - Cannot xfer in not connected state, " << LogHandleState(ctx_ptr, invite_handle_to_replace))
				GetCurrLightWeightProc()->SendResponse(
				req,
				new MsgCallBlindXferNack());

			return;
		}


		LogDebug("UACDialogUsageManager::UponBlindXferReq - dst:" << xfer_req->destination_uri << ", " << LogHandleState(ctx_ptr, invite_handle_to_replace));

		NameAddr name_addr(xfer_req->destination_uri.c_str());
		invite_handle_to_replace->refer(name_addr,false);

		GetCurrLightWeightProc()->SendResponse(
			req,
			new MsgCallBlindXferAck());

		invite_handle_to_replace->end();

	}

	/// Received a failure response from UAS
	void 
	UACDialogUsageManager::onFailure(
		IN ClientInviteSessionHandle is, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;

		

		UACAppDialogSet* uac_set = (UACAppDialogSet*)(is->getAppDialogSet().get());

		SipDialogContextPtr ctx_ptr = uac_set->_ptr;

		LogWarn("UACDialogUsageManager::onFailure rsh:" << is.getId() << ", iwh:" << ctx_ptr->stack_handle);

		switch ((uac_set)->_orig_request->message_id)
		{
		case MSG_MAKE_CALL_REQ:
			{
				GetCurrLightWeightProc()->SendResponse(
					(uac_set)->_orig_request,
					new MsgMakeCallNack());
				CleanUpCall(ctx_ptr);
				break;
			}
		default:
			{
				LogWarn("UACDialogUsageManager::onFailure Unknown req:"  << (uac_set)->_orig_request->message_id);
			}
		}

	}

#pragma TODO ("UACDialogUsageManager::onConnected takes 4 ms - SDP inspections is taking long")
	void 
	UACDialogUsageManager::onConnected(IN ClientInviteSessionHandle is, IN const SipMessage& msg)
	{
		FUNCTRACKER;
	
		ResipDialogHandlesMap::iterator iter = 
			_resipHandlesMap.find(is->getAppDialog());

		if (iter == _resipHandlesMap.end())
		{
			LogWarn("UACDialogUsageManager::onConnected non-existent call");
			is->end();
			return;
		}


		SipDialogContextPtr ctx_ptr = (*iter).second;

		MsgMakeCallOk *ack = 
			new MsgMakeCallOk();

		ack->stack_call_handle = ctx_ptr->stack_handle;

		const resip::SdpContents &sdp = 
			is->getRemoteSdp();
		const SdpContents::Session &s = sdp.session();
		const Data &addr_data = s.connection().getAddress();
	

		if (s.media().empty())
		{
			LogWarn("UACDialogUsageManager::onConnected Empty proposed medias list " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			CleanUpCall(ctx_ptr);
			return;
		}


		

		// currently we only "audio" conversation
		list<SdpContents::Session::Medium>::const_iterator medium_iter = s.media().begin();
		for (;medium_iter != s.media().end();medium_iter++)
		{
			const SdpContents::Session::Medium &curr_medium = (*medium_iter);
			if (_stricmp("audio",curr_medium.name().c_str()) == 0)
			{
				break;
			}
		}

		if (medium_iter == s.media().end())
		{
			LogWarn("UACDialogUsageManager::onConnected Not found audio connection " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			CleanUpCall(ctx_ptr);
			return;
		}

		

		const SdpContents::Session::Medium &medium = *medium_iter;

		int port =	medium.port();
		ack->remote_media = CnxInfo(addr_data.c_str(),port);

		// send list of codecs to the main process
		const list<Codec> &offered_codecs = medium.codecs();
		for (list<Codec>::const_iterator codec_iter = offered_codecs.begin(); 
			codec_iter != offered_codecs.end(); 
			codec_iter++)
		{

			ack->offered_codecs.push_front(
				MediaFormat(
				codec_iter->getName().c_str(),
				codec_iter->getRate(),
				codec_iter->payloadType()));
		}

		UACAppDialogSet* uac_set = (UACAppDialogSet*)(is->getAppDialogSet().get());

		const Uri &from_uri = msg.header(h_From).uri();
		ack->ani = from_uri.user().c_str();


		GetCurrLightWeightProc()->SendResponse(
			uac_set->_orig_request,
			ack);

			
	}

	UACDialogUsageManager::~UACDialogUsageManager(void)
	{
		FUNCTRACKER;

	}



#pragma warning( pop )

}