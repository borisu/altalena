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
#include "Call.h"
#include "Profiler.h"
#include "Logger.h"


#define CALL_RESET_STATE(X) ResetState(X,#X)

namespace ivrworx
{



Call::Call(IN LpHandlePair stack_pair):
_stackPair(stack_pair),
_stackCallHandle(IW_UNDEFINED),
_hangupDetected(FALSE),
_handlerPair(HANDLE_PAIR),
_dtmfChannel(new LpHandle()),
_hangupChannel(new LpHandle()),
_callState(CALL_STATE_UNKNOWN)
{
	CALL_RESET_STATE(CALL_STATE_UNKNOWN);
	throw "not supported";

}

Call::Call(
	 IN LpHandlePair stack_pair,
	 IN ScopedForking &forking,
	 IN shared_ptr<MsgCallOfferedReq> offered_msg):
 _stackPair(stack_pair),
 _stackCallHandle(offered_msg->stack_call_handle),
 _remoteMedia(offered_msg->remote_media),
 _handlerPair(offered_msg->call_handler_inbound),
 _dtmfChannel(new LpHandle()),
 _hangupChannel(new LpHandle()),
 _callState(CALL_STATE_UNKNOWN)
{
	FUNCTRACKER;

	CALL_RESET_STATE(CALL_STATE_INITIAL_OFFERED);
	Start(forking,_handlerPair,__FUNCTION__);

	_dnis	= offered_msg->dnis;
	_ani	= offered_msg->ani;

}

const string&
Call::Dnis()
{
  return _dnis;
}

const string&
Call::Ani()
{
	return _ani;
}

void
Call::ResetState(CallState state, const char *state_str)
{
	LogDebug("Call iwh:" << _stackCallHandle << ", transition from state:" << _callState << " to state:" << state << ", " << state_str);
	_callState = state;
}

void
Call::EnableMediaFormat(const MediaFormat& media_format)
{
	FUNCTRACKER;

	LogDebug("Media format:" << media_format << ", enabled for iwh:" << _stackCallHandle );

	_supportedMediaFormatsList.insert(
		MediaFormatMapPair(media_format.sdp_mapping(), media_format));
}

ApiErrorCode
Call::NegotiateMediaFormats(IN const MediaFormatsList &offered_medias, 
					  OUT MediaFormatsList &accepted_media,
					  OUT MediaFormat &accepted_speech_format)
{
	FUNCTRACKER;

	IX_PROFILE_FUNCTION();

	if (_supportedMediaFormatsList.empty() || offered_medias.empty())
	{
		LogWarn("Illegal parameters (empty?) for codecs negotiation.");
		return API_FAILURE;
	}

	accepted_media.clear();

	bool speech_chosen = false;
	bool dtmf_chosen = false;

	// firstly negotiate speech codec
	for (MediaFormatsList::const_iterator iter1 = offered_medias.begin(); 
		iter1 != offered_medias.end();
		iter1++)
	{


		const MediaFormat &media_format1 = (*iter1);

		MediaFormatsMap::iterator iter2 = 
			_supportedMediaFormatsList.find(media_format1.sdp_mapping());
		if (iter2 == _supportedMediaFormatsList.end())
		{
			continue;
		}

		const MediaFormat &media_format2 = (*iter2).second;

		if (media_format1.get_media_type() == MediaFormat::MediaType_SPEECH && speech_chosen)
			continue;

		if (media_format1.get_media_type() == MediaFormat::MediaType_DTMF && dtmf_chosen)
			continue;

		if (media_format2 == media_format1)
		{
			accepted_media.push_back(media_format1);
			switch (media_format1.get_media_type())
			{
			case MediaFormat::MediaType_DTMF:
				{ 
					dtmf_chosen = true; 
					break;
				}
			case MediaFormat::MediaType_SPEECH:
				{
					speech_chosen = true;
					accepted_speech_format = media_format2;
					break;
				}
			}
		}

		if (dtmf_chosen && speech_chosen)
		{
			break;
		}
	}


	if (accepted_media.empty() || speech_chosen == false)
	{
		LogWarn("Negotiation failure for call " << _stackCallHandle );
		return API_FAILURE;
	}

	return API_SUCCESS;

}

Call::~Call(void)
{
	FUNCTRACKER;

	HagupCall();
}

void 
Call::UponActiveObjectEvent(IwMessagePtr ptr)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case MSG_CALL_HANG_UP_EVT:
		{
			
			UponCallTerminated(ptr);
			break;
		}
	default:
		{
			throw;
		}
	}

	ActiveObject::UponActiveObjectEvent(ptr);

}

void 
Call::UponCallTerminated(IwMessagePtr ptr)
{
	_hangupChannel->Send(new MsgCallHangupEvt());
	CALL_RESET_STATE(CALL_STATE_TERMINATED);
}


ApiErrorCode
Call::RejectCall()
{
	FUNCTRACKER;

	LogDebug("RejectCall:: iwh:" << _stackCallHandle );

	MsgCallOfferedNack *msg = new MsgCallOfferedNack();
	msg->stack_call_handle = _stackCallHandle;

	_stackPair.inbound->Send(msg);

	return API_SUCCESS;

}

