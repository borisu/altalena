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
#include "MrcpSession.h"



using namespace boost;
using namespace boost::assign;

namespace ivrworx
{



MrcpSession::MrcpSession(IN ScopedForking &forking, IN HandleId handle_id):
_mrcpSessionHandle(IW_UNDEFINED),
_mrcpSessionHandlerPair(HANDLE_PAIR),
_forking(forking),
_hangupHandle(new LpHandle()),
_playStoppedHandle(new LpHandle()),
_recognitionStoppedHandle(new LpHandle()),
_mrcpServiceHandleId(handle_id)
{
	FUNCTRACKER;

	
}

MrcpSession::~MrcpSession(void)
{
	FUNCTRACKER;

	StopActiveObjectLwProc();

	if (_mrcpSessionHandle != IW_UNDEFINED)
	{
		TearDown();
	}

	_hangupHandle->Poison();

	
}

MrcpHandle
MrcpSession::SessionHandle()
{
	return _mrcpSessionHandle;
}

ApiErrorCode
MrcpSession::DefineGrammar(IN const MrcpParams &p,
						   IN const string &body,
						   IN Time timeout)
{
	FUNCTRACKER;

	LogDebug("MrcpSession::DefineGrammar mrcph:" << _mrcpSessionHandle);

	if (_mrcpSessionHandle == IW_UNDEFINED)
	{
		LogWarn("MrcpSession::Speak session is not allocated.");
		return API_FAILURE;
	}

	MsgMrcpDefineGrammarReq *msg = 
		new MsgMrcpDefineGrammarReq();

	msg->mrcp_handle	 = _mrcpSessionHandle;
	msg->body			 = body;
	msg->params			 = p;


	IwMessagePtr response  = NULL_MSG;
	ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_mrcpServiceHandleId,
		IwMessagePtr(msg),
		response,
		timeout,
		"Define Grammar TXN");

	if (IW_FAILURE(res) || response->message_id != MSG_MRCP_DEFINE_GRAMMAR_ACK)
	{
		LogWarn("Error sending define grammar request to mrcp, mrcph:" << _mrcpSessionHandle);
		return API_FAILURE;
	}

	return API_SUCCESS;

}

ApiErrorCode
MrcpSession::StopSpeak()
{
	FUNCTRACKER;

	LogDebug("MrcpSession::StopSpeak mrcph:" << _mrcpSessionHandle );

	if (_mrcpSessionHandle == IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	MsgMrcpStopSpeakReq *msg = new MsgMrcpStopSpeakReq();
	msg->mrcp_handle	= _mrcpSessionHandle;
	
	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_mrcpServiceHandleId,
		IwMessagePtr(msg),
		response,
		Seconds(5),
		"Stop Speak TXN");

	
	return res;

}



ApiErrorCode
MrcpSession::Speak(IN const MrcpParams &p,
				   IN const string &body,
				   IN BOOL sync)
{

	FUNCTRACKER;

	LogDebug("MrcpSession::Speak mrcph:" << _mrcpSessionHandle << ", sync:" << sync << ", body:" << body );

	if (_mrcpSessionHandle == IW_UNDEFINED)
	{
		LogWarn("MrcpSession::Speak session is not allocated.");
		return API_FAILURE;
	}


	MsgMrcpSpeakReq *msg = new MsgMrcpSpeakReq();
	msg->mrcp_handle	= _mrcpSessionHandle;
	msg->body			= body;
	msg->params			= p;

	
	IwMessagePtr response  = NULL_MSG;
	ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_mrcpServiceHandleId,
		IwMessagePtr(msg),
		response,
		Seconds(5),
		"Speak txn");

	if (IW_FAILURE(res) || response->message_id != MSG_MRCP_SPEAK_ACK)
	{
		LogDebug("Error sending play request to mrcp, mrcph:" << _mrcpSessionHandle);
		return res;
	}

	if (!sync)
	{
		return API_SUCCESS;
	}

	shared_ptr<MsgMrcpSpeakAck> ack = 
		dynamic_pointer_cast<MsgMrcpSpeakAck>(response);
	

	while  (true)
	{
		int handle_index = IW_UNDEFINED;
		res = GetCurrRunningContext()->WaitForTxnResponse(
			list_of(_playStoppedHandle)(_hangupHandle),
			handle_index,
			response, 
			Seconds(3600));

		if (handle_index == 1)
		{
			return API_HANGUP;
		}

		if (IW_FAILURE(res))
		{
			return res;
		}

		shared_ptr<MsgMrcpSpeakStoppedEvt> stopped_evt = 
			dynamic_pointer_cast<MsgMrcpSpeakStoppedEvt>(response);

		if (stopped_evt->correlation_id == ack->correlation_id)
		{
			return API_SUCCESS;
		}

	} 
}


