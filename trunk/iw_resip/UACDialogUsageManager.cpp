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
#include "ResipCommon.h"
#include "UacDialogUsageManager.h"
#include "IwAppDialogSet.h"
#include "FreeContent.h"


namespace ivrworx
{

#pragma warning( push )
#pragma warning( disable : 4267 )

	UACDialogUsageManager::UACDialogUsageManager(
		IN ConfigurationPtr conf,
		IN IwHandlesMap &ccu_handles_map,
		IN DialogUsageManager &dum):
		_conf(conf),
		_dum(dum),
		_iwHandlesMap(ccu_handles_map)
	{
		FUNCTRACKER;

	}


	void 
	UACDialogUsageManager::onFailure(
			IN ClientOutOfDialogReqHandle h, 
			IN const SipMessage& errorResponse)
	{
		FUNCTRACKER;

// 		IwAppDialogSet* uac_set = 
// 			(IwAppDialogSet*)(h->getAppDialogSet().get());
// 
// 		if (uac_set && uac_set->last_registration_req)
// 		{
// 			GetCurrRunningContext()->SendResponse(uac_set->last_registration_req,
// 				new MsgSipCallRegisterNack());
// 
// 			_iwHandlesMap.erase(uac_set->_ptr->stack_handle);
// 
// 		}
	}

	void 
	UACDialogUsageManager::onSuccess(
		IN ClientRegistrationHandle h, 
		IN const SipMessage& response)
	{
		FUNCTRACKER;
		IwAppDialogSet* uac_set = 
			(IwAppDialogSet*)(h->getAppDialogSet().get());

		uac_set->dialog_ctx->uac_register_handle = h;

		if (!uac_set->last_registration_req)
			return;
		
		MsgSipCallRegisterAck* ack = new MsgSipCallRegisterAck();
		ack->registration_id = uac_set->dialog_ctx->stack_handle;

		GetCurrRunningContext()->SendResponse(uac_set->last_registration_req,
			ack);

		uac_set->last_registration_req.reset();

	}

	void 
	UACDialogUsageManager::onFailure(
		IN ClientRegistrationHandle h, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;

		IwAppDialogSet* uac_set = 
			(IwAppDialogSet*)(h->getAppDialogSet().get());

		GetCurrRunningContext()->SendResponse(uac_set->last_registration_req,
			new MsgSipCallRegisterNack());

		_iwHandlesMap.erase(uac_set->dialog_ctx->stack_handle);

		

	}

	void 
	UACDialogUsageManager::onRemoved(
		IN ClientRegistrationHandle h, 
		IN const SipMessage& response)
	{
		FUNCTRACKER;

		IwAppDialogSet* uac_set = 
			(IwAppDialogSet*)(h->getAppDialogSet().get());

		_iwHandlesMap.erase(uac_set->dialog_ctx->stack_handle);

	}

