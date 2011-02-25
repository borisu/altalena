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
#include "FreeContent.h"


namespace ivrworx
{
	UASDialogUsageManager::UASDialogUsageManager(
		IN ConfigurationPtr conf,
		IN IwHandlesMap &handles_map,
		IN ResipDialogHandlesMap &resipHandlesMap,
		IN DialogUsageManager &dum):
		_conf(conf),
		_resipHandlesMap(resipHandlesMap),
		_refIwHandlesMap(handles_map),
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
		if (ack->localOffer.body.empty())
		{
			LogWarn("UASDialogUsageManager::UponCallOfferedAck -  No accepted codec found " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			HangupCall(ctx_ptr);
		}

		LogDebug("UASDialogUsageManager::UponCallOfferedAck -  " << LogHandleState(ctx_ptr, ctx_ptr->invite_handle));

		Data free_data(ack->localOffer.body);
		FreeContent fc(ack->localOffer.body,ack->localOffer.type);

		((UASAppDialogSet*)(ctx_ptr->uas_invite_handle->getAppDialogSet().get()))->_makeCallAck
			= req;
		

		ctx_ptr->uas_invite_handle.get()->provideAnswer(fc);
		ctx_ptr->uas_invite_handle.get()->accept();
	}


	void 
	UASDialogUsageManager::onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
	{
		FUNCTRACKER;

		sis->provisional(100);
		sis->provisional(180);

		// prepare dialog context
		SipDialogContextPtr ctx_ptr(new SipDialogContext(FALSE));
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

	void
	UASDialogUsageManager::UponSubscribeToIncomingReq(IwMessagePtr req)
	{
		FUNCTRACKER;
		shared_ptr<MsgCallSubscribeReq> subscribe_req = 
			dynamic_pointer_cast<MsgCallSubscribeReq>(req);

		if (_eventsHandle)
		{
			LogWarn("Only one listener suported curently");
			GetCurrRunningContext()->SendResponse(subscribe_req, 
				new MsgCallSubscribeNack());
		}
		else
		{
			_eventsHandle = subscribe_req->listener_handle;
			GetCurrRunningContext()->SendResponse(subscribe_req, 
				new MsgCallSubscribeAck());
		}
	

	}

#pragma TODO("UASDialogUsageManager::onOffer takes 7 ms - SDP parsing is taking long (almost 4 ms)")

	void 
	UASDialogUsageManager::onOffer(InviteSessionHandle is, const SipMessage& msg, const Contents& body)      
	{
		FUNCTRACKER;

		ResipDialogHandlesMap::iterator ctx_iter  = _resipHandlesMap.find(is->getAppDialog());
		if (ctx_iter == _resipHandlesMap.end())
		{
			LogCrit("UASDialogUsageManager::onOffer - without created context, rsh:" << is.getId());
			return;
		}

		if (!_eventsHandle)
		{
			LogWarn("Stack has no listener set - rejecting the call");
			is->getAppDialogSet()->end();
			return;
		}

		SipDialogContextPtr ctx_ptr = (*ctx_iter).second;
		ctx_ptr->invite_handle = is;

		LogDebug("UASDialogUsageManager::onOffer - " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));

		// keep alive session with the same sdp version number
		if (is->isAccepted())
		{
			LogDebug("UASDialogUsageManager::onOffer - Session timer keep-alive " << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
 			is->provideAnswer(is->getLocalSdp());
			return;
		}


		DECLARE_NAMED_HANDLE_PAIR(call_handler_pair);
		
		// keep alive or hold and reinvite are not accepted currently
		if (is->isAccepted())
		{
			LogWarn("UASDialogUsageManager::onOffer - In-dialog body is not supported, rejecting body" << LogHandleState(ctx_ptr,ctx_ptr->invite_handle));
			is->reject(488);
			return;
		}

		MsgCallOfferedReq *offered = new MsgCallOfferedReq();
		offered->remoteOffer.body = msg.getContents()->getBodyData().c_str();
		offered->remoteOffer.type = string("") + msg.getContents()->getType().type().c_str() + "/" + msg.getContents()->getType().subType().c_str();
		offered->stack_call_handle	= ctx_ptr->stack_handle;
		offered->call_handler_inbound = call_handler_pair;


		const Uri &to_uri = msg.header(h_To).uri();
		offered->dnis = to_uri.user().c_str();

		const Uri &from_uri = msg.header(h_From).uri();
		offered->ani = from_uri.user().c_str();

		
		if (_eventsHandle) 
		{
			
			ApiErrorCode res = _eventsHandle->Send(offered);
			if (IW_FAILURE(res))
				_eventsHandle.reset();
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


		GetCurrRunningContext()->SendResponse(
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