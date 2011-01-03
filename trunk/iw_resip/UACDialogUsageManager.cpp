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
#include "FreeContent.h"


namespace ivrworx
{

#pragma warning( push )
#pragma warning( disable : 4267 )

	UACDialogUsageManager::UACDialogUsageManager(
		IN ConfigurationPtr conf,
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
	UACDialogUsageManager::UponInfoReq(IN IwMessagePtr ptr)
	{
		FUNCTRACKER;

		shared_ptr<MsgSipCallInfoReq> options_req = 
			dynamic_pointer_cast<MsgSipCallInfoReq>(ptr);

		IwStackHandle handle = options_req->stack_call_handle;

		LogDebug("UACDialogUsageManager:UponInfoReq iwh:" << options_req->stack_call_handle);

		IwHandlesMap::iterator iter = _iwHandlesMap.find(handle);
		if (iter == _iwHandlesMap.end())
		{
			LogDebug("UACDialogUsageManager:UponInfoReq - the call iwh:" << options_req->stack_call_handle << " ctx not found.");
			GetCurrRunningContext()->SendResponse(ptr, new MsgSipCallInfoNack());
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;

		SharedPtr<UserProfile> user_profile;
		user_profile = SharedPtr<UserProfile>(_dum.getMasterProfile());

		FreeContent fc(options_req->localOffer.body, options_req->localOffer.type);
		ctx_ptr->invite_handle->info(fc);

		UACAppDialogSet* uac_set = (UACAppDialogSet*)(ctx_ptr->invite_handle->getAppDialogSet().get());
		uac_set->last_options_req = ptr;


	}



	/// called when response to INFO message is received 
	void 
	UACDialogUsageManager::onInfoSuccess(InviteSessionHandle is, const SipMessage& msg)
	{
		FUNCTRACKER;

		UACAppDialogSet* uac_set = (UACAppDialogSet*)(is->getAppDialogSet().get());

		SipDialogContextPtr ctx_ptr = uac_set->_ptr;

		LogDebug("UACDialogUsageManager::onInfoSuccess rsh:" << is.getId() << ", iwh:" << ctx_ptr->stack_handle);

		if (!(uac_set)->last_options_req)
		{
			return;
		}


		GetCurrRunningContext()->SendResponse(
			(uac_set)->last_options_req,
			new MsgSipCallInfoAck());

		(uac_set)->last_options_req.reset();

	}

	void UACDialogUsageManager::onInfoFailure(InviteSessionHandle is, const SipMessage& msg)
	{
		FUNCTRACKER;

		UACAppDialogSet* uac_set = (UACAppDialogSet*)(is->getAppDialogSet().get());

		SipDialogContextPtr ctx_ptr = uac_set->_ptr;

		LogWarn("UACDialogUsageManager::onInfoFailure rsh:" << is.getId() << ", iwh:" << ctx_ptr->stack_handle);

		if (!(uac_set)->last_options_req)
		{
			return;
		}
		
		GetCurrRunningContext()->SendResponse(
			(uac_set)->last_options_req,
			new MsgSipCallInfoNack());

		(uac_set)->last_options_req.reset();
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
	UACDialogUsageManager::UponCallConnected(IN IwMessagePtr ptr)
	{
		FUNCTRACKER;
	
		shared_ptr<MsgNewCallConnected> ack_req  = 
			shared_dynamic_cast<MsgNewCallConnected>(ptr);

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

		if (!ack_req->localOffer.body.empty())
		{
			FreeContent contents(ack_req->localOffer);
			ctx_ptr->uac_invite_handle->provideAnswer(contents);
		} 
	

	}

	

	#pragma TODO ("UACDialogUsageManager::UponMakeCallReq takes 20 ms - SDP preparation should be replaced with simple string concatenation")

	void 
	UACDialogUsageManager::onAnswer(
		IN InviteSessionHandle is, 
		IN const SipMessage& msg, 
		IN const Contents& body)
	{
		FUNCTRACKER;

		UACAppDialogSet* uac_set = (UACAppDialogSet*)(is->getAppDialogSet().get());
		SipDialogContextPtr ctx_ptr = uac_set->_ptr;

		LogDebug("UACDialogUsageManager::onAnswer rsh:" << is.getId() << ", iwh:" << ctx_ptr->stack_handle);

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
			user_profile = _dum.getMasterProfile();
			
			
			SharedPtr<SipMessage> invite_session;

			// create context
			//
			SipDialogContextPtr ctx_ptr = SipDialogContextPtr(new SipDialogContext());;

			UACAppDialogSet * uac_dialog_set = 
				new UACAppDialogSet(_dum,ctx_ptr,ptr);

			// generic offer INVITE
			if (!req->localOffer.body.empty())
			{
			
				Data free_data(req->localOffer.body);
				
				FreeContent fc(req->localOffer.body,req->localOffer.type);

				invite_session = _dum.makeInviteSession(
					name_addr, 
					user_profile,
					&fc, 
					uac_dialog_set); 
			}
			else
			{
				UACAppDialogSet * uac_dialog_set = new UACAppDialogSet(_dum,ctx_ptr,ptr);

				// empty invite
				invite_session = _dum.makeInviteSession(
					name_addr, 
					user_profile,
					NULL,
					uac_dialog_set); 
			};

			
			for (MapOfAny::iterator i = req->optional_params.begin(); 
				i != req->optional_params.end(); i++)
			{
				size_t pos = i->first.find("H_");
				if ( pos != 0 || i->second.type() != typeid(string))
					continue;
				
				string header_name  =  i->first.substr(2);
				string header_value =  any_cast<string>(i->second);


				
				Headers::Type header_type  = Headers::getType(header_name.c_str(), header_name.length());

				if (header_type != Headers::UNKNOWN)
				{
					HeaderFieldValueList hl;
					hl.push_back(new HeaderFieldValue(header_value.c_str(), header_value.length()));

					invite_session->setRawHeader(
						&hl,
						header_type);
				} else
				{
					ExtensionHeader ext(header_name.c_str());
					invite_session->header(ext).push_back(StringCategory(header_value.c_str()));

				}
				
				
			}
			
			
			_dum.send(invite_session);

			
			ctx_ptr->stack_handle = req->stack_call_handle == IW_UNDEFINED ? 
				GenerateCallHandle() : req->stack_call_handle;

			ctx_ptr->call_handler_inbound = req->call_handler_inbound;

			LogDebug("UACDialogUsageManager::UponMakeCallReq - INVITE iwh:" << ctx_ptr->stack_handle)

	
		} 
		catch (BaseException &e)
		{
			LogWarn("Exception while issuing invite:" << e.getMessage().c_str());
			GetCurrRunningContext()->SendResponse(ptr, new MsgMakeCallNack());
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
			GetCurrRunningContext()->SendResponse(
				req,
				new MsgCallBlindXferNack());
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;

		InviteSessionHandle invite_handle_to_replace = ctx_ptr->invite_handle;
		

		if (!invite_handle_to_replace.isValid() || invite_handle_to_replace->isConnected() == false)
		{
			LogWarn("UACDialogUsageManager::UponBlindXferReq - Cannot xfer in not connected state, " << LogHandleState(ctx_ptr, invite_handle_to_replace))
				GetCurrRunningContext()->SendResponse(
				req,
				new MsgCallBlindXferNack());

			return;
		}


		LogDebug("UACDialogUsageManager::UponBlindXferReq - dst:" << xfer_req->destination_uri << ", " << LogHandleState(ctx_ptr, invite_handle_to_replace));

		NameAddr name_addr(xfer_req->destination_uri.c_str());
		invite_handle_to_replace->refer(name_addr,false);

		GetCurrRunningContext()->SendResponse(
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

		LogWarn("UACDialogUsageManager::onFailure rsh:" << is.getId() <<", reason:" << msg.getReason().c_str() << ", iwh:" << ctx_ptr->stack_handle);

		switch ((uac_set)->last_makecall_req->message_id)
		{
		case MSG_MAKE_CALL_REQ:
			{
				GetCurrRunningContext()->SendResponse(
					(uac_set)->last_makecall_req,
					new MsgMakeCallNack());
				CleanUpCall(ctx_ptr);
				break;
			}
		default:
			{
				LogWarn("UACDialogUsageManager::onFailure Unknown req:"  << (uac_set)->last_makecall_req->message_id);
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

		LogDebug("UACDialogUsageManager::onConnected rsh:" << is.getId() << ", iwh:" << ctx_ptr->stack_handle);

		UACAppDialogSet* uac_set = (UACAppDialogSet*)(is->getAppDialogSet().get());

		MsgMakeCallOk *ack = 
			new MsgMakeCallOk();

		ack->stack_call_handle = ctx_ptr->stack_handle;

		const Uri &from_uri = msg.header(h_From).uri();
		ack->ani = from_uri.user().c_str();

		

		if (msg.getContents())
		{
			ack->remoteOffer.body = msg.getContents()->getBodyData().c_str();
			ack->remoteOffer.type = string("") + msg.getContents()->getType().type().c_str() + "/" + msg.getContents()->getType().subType().c_str();
		}

		GetCurrRunningContext()->SendResponse(
			uac_set->last_makecall_req,
			ack);

		return;

					
	}

	UACDialogUsageManager::~UACDialogUsageManager(void)
	{
		FUNCTRACKER;

	}



#pragma warning( pop )

}