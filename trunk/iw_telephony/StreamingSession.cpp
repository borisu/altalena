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
#include "StreamingSession.h"



using namespace boost;
using namespace boost::assign;

namespace ivrworx
{


StreamingSession::StreamingSession(IN ScopedForking &forking, IN HandleId streamerHandleId):
_streamerHandleId(streamerHandleId),
_streamingSessionHandle(IW_UNDEFINED),
_forking(forking),
_hangupHandle(new LpHandle()),
_playStoppedHandle(new LpHandle()),
_state(STREAMER_SESSION_STATE_NOT_PLAYING)
{
	FUNCTRACKER;
}

StreamingSession::~StreamingSession(void)
{
	FUNCTRACKER;

	StopActiveObjectLwProc();

	if (_streamingSessionHandle != IW_UNDEFINED)
	{
		TearDown();
	}

	_hangupHandle->Poison();

	
}

ApiErrorCode
StreamingSession::StopPlay()
{
	FUNCTRACKER;

	if (_streamingSessionHandle == IW_UNDEFINED)
	{
		return API_WRONG_STATE;
	}

	if (_state == STREAMER_SESSION_STATE_NOT_PLAYING)
	{
		return API_SUCCESS;
	}

	MsgStreamStopPlayReq *msg = new MsgStreamStopPlayReq();
	msg->streamer_handle	= _streamingSessionHandle;
	
	ApiErrorCode res = GetCurrRunningContext()->SendMessage(_streamerHandleId,IwMessagePtr(msg));
	return res;

}

ApiErrorCode
StreamingSession::PlayFile(IN const string &file_name,
					 IN BOOL sync,
					 IN BOOL loop)
{

	FUNCTRACKER;

	if (_streamingSessionHandle == IW_UNDEFINED)
	{
		return API_WRONG_STATE;
	}

	// cannot sync and loop
	if (sync == TRUE && loop == TRUE && file_name != "")
	{
		LogWarn("StreamingSession::PlayFile - Cannot play sync and in loop.");
		return API_FAILURE;
	}

	_state = STREAMER_SESSION_STATE_UNKNOWN;

	IwMessagePtr response = NULL_MSG;
	int handle_index = IW_UNDEFINED;
	
	
	MsgStreamPlayReq *msg = new MsgStreamPlayReq();
	msg->streamer_handle	= _streamingSessionHandle;
	msg->file_name			= file_name;
	msg->loop				= loop;
	
	
	ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_streamerHandleId,
		IwMessagePtr(msg),
		response,
		Seconds(5),
		"Plat TXN");

	if (IW_FAILURE(res) || response->message_id != MSG_STREAM_PLAY_ACK)
	{
		LogDebug("StreamingSession::PlayFile - Error sending play request to Stream, Streamh:" << _streamingSessionHandle);
		return res;
	}

	if (!sync)
	{
		return API_SUCCESS;
	}

	shared_ptr<MsgStreamPlayAck> ack = 
		dynamic_pointer_cast<MsgStreamPlayAck>(response);


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

		shared_ptr<MsgStreamPlayStopped> stopped_evt = 
			dynamic_pointer_cast<MsgStreamPlayStopped>(response);

		if (stopped_evt->correlation_id == ack->correlation_id)
		{
			_state = STREAMER_SESSION_STATE_NOT_PLAYING;
			return API_SUCCESS;
		}

	} 
}

AbstractOffer
StreamingSession::LocalOffer()
{
	return _localOffer;

}

AbstractOffer 
StreamingSession::RemoteOffer()
{
	return _remoteOffer;
}

void
StreamingSession::UponActiveObjectEvent(IwMessagePtr ptr)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case MSG_STREAM_PLAY_STOPPED_EVT:
		{
			_playStoppedHandle->Send(ptr);
		}
	default:
		{

		}
	};

	ActiveObject::UponActiveObjectEvent(ptr);
}



StreamerHandle
StreamingSession::SessionHandle()
{
	return _streamingSessionHandle;
}

ApiErrorCode
StreamingSession::Allocate(IN const AbstractOffer &remote_end, 
					       IN RcvDeviceType rcvDeviceType,
						   IN SndDeviceType sndDeviceType)
{
	FUNCTRACKER;

	LogDebug("StreamingSession::Allocate dest ci:" <<  remote_end.body  << ", Streamh:" << _streamingSessionHandle);
	
	if (_streamingSessionHandle != IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

	MsgStreamAllocateSessionReq *msg = new MsgStreamAllocateSessionReq();
	msg->offer = remote_end;
	msg->session_handler = session_handler_pair;
	msg->rcv_device_type = rcvDeviceType;
	msg->snd_device_type = sndDeviceType;

	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_streamerHandleId,
		IwMessagePtr(msg),
		response,
		MilliSeconds(GetCurrRunningContext()->TransactionTimeout()),
		"Allocate Stream Connection TXN");

	if (res != API_SUCCESS)
	{
		LogWarn("Error allocating Stream connection " << res);
		return res;
	}

	switch (response->message_id)
	{
	case MSG_STREAM_ALLOCATE_SESSION_ACK:
		{


			shared_ptr<MsgStreamAllocateSessionAck> ack = 
				shared_polymorphic_cast<MsgStreamAllocateSessionAck>(response);
			_streamingSessionHandle	= ack->streamer_handle;
			_localOffer = ack->offer;
			

			StartActiveObjectLwProc(_forking,session_handler_pair,"Stream Session handler");

			LogDebug("Stream session allocated successfully, Streamh:" << _streamingSessionHandle );

			break;

		}
	case MSG_STREAM_ALLOCATE_SESSION_NACK:
		{
			LogDebug("Error allocating Stream session.");
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
StreamingSession::InterruptWithHangup()
{
	FUNCTRACKER;

	_hangupHandle->Send(new MsgStreamTearDownReq());
}

ApiErrorCode
StreamingSession::ModifyConnection(IN const AbstractOffer &remote_end)
{
	FUNCTRACKER;

	LogDebug("StreamingSession::ModifyConnection remote:" <<  remote_end.body << ", Streamh:" << _streamingSessionHandle);

	if (_streamingSessionHandle == IW_UNDEFINED)
	{
		return API_FAILURE;
	}

	DECLARE_NAMED_HANDLE_PAIR(session_handler_pair);

	MsgStreamModifyReq *msg = new MsgStreamModifyReq();
	msg->offer = remote_end;
	msg->streamer_handle = _streamingSessionHandle;
	

	IwMessagePtr response = NULL_MSG;
	ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
		_streamerHandleId,
		IwMessagePtr(msg),
		response,
		MilliSeconds(GetCurrRunningContext()->TransactionTimeout()),
		"Modify Stream Connection TXN");

	if (res != API_SUCCESS)
	{
		LogWarn("Error modifying Stream connection " << res);
		return res;
	}

	switch (response->message_id)
	{
	case MSG_STREAM_MODIFY_ACK:
		{
			_remoteOffer = remote_end;
			return API_SUCCESS;
		}
	case MSG_STREAM_MODIFY_NACK:
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
StreamingSession::TearDown()
{
	if(_streamingSessionHandle == IW_UNDEFINED)
	{
		return;
	}

	MsgStreamTearDownReq *tear_req = new MsgStreamTearDownReq();
	tear_req->streamer_handle = _streamingSessionHandle;

	// no way back
	_streamingSessionHandle = IW_UNDEFINED;
	_state = STREAMER_SESSION_STATE_UNKNOWN;

	ApiErrorCode res = GetCurrRunningContext()->SendMessage(_streamerHandleId,IwMessagePtr(tear_req));

}

}