void
MrcpSession::UponActiveObjectEvent(IwMessagePtr ptr)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case MSG_MRCP_SPEAK_STOPPED_EVT:
		{
			_playStoppedHandle->Send(ptr);
			break;
		}
	case MSG_MRCP_RECOGNITION_COMPLETE_EVT:
		{
			_recognitionStoppedHandle->Send(ptr);
			break;
		}
	}

	ActiveObject::UponActiveObjectEvent(ptr);
}

ApiErrorCode 
MrcpSession::WaitForRecogResult(IN Time timeout, OUT string &answer)
{
	ApiErrorCode res = API_SUCCESS;
	IwMessagePtr response;

	
	int handle_index = IW_UNDEFINED;
	res = GetCurrRunningContext()->WaitForTxnResponse(
		list_of(_recognitionStoppedHandle)(_hangupHandle),
		handle_index,
		response, 
		timeout);

	if (handle_index == 1)
	{
		return API_HANGUP;
	}

	if (IW_FAILURE(res))
	{
		return res;
	}

	switch (response->message_id)
	{
	case MSG_MRCP_RECOGNITION_COMPLETE_EVT:
		{
			shared_ptr<MsgMrcpRecognitionCompleteEvt> recog_evt = 
				dynamic_pointer_cast<MsgMrcpRecognitionCompleteEvt>(response);
			answer = recog_evt->body;
			break;
		}
	default:
		{
			LogWarn("MrcpSession::WaitForRecogResult - unknown response res:" << response->message_id);
			res = API_UNKNOWN_RESPONSE;
			break;
		}
	}

	

	return res;

}