ApiErrorCode
Call::HagupCall()
{
	
	FUNCTRACKER;

	LogDebug("HagupCall:: iwh:" << _stackCallHandle);

	if (_stackCallHandle == IW_UNDEFINED || 
		_callState == CALL_STATE_UNKNOWN || 
		_callState == CALL_STATE_TERMINATED)
	{
		return API_SUCCESS;
	}

	MsgHangupCallReq *msg = new MsgHangupCallReq(_stackCallHandle);

	_stackPair.inbound->Send(msg);

	_stackCallHandle = IW_UNDEFINED;

	_hangupDetected = TRUE;

	return API_SUCCESS;
}

ApiErrorCode
Call::AcceptInitialOffer( IN const CnxInfo &local_connection, 
						 IN const MediaFormatsList &accepted_media_formats_list,
						 IN const MediaFormat &speech_codec)
{
	
	FUNCTRACKER;
	IX_PROFILE_FUNCTION();

	LogDebug("AcceptInitialOffer:: iwh:" << _stackCallHandle);

	
	_acceptedSpeechFormat = speech_codec;
	_acceptedMediaFormats = accepted_media_formats_list;

	IwMessagePtr response = NULL_MSG;
	
	MsgCalOfferedAck *ack	= new MsgCalOfferedAck();
	ack->stack_call_handle	= _stackCallHandle;
	ack->local_media		= local_connection;
	ack->accepted_codecs	= accepted_media_formats_list;
	

	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		_stackPair.inbound,
		IwMessagePtr(ack),
		response,
		MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
		"Accept Call TXN");

	if (IW_FAILURE(res))
	{
		return res;
	}

	switch (response->message_id)
	{
	case MSG_CALL_CONNECTED:
		{
			shared_ptr<MsgNewCallConnected> make_call_sucess = 
				dynamic_pointer_cast<MsgNewCallConnected>(response);

			_stackCallHandle = make_call_sucess->stack_call_handle;

			break;
		}
	default:
		{
			throw;
		}
	}

	_callState = CALL_STATE_CONNECTED;

	return res;
}

int 
Call::StackCallHandle() const 
{ 
	return _stackCallHandle; 
}

void
Call::WaitTillHangup()
{
	FUNCTRACKER;

	LogDebug("WaitTillHangup:: iwh:" << _stackCallHandle);

	ApiErrorCode res = API_TIMEOUT;
	IwMessagePtr response;
	while (res == API_TIMEOUT)
	{
		res = GetCurrLightWeightProc()->WaitForTxnResponse(_hangupChannel,response,Seconds(100));
	}

}

ApiErrorCode
Call::BlindXfer(IN const string &destination_uri)
{
	FUNCTRACKER;

	LogDebug("BlindXfer:: iwh:" << _stackCallHandle << " dest:" << destination_uri);

	if (_stackCallHandle == IW_UNDEFINED)
	{
		return API_FAILURE;
	}


	IwMessagePtr response = NULL_MSG;

	MsgCallBlindXferReq *msg = new MsgCallBlindXferReq();
	msg->destination_uri = destination_uri;
	msg->stack_call_handle = _stackCallHandle;

	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		_stackPair.inbound,
		IwMessagePtr(msg),
		response,
		Seconds(60),
		"Blind Xfer Call TXN");

	if (res != API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case MSG_CALL_BLIND_XFER_ACK:
		{
			CALL_RESET_STATE(CALL_STATE_TERMINATED);
			break;
		}
	case MSG_CALL_BLIND_XFER_NACK:
		{
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


ApiErrorCode
Call::MakeCall(IN const string &destination_uri, 
			   IN const CnxInfo &local_media)
{
	FUNCTRACKER;

	return API_FAILURE;

	_localMedia = local_media;

	IwMessagePtr response = NULL_MSG;

	MsgMakeCallReq *msg = new MsgMakeCallReq();
	msg->local_media = local_media;
	msg->destination_uri = destination_uri;
	msg->call_handler_inbound = _handlerPair.inbound;


	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		_stackPair.inbound,
		IwMessagePtr(msg),
		response,
		Seconds(60),
		"Make Call TXN");

	if (res != API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case MSG_MAKE_CALL_ACK:
		{
			shared_ptr<MsgMakeCallAck> make_call_sucess = 
				dynamic_pointer_cast<MsgMakeCallAck>(response);

			_stackCallHandle = make_call_sucess->stack_call_handle;

			_remoteMedia = make_call_sucess->remote_media;

			break;
		}
	case MSG_MAKE_CALL_NACK:
		{
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

CnxInfo 
Call::RemoteMedia() const 
{ 
	return _remoteMedia; 
}

void 
Call::RemoteMedia(CnxInfo &val) 
{ 
	_remoteMedia = val; 
}


CnxInfo 
Call::LocalMedia() const 
{ 
	return _localMedia; 
}

void 
Call::LocalMedia(CnxInfo &val) 
{ 
	_localMedia = val; 
}

}
