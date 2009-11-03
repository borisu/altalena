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


using namespace boost;
using namespace boost::assign;

namespace ivrworx
{



ImsSession::ImsSession(IN ScopedForking &forking):
_imsSessionHandle(IW_UNDEFINED),
_imsSessionHandlerPair(HANDLE_PAIR),
_forking(forking),
_hangupHandle(new LpHandle()),
_playStoppedHandle(new LpHandle())
{
	FUNCTRACKER;
}

ImsSession::~ImsSession(void)
{
	FUNCTRACKER;

	StopActiveObjectLwProc();

	if (_imsSessionHandle != IW_UNDEFINED)
	{
		TearDown();
	}

	_hangupHandle->Poison();

	
}

ApiErrorCode
ImsSession::StopPlay()
{
	FUNCTRACKER;

	if (_imsSessionHandle == IW_UNDEFINED)
	{
		return API_WRONG_STATE;
	}

	MsgImsStopPlayReq *msg = new MsgImsStopPlayReq();
	msg->ims_handle	= _imsSessionHandle;
	
	ApiErrorCode res = GetCurrLightWeightProc()->SendMessage(IMS_Q,IwMessagePtr(msg));
	return res;

}

ApiErrorCode
ImsSession::PlayFile(IN const string &file_name,
					 IN BOOL sync,
					 IN BOOL loop)
{

	FUNCTRACKER;

	if (_imsSessionHandle == IW_UNDEFINED)
	{
		return API_WRONG_STATE;
	}

	// cannot sync and loop
	if (sync == TRUE && loop == TRUE)
	{
		LogWarn("ImsSession::PlayFile - Cannot play sync and in loop.");
		return API_FAILURE;
	}

	IwMessagePtr response = NULL_MSG;
	int handle_index = IW_UNDEFINED;
	
	
	MsgImsPlayReq *msg = new MsgImsPlayReq();
	msg->ims_handle	= _imsSessionHandle;
	msg->file_name			= file_name;
	msg->loop				= loop;
	
	
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		IMS_Q,
		IwMessagePtr(msg),
		response,
		Seconds(5),
		"Plat TXN");

	if (IW_FAILURE(res) || response->message_id != MSG_IMS_PLAY_ACK)
	{
		LogDebug("ImsSession::PlayFile - Error sending play request to ims, imsh:" << _imsSessionHandle);
		return res;
	}

	if (!sync)
	{
		return API_SUCCESS;
	}

	shared_ptr<MsgImsPlayAck> ack = 
		dynamic_pointer_cast<MsgImsPlayAck>(response);


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

		shared_ptr<MsgImsPlayStopped> stopped_evt = 
			dynamic_pointer_cast<MsgImsPlayStopped>(response);

		if (stopped_evt->correlation_id == ack->correlation_id)
		{
			return API_SUCCESS;
		}

	} 
}



void
ImsSession::UponActiveObjectEvent(IwMessagePtr ptr)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case MSG_IMS_PLAY_STOPPED_EVT:
		{
			_playStoppedHandle->Send(ptr);
		}
	default:
		{

		}
	};

	ActiveObject::UponActiveObjectEvent(ptr);
}

ApiErrorCode
ImsSession::Allocate(IN const CnxInfo &local_end)
{
	FUNCTRACKER;

	return Allocate(local_end,CnxInfo(),MediaFormat()); 
		

}

ImsHandle
ImsSession::SessionHandle()
{
	return _imsSessionHandle;
}

ApiErrorCode
ImsSession::Allocate(IN const CnxInfo &local_end, 
					 IN const CnxInfo &remote_end, 
					 IN const MediaFormat &codec)
{
	FUNCTRACKER;

	LogDebug("ImsSession::Allocate - lci:" << local_end.ipporttos() <<", rci:" <<  remote_end.ipporttos()  << ", codec:"  << codec << ", imsh:" << _imsSessionHandle);
	
	if (_imsSessionHandle != IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	_imsMediaData = local_end;

	DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

	MsgImsAllocateSessionReq *msg = new MsgImsAllocateSessionReq();
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
	case MSG_IMS_ALLOCATE_SESSION_ACK:
		{


			shared_ptr<MsgImsAllocateSessionAck> ack = 
				shared_polymorphic_cast<MsgImsAllocateSessionAck>(response);
			_imsSessionHandle	= ack->ims_handle;
			

			StartActiveObjectLwProc(_forking,session_handler_pair,"Ims Session handler");

			LogDebug("Ims session allocated successfully, imsh:" << _imsSessionHandle );

			break;

		}
	case MSG_IMS_ALLOCATE_SESSION_NACK:
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

void
ImsSession::InterruptWithHangup()
{
	FUNCTRACKER;

	_hangupHandle->Send(new MsgImsTearDownReq());
}

ApiErrorCode
ImsSession::ModifyConnection(IN const CnxInfo &remote_end, 
							 IN const MediaFormat &codec)
{
	FUNCTRACKER;

	LogDebug("ImsSession::ModifyConnection remote:" <<  remote_end.ipporttos()  << ", codec:"  << codec << ", imsh:" << _imsSessionHandle);

	if (_imsSessionHandle == IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

	MsgImsModifyReq *msg = new MsgImsModifyReq();
	msg->remote_media_data = remote_end;
	msg->codec = codec;
	msg->ims_handle = _imsSessionHandle;

	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		IMS_Q,
		IwMessagePtr(msg),
		response,
		MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
		"Modify IMS Connection TXN");

	if (res != API_SUCCESS)
	{
		LogWarn("Error modifying Ims connection " << res);
		return res;
	}

	switch (response->message_id)
	{
	case MSG_IMS_MODIFY_ACK:
		{
			return API_SUCCESS;
		}
	case MSG_IMS_MODIFY_NACK:
		{
			return API_FAILURE;
		}
	default:
		{
			throw;
		}
	}

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
	tear_req->ims_handle = _imsSessionHandle;

	// no way back
	_imsSessionHandle = IW_UNDEFINED;

	ApiErrorCode res = GetCurrLightWeightProc()->SendMessage(IMS_Q,IwMessagePtr(tear_req));

}

}