ApiErrorCode 
MrcpSession::Recognize(IN const MrcpParams &p,
					   IN const string &body,
					   IN Time timeout, 
					   IN BOOL sync, 
					   OUT string& answer)
{
	FUNCTRACKER;

	if (_mrcpSessionHandle == IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

	MsgMrcpRecognizeReq *msg = new MsgMrcpRecognizeReq();
	msg->params = p;
	msg->mrcp_handle = _mrcpSessionHandle;
	msg->body = body;
	
	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_mrcpServiceHandleId,
		IwMessagePtr(msg),
		response,
		Seconds(15),
		"Recognize MRCP TXN");

	if (IW_FAILURE(res))
	{
		LogWarn("Error sending recognize request to mrcp, mrcph:" << _mrcpSessionHandle);
		return res;
	}

	if (!sync)
	{
		return API_SUCCESS;
	};

	switch (response->message_id)
	{
	case MSG_MRCP_RECOGNIZE_ACK:
		{

			shared_ptr<MsgMrcpRecognizeAck> ack = 
				shared_polymorphic_cast<MsgMrcpRecognizeAck>(response);

			
			LogDebug("MrcpSession::Recognize - recognized succesfully , mrcph:" << _mrcpSessionHandle );

			break;

		}
	case MSG_MRCP_RECOGNIZE_NACK:
		{
			shared_ptr<MsgMrcpRecognizeNack> nack = 
				shared_polymorphic_cast<MsgMrcpRecognizeNack>(response);

			LogWarn("MrcpSession::Recognize - Error allocating Mrcp session , mrcph:" << _mrcpSessionHandle << " error:" << nack->response_error_code);
			res = API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}

	if (IW_FAILURE(res))
		return res;

	return WaitForRecogResult(timeout, answer);

}

ApiErrorCode
MrcpSession::Allocate(IN MrcpResource rsc)
{
	FUNCTRACKER;

	AbstractOffer dummy_offer;
	return Allocate(rsc, dummy_offer, Seconds(15)); 
}

ApiErrorCode
MrcpSession::Allocate(IN MrcpResource rsc,
					  IN const AbstractOffer &localOffer, 
					  IN Time timeout)
{
	FUNCTRACKER;

	LogDebug("MrcpSession::Allocate lci:" <<  localOffer.body  << ", mrcph:" << _mrcpSessionHandle);
	
	DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

	MsgMrcpAllocateSessionReq *msg = new MsgMrcpAllocateSessionReq();
	msg->offer		= localOffer;
	msg->session_handler	= session_handler_pair;
	msg->resource			= rsc;
	msg->mrcp_handle		= _mrcpSessionHandle;

	_localOffers[rsc] = msg->offer;

	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_mrcpServiceHandleId,
		IwMessagePtr(msg),
		response,
		timeout,
		"Allocate MRCP Connection TXN");

	if (res != API_SUCCESS)
	{
		LogWarn("Error allocating Mrcp connection " << res);
		return res;
	}

	switch (response->message_id)
	{
	case MSG_MRCP_ALLOCATE_SESSION_ACK:
		{

			shared_ptr<MsgMrcpAllocateSessionAck> ack = 
				shared_polymorphic_cast<MsgMrcpAllocateSessionAck>(response);

			if (_mrcpSessionHandle == IW_UNDEFINED)
			{
				StartActiveObjectLwProc(_forking,session_handler_pair,"Mrcp Session handler");
				_mrcpSessionHandle	  = ack->mrcp_handle;
			}

			_remoteOffers[rsc] = ack->offer;

			LogDebug("MrcpSession::Allocate - Mrcp session allocated successfully, rsc:" <<  rsc << ", mrcph:" << _mrcpSessionHandle << ", sdp:" << ack->offer.body);

			break;

		}
	case MSG_MRCP_ALLOCATE_SESSION_NACK:
		{
			LogDebug("Error allocating Mrcp session.");
			res = API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}
	return res;

}

void
MrcpSession::InterruptWithHangup()
{
	FUNCTRACKER;

	_hangupHandle->Send(new MsgMrcpStopSpeakReq());
}

ApiErrorCode
MrcpSession::ModifySession(IN const AbstractOffer &remote_offer, 
						   IN Time timeout)
{
	FUNCTRACKER;

	LogDebug("MrcpSession::ModifyConnection remote:" <<  remote_offer.body  << ", mrcph:" << _mrcpSessionHandle);

	if (_mrcpSessionHandle == IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

	MsgMrcpModifyReq *msg = new MsgMrcpModifyReq();
	msg->offer = remote_offer;
	msg->mrcp_handle = _mrcpSessionHandle;

	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_mrcpServiceHandleId,
		IwMessagePtr(msg),
		response,
		MilliSeconds(GetCurrRunningContext()->TransactionTimeout()),
		"Modify MRCP Connection TXN");

	if (res != API_SUCCESS)
	{
		LogWarn("Error modifying Mrcp connection " << res);
		return res;
	}

	switch (response->message_id)
	{
	case MSG_MRCP_MODIFY_ACK:
		{
			return API_SUCCESS;
		}
	case MSG_MRCP_MODIFY_NACK:
		{
			return API_FAILURE;
		}
	default:
		{
			throw;
		}
	}

}


void
MrcpSession::TearDown()
{
	if(_mrcpSessionHandle == IW_UNDEFINED)
	{
		return;
	}

	MsgMrcpTearDownReq *tear_req = new MsgMrcpTearDownReq();
	tear_req->mrcp_handle = _mrcpSessionHandle;

	// no way back
	_mrcpSessionHandle = IW_UNDEFINED;

	ApiErrorCode res = GetCurrRunningContext()->SendMessage(_mrcpServiceHandleId,IwMessagePtr(tear_req));

}

AbstractOffer 
MrcpSession::RemoteOffer(MrcpResource rsc)  
{ 
	OffersMap::iterator iter  = 
		_remoteOffers.find(rsc);

	if (iter == _remoteOffers.end())
	{
		return AbstractOffer();
	}

	return iter->second; 
}



AbstractOffer 
MrcpSession::LocalOffer(MrcpResource rsc)  
{ 
	OffersMap::iterator iter  = 
		_localOffers.find(rsc);

	if (iter == _localOffers.end())
	{
		return AbstractOffer();
	}

	return iter->second; 
}



}