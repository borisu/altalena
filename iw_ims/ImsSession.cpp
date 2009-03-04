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
#include "ProcIms.h"
#include "ImsSession.h"
#include "Ims.h"
#include "Call.h"

using namespace boost;

namespace ivrworx
{

ImsSession::ImsSession(IN ScopedForking &forking):
_imsSessionHandle(IW_UNDEFINED),
_imsSessionHandlerPair(HANDLE_PAIR),
_forking(forking),
_rfc2833DtmfHandle(new LpHandle())
{
	
}

ImsSession::~ImsSession(void)
{
	if (_imsSessionHandle != IW_UNDEFINED)
	{
		TearDown();
	}
}

ApiErrorCode
ImsSession::PlayFile(IN const string &file_name,
					 IN BOOL sync,
					 IN BOOL loop,
					 IN BOOL provisional)
{

	FUNCTRACKER;

	if (_imsSessionHandle == IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	// cannot sync and loop
	if (sync == TRUE && loop == TRUE)
	{
		LogWarn("Cannot play sync and in loop.");
		return API_FAILURE;
	}

	IwMessagePtr response = NULL_MSG;
	
	DECLARE_NAMED_HANDLE(ims_play_txn);
	ims_play_txn->HandleName("Ims Play TXN"); // for logging purposes
	ims_play_txn->Direction(MSG_DIRECTION_INBOUND);

	RegistrationGuard guard(ims_play_txn);

	
	MsgStartPlayReq *msg = new MsgStartPlayReq();
	msg->playback_handle	= _imsSessionHandle;
	msg->file_name			= file_name;
	msg->send_provisional	= provisional;
	msg->loop				= loop;
	msg->source.handle_id	= ims_play_txn->GetObjectUid();
	msg->transaction_id		= GenerateNewTxnId();
	
	ApiErrorCode res = GetCurrLightWeightProc()->SendMessage(IMS_Q,IwMessagePtr(msg));
	if (IW_FAILURE(res))
	{
		return res;
	}

	if (provisional)
	{
		res = GetCurrLightWeightProc()->WaitForTxnResponse(
			ims_play_txn,
			response, 
			MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()));
		if (IW_FAILURE(res))
		{
			return res;
		}

		if (response->message_id != MSG_START_PLAY_REQ_ACK)
		{
			return API_FAILURE;
		}

	}
	
	if (!sync)
	{
		return API_SUCCESS;
	}

	res = GetCurrLightWeightProc()->WaitForTxnResponse(ims_play_txn,response,  Seconds(3600));
	if (IW_FAILURE(res))
	{
		return res;
	}

	if (response->message_id != MSG_IMS_PLAY_STOPPED)
	{
		return API_FAILURE;
	}

	return API_SUCCESS;
}

const string& 
ImsSession::GetDtmfString()
{
	return _dtmf;
}


void 
ImsSession::ClearDtmfs()
{
	_dtmf.clear();
}

ApiErrorCode
ImsSession::WaitForDtmf(OUT int &dtmf, IN Time timeout)
{
	
	IwMessagePtr ptr = NULL_MSG;

	ApiErrorCode res = GetCurrLightWeightProc()->WaitForTxnResponse(_rfc2833DtmfHandle,ptr,timeout);
	if (IW_FAILURE(res))
	{
		return res;
	}

	shared_ptr<MsgImsRfc2833DtmfEvt> dtmf_event = shared_dynamic_cast<MsgImsRfc2833DtmfEvt> (ptr);
	dtmf = dtmf_event->dtmf_digit;

	_dtmf = _dtmf + (char)dtmf;

	return API_SUCCESS;

}

void
ImsSession::UponActiveObjectEvent(IwMessagePtr ptr)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case MSG_IMS_RFC2833DTMF_EVT:
		{
			_rfc2833DtmfHandle->Send(ptr);
		}
	default:
		{

		}
	};

	ActiveObject::UponActiveObjectEvent(ptr);
}

ApiErrorCode
ImsSession::AllocateIMSConnection(IN CnxInfo remote_end, 
								  IN MediaFormat codec)
{
	FUNCTRACKER;

	LogDebug("Allocating IMS session remote end = " <<  remote_end.ipporttos()  << ", codec = "  << codec);
	
	if (_imsSessionHandle != IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

	MsgAllocateImsSessionReq *msg = new MsgAllocateImsSessionReq();
	msg->remote_media_data = remote_end;
	msg->codec = codec;
	msg->session_handler = session_handler_pair;

	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		IMS_Q,
		IwMessagePtr(msg),
		response,
		MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
		"Allocate IMS Connection TXN");

	if (res != API_SUCCESS)
	{
		LogWarn("Error allocating Ims connection " << res);
		return res;
	}

	switch (response->message_id)
	{
	case MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK:
		{


			shared_ptr<MsgAllocateImsSessionAck> ack = 
				shared_polymorphic_cast<MsgAllocateImsSessionAck>(response);
			_imsSessionHandle	= ack->playback_handle;
			_imsMediaData		= ack->ims_media_data;

			Start(_forking,session_handler_pair,"Ims Session handler");

			LogDebug("Ims session allocated successfully, ims handle=[" << _imsSessionHandle << "]");

			break;

		}
	case MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK:
		{
			LogDebug("Error allocating Ims session.");
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



CnxInfo ImsSession::ImsMediaData() const 
{ 
	return _imsMediaData; 
}

void ImsSession::ImsMediaData(IN CnxInfo val) 
{ 
	_imsMediaData = val; 
}

void
ImsSession::TearDown()
{
	if(_imsSessionHandle == IW_UNDEFINED)
	{
		return;
	}

	MsgImsTearDownReq *tear_req = new MsgImsTearDownReq();
	tear_req->handle = _imsSessionHandle;

	// no way back
	_imsSessionHandle = IW_UNDEFINED;

	ApiErrorCode res = GetCurrLightWeightProc()->SendMessage(IMS_Q,IwMessagePtr(tear_req));

}

}