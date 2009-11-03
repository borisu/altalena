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
#include "ProcMrcp.h"
#include "MrcpSession.h"
#include "Mrcp.h"


using namespace boost;
using namespace boost::assign;

namespace ivrworx
{



MrcpSession::MrcpSession(IN ScopedForking &forking):
_mrcpSessionHandle(IW_UNDEFINED),
_mrcpSessionHandlerPair(HANDLE_PAIR),
_forking(forking),
_hangupHandle(new LpHandle()),
_playStoppedHandle(new LpHandle())
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
MrcpSession::StopSpeak()
{
	FUNCTRACKER;

	if (_mrcpSessionHandle == IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	MsgMrcpStopSpeakReq *msg = new MsgMrcpStopSpeakReq();
	msg->mrcp_handle	= _mrcpSessionHandle;
	
	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		MRCP_Q,
		IwMessagePtr(msg),
		response,
		Seconds(5),
		"Stop Speak TXN");
	return res;

}



ApiErrorCode
MrcpSession::Speak(IN const string &mrcp_xml,
					 IN BOOL sync)
{

	FUNCTRACKER;

	if (_mrcpSessionHandle == IW_UNDEFINED)
	{
		LogWarn("MrcpSession::Speak session is not allocated.");
		return API_FAILURE;
	}

	MsgMrcpSpeakReq *msg = new MsgMrcpSpeakReq();
	msg->mrcp_handle	= _mrcpSessionHandle;
	msg->mrcp_xml			= mrcp_xml;

	
	IwMessagePtr response  = NULL_MSG;
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		MRCP_Q,
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
		res = GetCurrLightWeightProc()->WaitForTxnResponse(
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
	}

	ActiveObject::UponActiveObjectEvent(ptr);
}

ApiErrorCode
MrcpSession::Allocate()
{
	FUNCTRACKER;

	return Allocate(CnxInfo(),MediaFormat()); 
}

ApiErrorCode
MrcpSession::Allocate(IN CnxInfo remote_end, 
								  IN MediaFormat codec)
{
	FUNCTRACKER;

	LogDebug("MrcpSession::Allocate remote:" <<  remote_end.ipporttos()  << ", codec:"  << codec << ", mrcph:" << _mrcpSessionHandle);
	
	if (_mrcpSessionHandle != IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

	MsgMrcpAllocateSessionReq *msg = new MsgMrcpAllocateSessionReq();
	msg->remote_media_data = remote_end;
	msg->codec = codec;
	msg->session_handler = session_handler_pair;

	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		MRCP_Q,
		IwMessagePtr(msg),
		response,
		Seconds(15),
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
			_mrcpSessionHandle	= ack->mrcp_handle;
			_mrcpMediaData		= ack->mrcp_media_data;

			StartActiveObjectLwProc(_forking,session_handler_pair,"Mrcp Session handler");

			LogDebug("Mrcp session allocated successfully, mrcp handle=[" << _mrcpSessionHandle << "]");

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
MrcpSession::ModifySession(IN CnxInfo remote_end, 
								  IN MediaFormat codec)
{
	FUNCTRACKER;

	LogDebug("MrcpSession::ModifyConnection remote:" <<  remote_end.ipporttos()  << ", codec:"  << codec << ", mrcph:" << _mrcpSessionHandle);

	if (_mrcpSessionHandle == IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

	MsgMrcpModifyReq *msg = new MsgMrcpModifyReq();
	msg->remote_media_data = remote_end;
	msg->codec = codec;
	msg->mrcp_handle = _mrcpSessionHandle;

	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		MRCP_Q,
		IwMessagePtr(msg),
		response,
		MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
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


CnxInfo MrcpSession::MrcpMediaData() const 
{ 
	return _mrcpMediaData; 
}

void MrcpSession::MrcpMediaData(IN CnxInfo val) 
{ 
	_mrcpMediaData = val; 
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

	ApiErrorCode res = GetCurrLightWeightProc()->SendMessage(MRCP_Q,IwMessagePtr(tear_req));

}

}