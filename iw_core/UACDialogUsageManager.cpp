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
#include "UACUserProfile.h"
#include "UACAppDialogSet.h"
#include "Logger.h"

namespace ivrworx
{

#pragma warning( push )
#pragma warning( disable : 4267 )

	UACDialogUsageManager::UACDialogUsageManager(
		IN Configuration &conf,
		IN SipStack &sipStack,
		IN IwHandlesMap &ccu_handles_map,
		IN LightweightProcess &ccu_stack):
		DialogUsageManager(sipStack),
			_conf(conf),
			_iwHandlesMap(ccu_handles_map)
		{

			FUNCTRACKER;

			CnxInfo ipAddr = conf.IvrCnxInfo();
		
			string uasUri = "sip:" + conf.From() + "@" + conf.IvrCnxInfo().iptoa();
			_nameAddr = NameAddrPtr(new NameAddr(uasUri.c_str()));


			SharedPtr<MasterProfile> uacMasterProfile = 
				SharedPtr<MasterProfile>(new MasterProfile());
			auto_ptr<ClientAuthManager> uacAuth(new ClientAuthManager);

			setMasterProfile(uacMasterProfile);
			getMasterProfile()->setDefaultFrom(*_nameAddr);
		
			setClientAuthManager(uacAuth);

			setInviteSessionHandler(this);
			setClientRegistrationHandler(this);
			addOutOfDialogHandler(OPTIONS,this);



	}

	void 
	UACDialogUsageManager::HangupCall(SipDialogContextPtr ptr)
	{
		FUNCTRACKER;

		_iwHandlesMap.erase(ptr->stack_handle);
		_resipHandlesMap.erase(ptr->uac_invite_handle->getAppDialog());
		ptr->uac_invite_handle->end();

	

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
			LogWarn("::UponMakeCallAckReq non-existent call");
			return;
		}

		//
		//
		//

		SipDialogContextPtr ctx_ptr = (*iter).second;

		if ( ctx_ptr->uac_invite_handle->isConnected() == false)
		{
			LogCrit("No support for sdp in ACK");
			HangupCall(ctx_ptr);
		}


	}



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
			user_profile = SharedPtr<UserProfile>(getMasterProfile());
			


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

			for (MediaFormatsPtrList::const_iterator iter = _conf.MediaFormats().begin(); 
				iter != _conf.MediaFormats().end(); 
				iter ++)	
			{

				const MediaFormat &media_format = *(*iter);

				medium.addFormat(media_format.sdp_mapping_tos().c_str());

				string rtpmap = media_format.sdp_mapping_tos() + " " + media_format.sdp_name_tos() + "/" + media_format.sampling_rate_tos();
				medium.addAttribute("rtpmap", rtpmap.c_str());
			}


			session.addMedium(medium);
			sdp.session() = session;

			Data encoded(Data::from(sdp));


			//
			// create context
			//
			SipDialogContextPtr ctx_ptr = 
				SipDialogContextPtr(new SipDialogContext());

			UACAppDialogSet * uac_dialog_set = new UACAppDialogSet(*this,ctx_ptr);
			SharedPtr<SipMessage> invite_session = 
				makeInviteSession(
				name_addr, 
				user_profile,
				&sdp, 
				uac_dialog_set); 


			send(invite_session);

			ctx_ptr->stack_handle = GenerateSipHandle();
			ctx_ptr->transaction_type = TXN_TYPE_UAC;
			ctx_ptr->call_handler_inbound = req->call_handler_inbound;
			ctx_ptr->last_user_request = ptr;

			LogDebug("sent INVITE iwh:" << ctx_ptr->stack_handle)

	
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


		//
		// put them in maps
		//
		_resipHandlesMap[s->getAppDialog()]= ctx_ptr;
		_iwHandlesMap[ctx_ptr->stack_handle]= ctx_ptr;

	}

	void 
	UACDialogUsageManager::onTerminated(IN InviteSessionHandle is , IN InviteSessionHandler::TerminatedReason reason, IN const SipMessage* msg) 
	{
		FUNCTRACKER;

		LogInfo("UACDialogUsageManager::onTerminated rsh:" << is.getId()<< ", reason:" << reason);

		SipDialogContextPtr ctx_ptr = ((UACAppDialogSet*)(is->getAppDialogSet().get()))->_ptr;


		// handle is in offering mode
		if (ctx_ptr->uac_invite_handle->isConnected() == false)
		{
			GetCurrLightWeightProc()->SendResponse(
				ctx_ptr->last_user_request,
				new MsgMakeCallNack());
			return;
		}

		MsgCallHangupEvt *hang_up_evt = 
			new MsgCallHangupEvt();

		hang_up_evt->stack_call_handle = ctx_ptr->stack_handle;

		ctx_ptr->call_handler_inbound->Send(hang_up_evt);

		//
		// remove from maps
		//
		_resipHandlesMap.erase(is->getAppDialog());
		_iwHandlesMap.erase(ctx_ptr->stack_handle);

	}

	void 
	UACDialogUsageManager::onConnectedConfirmed(InviteSessionHandle handle, const SipMessage& msg)
	{

	}

	void 
	UACDialogUsageManager::onConnected(IN ClientInviteSessionHandle is, IN const SipMessage& msg)
	{
		FUNCTRACKER;

		ResipDialogHandlesMap::iterator iter = 
			_resipHandlesMap.find(is->getAppDialog());

		if (iter == _resipHandlesMap.end())
		{
			LogWarn("::onConnected non-existent call");
			is->end();
			return;
		}


		SipDialogContextPtr ctx_ptr = (*iter).second;

		MsgMakeCallOk *ack = 
			new MsgMakeCallOk();

		ack->stack_call_handle = ctx_ptr->stack_handle;

		const resip::SdpContents sdp = 
			is->getRemoteSdp();
		const SdpContents::Session &s = sdp.session();

		CnxInfo data = 
			ExtractCnxInfo(sdp);
		ack->remote_media = data;


		if (s.media().empty())
		{
			LogWarn("::onConnected Empty proposed medias list " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			HangupCall(ctx_ptr);
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
			LogWarn("::onConnected Not found audio connection " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			HangupCall(ctx_ptr);
			return;
		}

		const SdpContents::Session::Medium &medium = *medium_iter;

		

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


		GetCurrLightWeightProc()->SendResponse(
			ctx_ptr->last_user_request,
			ack);

			
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

}