	void 
	UACDialogUsageManager::UponUnRegisterReq(IN IwMessagePtr ptr)
	{
		FUNCTRACKER;

		shared_ptr<MsgSipCallUnRegisterReq> unreg_req = 
			dynamic_pointer_cast<MsgSipCallUnRegisterReq>(ptr);

		IwHandlesMap::iterator iter = _iwHandlesMap.find(unreg_req->registration_id);
		if (iter == _iwHandlesMap.end())
		{
			LogDebug("UACDialogUsageManager::UponRegisterReq - the call iwh:" << unreg_req->registration_id << " already unregistered");
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;

		if (ctx_ptr->uac_register_handle.isValid())
		{
			ctx_ptr->uac_register_handle->removeMyBindings(true);
		}
		
		_iwHandlesMap.erase(ctx_ptr->stack_handle);

		

	
		
	}

	void 
	UACDialogUsageManager::UponRegisterReq(IN IwMessagePtr ptr)
	{
		FUNCTRACKER;

		shared_ptr<MsgSipCallRegisterReq> reg_req = 
			dynamic_pointer_cast<MsgSipCallRegisterReq>(ptr);

		// create context
		//
		SipDialogContextPtr ctx_ptr = SipDialogContextPtr(new SipDialogContext(TRUE));
		ctx_ptr->stack_handle = GenerateCallHandle();
		

		IwAppDialogSet *uac_dialog_set = 
			new IwAppDialogSet(_dum,ctx_ptr,ptr);
		uac_dialog_set->last_registration_req = reg_req;


		NameAddr addr(reg_req ->registrar.c_str());
		SharedPtr<UserProfile> &bp = _dum.getMasterUserProfile();
		SharedPtr<UserProfile> up (new UserProfile(bp));
		

		ctx_ptr->user_profile = up;

		
		if (reg_req->max_registration_time != IW_UNDEFINED)
			up->setDefaultMaxRegistrationTime(reg_req->max_registration_time);
		else
			up->setDefaultMaxRegistrationTime(1);

		if (reg_req->max_registration_time != IW_UNDEFINED)
			up->setDefaultRegistrationRetryTime(reg_req->registration_retry_time);
		else
			up->setDefaultMaxRegistrationTime(180);

		LogDebug("UACDialogUsageManager::UponRegisterReq - " << reg_req->credentials.username.c_str() << "(@)" << reg_req->credentials.realm.c_str());
		up->setDigestCredential(
			reg_req->credentials.realm.c_str(),
			reg_req->credentials.username.c_str(), 
			reg_req->credentials.password.c_str());

		up->setDefaultFrom(bp->getDefaultFrom());

		

		SharedPtr<SipMessage> msg =  _dum.makeRegistration(addr,up,uac_dialog_set);
		_iwHandlesMap[ctx_ptr->stack_handle]= ctx_ptr;
		

		

		_dum.send(msg);

	}

	//subscription can be ended through a notify or a failure response.
	void 
	UACDialogUsageManager::onTerminated(
		IN ClientSubscriptionHandle h, 
		IN const SipMessage* msg)
	{
		FUNCTRACKER;

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(h->getAppDialogSet().get());

		SipDialogContextPtr ctx_ptr = uac_set->dialog_ctx;
		ctx_ptr->uac_subscription_handle = h;

		LogWarn("UACDialogUsageManager::onTerminated rsh:" << h.getId() << ", iwh:" << ctx_ptr->stack_handle);

		GetCurrRunningContext()->SendResponse(
			ctx_ptr->last_subscribe_req,
			new MsgSipCallSubscribeNack());

		CleanUpCall(ctx_ptr);

		return;

	}

	//not sure if this has any value.
	void 
	UACDialogUsageManager::onNewSubscription(
		IN ClientSubscriptionHandle h, 
		IN const SipMessage& notify)
	{
		FUNCTRACKER;

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(h->getAppDialogSet().get());

		SipDialogContextPtr ctx_ptr = uac_set->dialog_ctx;
		ctx_ptr->uac_subscription_handle = h;

		LogDebug("UACDialogUsageManager::onNewSubscription rsh:" << h.getId() << ", iwh:" << ctx_ptr->stack_handle);

		GetCurrRunningContext()->SendResponse(
			ctx_ptr->last_subscribe_req,
			new MsgSipCallSubscribeAck());

		return;

	}

	void 
	UACDialogUsageManager::UponSubscribeReq(IN IwMessagePtr ptr)
	{
		FUNCTRACKER;

		shared_ptr<MsgSipCallSubscribeReq> subscribe_req = 
			dynamic_pointer_cast<MsgSipCallSubscribeReq>(ptr);

		SipDialogContextPtr ctx_ptr;
		IwAppDialogSet *uac_dialog_set = NULL;

		// on non-existing INVITE dialog?
		if (subscribe_req->stack_call_handle == IW_UNDEFINED)
		{
			// create context
			//
			ctx_ptr = SipDialogContextPtr(new SipDialogContext(TRUE));
			ctx_ptr->stack_handle		   = GenerateCallHandle();
			ctx_ptr->call_handler_inbound  =  subscribe_req->call_handler_inbound;


			uac_dialog_set = new IwAppDialogSet(_dum,ctx_ptr,ptr);
			uac_dialog_set->last_registration_req = subscribe_req;



			NameAddr addr(subscribe_req ->dest.c_str());
			SharedPtr<UserProfile> &bp = _dum.getMasterUserProfile();
			SharedPtr<UserProfile> up (new UserProfile(bp));

			ctx_ptr->user_profile = up;
			

			LogDebug("UACDialogUsageManager::UponSubscribeReq - " << subscribe_req->credentials.username.c_str() << "(@)" << subscribe_req->credentials.realm.c_str());
			up->setDigestCredential(
				subscribe_req->credentials.realm.c_str(),
				subscribe_req->credentials.username.c_str(), 
				subscribe_req->credentials.password.c_str());

			if (subscribe_req->ani.empty()) 
			{
				up->setDefaultFrom(bp->getDefaultFrom());
			} 
			else
			{
				up->setDefaultFrom(NameAddr(subscribe_req->ani.c_str()));
			}
				
			_iwHandlesMap[ctx_ptr->stack_handle]= ctx_ptr;

		} 
		else
		{
			IwHandlesMap::iterator iter = _iwHandlesMap.find(subscribe_req->stack_call_handle);
			if (iter == _iwHandlesMap.end())
			{
				LogDebug("UACDialogUsageManager::UponSubscribeReq - the call iwh:" << subscribe_req->stack_call_handle << " does not exits.");
				GetCurrRunningContext()->SendResponse(
					subscribe_req,
					new MsgSipCallSubscribeNack());

				return;
			}

			ctx_ptr = (*iter).second;

			if (ctx_ptr->last_subscribe_req)
			{
				LogDebug("UACDialogUsageManager::UponSubscribeReq - transaction in progress.");
				GetCurrRunningContext()->SendResponse(
					subscribe_req,
					new MsgSipCallSubscribeNack());

				return;
			}

			AppDialogSetHandle app_diag_set_handle = ctx_ptr->invite_handle->getAppDialogSet();
			uac_dialog_set = (IwAppDialogSet*)app_diag_set_handle.get();

		}

		

		ctx_ptr->last_subscribe_req = 
			subscribe_req;

		SharedPtr<SipMessage> msg =  _dum.makeSubscription(
			NameAddr(subscribe_req->dest.c_str()),
			ctx_ptr->user_profile,
			subscribe_req->events_package.c_str(),
			subscribe_req->refresh_interval,
			subscribe_req->refresh_interval,
			uac_dialog_set);
		

		_dum.send(msg);

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

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(ctx_ptr->invite_handle->getAppDialogSet().get());
		uac_set->last_options_req = ptr;


	}



	/// called when response to INFO message is received 
	void 
	UACDialogUsageManager::onInfoSuccess(InviteSessionHandle is, const SipMessage& msg)
	{
		FUNCTRACKER;

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(is->getAppDialogSet().get());

		SipDialogContextPtr ctx_ptr = uac_set->dialog_ctx;

		LogDebug("UACDialogUsageManager::onInfoSuccess rsh:" << is.getId() << ", iwh:" << ctx_ptr->stack_handle);

		if (!(uac_set)->last_options_req)
		{
			return;
		}

		MsgSipCallInfoAck * ack = new MsgSipCallInfoAck();

		if (msg.getContents())
		{
			ack->remoteOffer.body = msg.getContents()->getBodyData().c_str();
			ack->remoteOffer.type = string("") + msg.getContents()->getType().type().c_str() + "/" + msg.getContents()->getType().subType().c_str();
		}


		GetCurrRunningContext()->SendResponse(
			(uac_set)->last_options_req,
			ack);

		(uac_set)->last_options_req.reset();

	}

	void UACDialogUsageManager::onInfoFailure(InviteSessionHandle is, const SipMessage& msg)
	{
		FUNCTRACKER;

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(is->getAppDialogSet().get());

		SipDialogContextPtr ctx_ptr = uac_set->dialog_ctx;

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

			ctx_ptr->uac_invite_handle->getAppDialogSet()->end();
		}

		if (ctx_ptr->uas_invite_handle.isValid())
		{
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
			ctx_ptr->uac_invite_handle->getAppDialogSet()->end();
		}
		if (ctx_ptr->uas_invite_handle.isValid())
		{
			ctx_ptr->uas_invite_handle->getAppDialogSet()->end();
		}
		if (ctx_ptr->uac_subscription_handle.isValid())
		{
			ctx_ptr->uac_subscription_handle->getAppDialogSet()->end();
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

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(is->getAppDialogSet().get());
		SipDialogContextPtr ctx_ptr = uac_set->dialog_ctx;

		LogDebug("UACDialogUsageManager::onAnswer rsh:" << is.getId() << ", iwh:" << ctx_ptr->stack_handle);

		if (ctx_ptr->last_reoffer_req)
		{
			MsgCallReofferAck *ok = new MsgCallReofferAck();

			if (msg.getContents())
			{
				ok->remoteOffer.body = msg.getContents()->getBodyData().c_str();
				ok->remoteOffer.type = string("") + msg.getContents()->getType().type().c_str() + "/" + msg.getContents()->getType().subType().c_str();
			}

			GetCurrRunningContext()->SendResponse(
				ctx_ptr->last_reoffer_req,
				ok);

			ctx_ptr->last_reoffer_req.reset();
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

			
			
			SharedPtr<SipMessage> invite_session;

			// create context
			//
			SipDialogContextPtr ctx_ptr = SipDialogContextPtr(new SipDialogContext(TRUE));;

			IwAppDialogSet * uac_dialog_set = 
				new IwAppDialogSet(_dum,ctx_ptr,ptr);

			
			MapOfAny::iterator param_iter = req->optional_params.find("registration_id");
				 
			if (param_iter != req->optional_params.end())
			{
				int registration_id = ::atoi(param_iter->second.c_str());
				LogDebug("UACDialogUsageManager::UponMakeCallReq - the call has registration id:" << registration_id << ". re-use its user profile");
				IwHandlesMap::iterator iter = _iwHandlesMap.find(registration_id);
				if (iter == _iwHandlesMap.end())
				{
					LogWarn("UACDialogUsageManager::UponMakeCallReq - cannot find specified registration id" << registration_id);
					GetCurrRunningContext()->SendResponse(ptr, new MsgMakeCallNack());
					return;
				}

				SipDialogContextPtr ctx_ptr = iter->second;
				user_profile = ctx_ptr->user_profile;

			} 
			else 
			{
				
				SharedPtr<UserProfile> &bp = _dum.getMasterUserProfile();
				SharedPtr<UserProfile> up (new UserProfile(bp));

				ctx_ptr->user_profile = up;

				if (req->credentials.isValid())
				{
					LogDebug("UACDialogUsageManager::UponSubscribeReq - " << req->credentials.username.c_str() << "(@)" << req->credentials.realm.c_str());
					up->setDigestCredential(
						req->credentials.realm.c_str(),
						req->credentials.username.c_str(), 
						req->credentials.password.c_str());
				};

				user_profile = up;
				user_profile->setDefaultFrom(_dum.getMasterProfile()->getDefaultFrom());
			}

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
				IwAppDialogSet * uac_dialog_set = new IwAppDialogSet(_dum,ctx_ptr,ptr);

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

				if (pos == string::npos)
					continue;
		
				string header_name  =  i->first.substr(2);
				string header_value = i->second;

				
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


		SipDialogContextPtr ctx_ptr = ((IwAppDialogSet*)(s->getAppDialogSet().get()))->dialog_ctx;
		ctx_ptr->uac_invite_handle = s;

		
		ctx_ptr->invite_handle =_dum.findInviteSession(s->getAppDialog()->getDialogId());
		if (!ctx_ptr->invite_handle.isValid())
		{
			LogWarn("UACDialogUsageManager::onNewSession - Invalid invite handle");
		}

		
		//
		// put them in maps
		//
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

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(is->getAppDialogSet().get());

		SipDialogContextPtr ctx_ptr = uac_set->dialog_ctx;

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
	
		SipDialogContextPtr ctx_ptr = ((IwAppDialogSet*)is->getAppDialogSet().get())->dialog_ctx;

		LogDebug("UACDialogUsageManager::onConnected rsh:" << is.getId() << ", iwh:" << ctx_ptr->stack_handle);

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(is->getAppDialogSet().get());

		if (ctx_ptr->last_reoffer_req)
		{
			MsgCallReofferAck *ok = new MsgCallReofferAck();

			if (msg.getContents())
			{
				ok->remoteOffer.body = msg.getContents()->getBodyData().c_str();
				ok->remoteOffer.type = string("") + msg.getContents()->getType().type().c_str() + "/" + msg.getContents()->getType().subType().c_str();
			}

			GetCurrRunningContext()->SendResponse(
				ctx_ptr->last_reoffer_req,
				ok);

			ctx_ptr->last_reoffer_req.reset();
		} 
		else
		{
			MsgMakeCallOk *ok = new MsgMakeCallOk();

			ok->stack_call_handle = ctx_ptr->stack_handle;

			const Uri &from_uri = msg.header(h_From).uri();
			ok->ani = from_uri.user().c_str();

			if (msg.getContents())
			{
				ok->remoteOffer.body = msg.getContents()->getBodyData().c_str();
				ok->remoteOffer.type = string("") + msg.getContents()->getType().type().c_str() + "/" + msg.getContents()->getType().subType().c_str();
			}

			GetCurrRunningContext()->SendResponse(
				uac_set->last_makecall_req,
				ok);
		}

		return;
					
	}

	void 
	UACDialogUsageManager::onUpdatePending(
		IN ClientSubscriptionHandle h, 
		IN const SipMessage& notify, 
		IN bool outOfOrder)
	{
		FUNCTRACKER;
		h->acceptUpdate();

		MsgSipCallNotifyEvt *msg = new MsgSipCallNotifyEvt();

		if (notify.getContents())
		{
			msg->remoteOffer.body = notify.getContents()->getBodyData().c_str();
			msg->remoteOffer.type = string("") + notify.getContents()->getType().type().c_str() + "/" + notify.getContents()->getType().subType().c_str();
		}

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(h->getAppDialogSet().get());
		uac_set->dialog_ctx->call_handler_inbound->Send(msg);
		

	}

	void 
	UACDialogUsageManager::onUpdateActive(
		IN ClientSubscriptionHandle h, 
		IN const SipMessage& notify, 
		IN bool outOfOrder)
	{
		FUNCTRACKER;
		h->acceptUpdate();

		MsgSipCallNotifyEvt *msg = new MsgSipCallNotifyEvt();

		if (notify.getContents())
		{
			msg->remoteOffer.body = notify.getContents()->getBodyData().c_str();
			msg->remoteOffer.type = string("") + notify.getContents()->getType().type().c_str() + "/" + notify.getContents()->getType().subType().c_str();
		}

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(h->getAppDialogSet().get());
		uac_set->dialog_ctx->call_handler_inbound->Send(msg);
		

	}

	//unknown Subscription-State value
	void 
	UACDialogUsageManager::onUpdateExtension(
		IN ClientSubscriptionHandle h, 
		IN const SipMessage& notify, 
		IN bool outOfOrder)
	{
		FUNCTRACKER;
		h->acceptUpdate();

		MsgSipCallNotifyEvt *msg = new MsgSipCallNotifyEvt();

		if (notify.getContents())
		{
			msg->remoteOffer.body = notify.getContents()->getBodyData().c_str();
			msg->remoteOffer.type = string("") + notify.getContents()->getType().type().c_str() + "/" + notify.getContents()->getType().subType().c_str();
		}

		IwAppDialogSet* uac_set = (IwAppDialogSet*)(h->getAppDialogSet().get());
		uac_set->dialog_ctx->call_handler_inbound->Send(msg);
			

	}

	UACDialogUsageManager::~UACDialogUsageManager(void)
	{
		FUNCTRACKER;

	}



#pragma warning( pop )

}