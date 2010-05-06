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
#include "ICall.h"
#include "Logger.h"
#include "ActiveObject.h"



namespace ivrworx
{

IwStackHandle 
GenerateCallHandle()
{
	static volatile int index = 10000;
	return ::InterlockedIncrement((LONG*)&index);
};


ICall::ICall(IN ScopedForking &forking, 
			 IN HandleId stack_handle_id):
	_stackHandleId(stack_handle_id),
	_iwCallHandle(IW_UNDEFINED),
	_hangupDetected(FALSE),
	_handlerPair(HANDLE_PAIR),
	_dtmfChannel(new LpHandle()),
	_hangupChannel(new LpHandle()),
	_callState(CALL_STATE_UNKNOWN)
{
	FUNCTRACKER;

	CALL_RESET_STATE(CALL_STATE_UNKNOWN);

	StartActiveObjectLwProc(forking,_handlerPair,"ICall Session Handler");

	_iwCallHandle = GenerateCallHandle();


}

ICall::ICall(IN ScopedForking &forking,
			 IN HandleId stack_handle_id,
			 IN shared_ptr<MsgCallOfferedReq> offered_msg):
	_stackHandleId(stack_handle_id),
	_iwCallHandle(offered_msg->stack_call_handle),
	_remoteMedia(offered_msg->remote_media),
	_handlerPair(offered_msg->call_handler_inbound),
	_dtmfChannel(new LpHandle()),
	_hangupChannel(new LpHandle()),
	_callState(CALL_STATE_UNKNOWN)
{
	FUNCTRACKER;

	CALL_RESET_STATE(CALL_STATE_INITIAL_OFFERED);
	StartActiveObjectLwProc(forking,_handlerPair,__FUNCTION__);

	_dnis	= offered_msg->dnis;
	_ani	= offered_msg->ani;

}

const string&
ICall::Dnis()
{
	return _dnis;
}

const string&
ICall::Ani()
{
	return _ani;
}

const MediaFormat& 
ICall::AcceptedSpeechCodec()
{
	return _acceptedSpeechFormat;
}

void
ICall::ResetState(CallState state, const char *state_str)
{
	LogDebug("ICall iwh:" << _iwCallHandle << ", transition from state:" << _callState << " to state:" << state << ", " << state_str);
	_callState = state;
}

void
ICall::EnableMediaFormat(IN const MediaFormat& media_format)
{
	FUNCTRACKER;

	LogDebug("ICall::EnableMediaFormat - Media format:" << media_format << ", enabled for iwh:" << _iwCallHandle );

	_supportedMediaFormatsMap.insert(
		MediaFormatMapPair(media_format.sdp_mapping(), media_format));
}

ApiErrorCode
ICall::NegotiateMediaFormats(IN const MediaFormatsList &offered_medias, 
							   OUT MediaFormatsList &accepted_media,
							   OUT MediaFormat &accepted_speech_format)
{
	FUNCTRACKER;

	for (MediaFormatsList::const_iterator i = offered_medias.begin();
		i != offered_medias.end();
		++i)
	{
		LogDebug("ICall::NegotiateMediaFormats offered:" << (*i).sdp_name_tos() << ", iwh:" << _iwCallHandle);
	}

	for (MediaFormatsMap::const_iterator i = _supportedMediaFormatsMap.begin();
		i != _supportedMediaFormatsMap.end();
		++i)
	{
		LogDebug("ICall::NegotiateMediaFormats enabled:" << (*i).second.sdp_name_tos() << ", iwh:" << _iwCallHandle);
	}

	if (_supportedMediaFormatsMap.empty() || offered_medias.empty())
	{
		LogWarn("ICall::NegotiateMediaFormats - Illegal parameters (empty?) for codecs negotiation.");
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
			_supportedMediaFormatsMap.find(media_format1.sdp_mapping());
		if (iter2 == _supportedMediaFormatsMap.end())
		{
			continue;
		}

		const MediaFormat &media_format2 = (*iter2).second;

		// have we already found speech codec?
		if (media_format1.get_media_type() == MediaFormat::MediaType_SPEECH && speech_chosen)
			continue;

		// have we already found dtmf codec?
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
		LogWarn("ICall::NegotiateMediaFormats - negotiation failure iwh:" << _iwCallHandle );
		return API_FAILURE;
	}

	LogDebug("ICall::NegotiateMediaFormats speech:" << accepted_speech_format << ", rc2833 dtmf chosen:" << dtmf_chosen << ", iwh:" << _iwCallHandle);

	return API_SUCCESS;

}

ICall::~ICall(void)
{
	FUNCTRACKER;

	StopActiveObjectLwProc();

	HangupCall();


}

void 
ICall::UponDtmfEvt(IwMessagePtr ptr)
{
	FUNCTRACKER;

	// just proxy the event
	_dtmfChannel->Send(ptr);

}

void 
ICall::CleanDtmfBuffer()
{
	FUNCTRACKER;

	// the trick we are using
	// to clean the buffer is just replace the handle
	// with  new one
	_dtmfChannel->Poison();
	_dtmfChannel = LpHandlePtr(new LpHandle());

	LogDebug("ICall::CleanDtmfBuffer iwh:" << _iwCallHandle);

}


ApiErrorCode 
ICall::WaitForDtmf(OUT string &signal, IN const Time timeout)
{
	FUNCTRACKER;

	// just proxy the event
	int handle_index= IW_UNDEFINED;
	IwMessagePtr response = NULL_MSG;

	ApiErrorCode res = GetCurrLightWeightProc()->WaitForTxnResponse(
		assign::list_of(_dtmfChannel)(_hangupChannel),
		handle_index,
		response, 
		timeout);

	if (IW_FAILURE(res))
	{
		return res;
	}

	if (handle_index == 1)
	{
		return API_HANGUP;
	}

	shared_ptr<MsgCallDtmfEvt> dtmf_evt = 
		dynamic_pointer_cast<MsgCallDtmfEvt> (response);

	signal = dtmf_evt->signal;

	LogDebug("ICall::WaitForDtmf iwh:" << _iwCallHandle << " received signal:" << signal);

	return API_SUCCESS;


}



void 
ICall::UponActiveObjectEvent(IwMessagePtr ptr)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case MSG_CALL_HANG_UP_EVT:
		{

			UponCallTerminated(ptr);
			break;
		}
	case MSG_CALL_DTMF_EVT:
		{

			UponDtmfEvt(ptr);
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
ICall::UponCallTerminated(IwMessagePtr ptr)
{
	_hangupChannel->Send(new MsgCallHangupEvt());
	CALL_RESET_STATE(CALL_STATE_TERMINATED);
}


ApiErrorCode
ICall::RejectCall()
{
	FUNCTRACKER;

	LogDebug("ICall::RejectCall iwh:" << _iwCallHandle );

	MsgCallOfferedNack *msg = new MsgCallOfferedNack();
	msg->stack_call_handle = _iwCallHandle;

	ApiErrorCode res = GetCurrLightWeightProc()->SendMessage(_stackHandleId,msg);
	return res;

}

ApiErrorCode
ICall::HangupCall()
{

	FUNCTRACKER;

	LogDebug("ICall::HangupCall iwh:" << _iwCallHandle);

	if (_iwCallHandle == IW_UNDEFINED || 
		_callState == CALL_STATE_UNKNOWN || 
		_callState == CALL_STATE_TERMINATED)
	{
		return API_SUCCESS;
	}

	MsgHangupCallReq *msg = new MsgHangupCallReq(_iwCallHandle);

	ApiErrorCode res = GetCurrLightWeightProc()->SendMessage(_stackHandleId,msg);

	_iwCallHandle = IW_UNDEFINED;
	_hangupDetected = TRUE;

	return res;
}

ApiErrorCode
ICall::AcceptInitialOffer( IN const CnxInfo &local_connection, 
							IN const MediaFormatsList &accepted_media_formats_list,
							IN const MediaFormat &speech_codec)
{

	FUNCTRACKER;

	LogDebug("ICall::AcceptInitialOffer iwh:" << _iwCallHandle);

	if (_callState != CALL_STATE_INITIAL_OFFERED)
	{
		LogDebug("AcceptInitialOffer:: wrong call state:" << _callState << ", iwh:" << _iwCallHandle);
		return API_WRONG_STATE;
	}


	_acceptedSpeechFormat = speech_codec;
	_acceptedMediaFormats = accepted_media_formats_list;

	IwMessagePtr response = NULL_MSG;

	MsgCalOfferedAck *ack	= new MsgCalOfferedAck();
	ack->stack_call_handle	= _iwCallHandle;
	ack->local_media		= local_connection;
	ack->accepted_codecs	= accepted_media_formats_list;


	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		_stackHandleId,
		IwMessagePtr(ack),
		response,
		MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
		"Accept ICall TXN");

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

			_iwCallHandle = make_call_sucess->stack_call_handle;

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
ICall::StackCallHandle() const 
{ 
	return _iwCallHandle; 
}

void
ICall::WaitTillHangup()
{
	FUNCTRACKER;

	LogDebug("ICall::WaitTillHangup iwh:" << _iwCallHandle);

	ApiErrorCode res = API_TIMEOUT;
	IwMessagePtr response;
	while (res == API_TIMEOUT)
	{
		res = GetCurrLightWeightProc()->WaitForTxnResponse(_hangupChannel,response,Seconds(100));
	}

}

ApiErrorCode
ICall::BlindXfer(IN const string &destination_uri)
{
	FUNCTRACKER;

	LogDebug("ICall::BlindXfer iwh:" << _iwCallHandle << " dest:" << destination_uri);

	if (_iwCallHandle == IW_UNDEFINED)
	{
		return API_FAILURE;
	}


	IwMessagePtr response = NULL_MSG;

	MsgCallBlindXferReq *msg = new MsgCallBlindXferReq();
	msg->destination_uri = destination_uri;
	msg->stack_call_handle = _iwCallHandle;

	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		_stackHandleId,
		IwMessagePtr(msg),
		response,
		Seconds(60),
		"Blind Xfer ICall TXN");

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
ICall::MakeCall(IN const string &destination_uri, 
				  IN const CnxInfo &local_media,
				  IN csp::Time ring_timeout)
{
	FUNCTRACKER;

	LogDebug("ICall::MakeCall dst:" << destination_uri <<", lci:" << local_media);

	if (_callState != CALL_STATE_UNKNOWN)
	{
		return API_FAILURE;
	}

	CALL_RESET_STATE(CALL_STATE_OFFERING);

	_localMedia = local_media;

	IwMessagePtr response = NULL_MSG;

	MsgMakeCallReq *msg = new MsgMakeCallReq();
	msg->local_media = local_media;
	msg->destination_uri = destination_uri;
	msg->call_handler_inbound = _handlerPair.inbound;
	msg->stack_call_handle = _iwCallHandle;


	// wait for ok or nack
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		_stackHandleId,
		IwMessagePtr(msg),
		response,
		ring_timeout,
		"Make ICall TXN");


	if (res == API_TIMEOUT)
	{
		// just to be sure that timeout-ed call
		// eventually gets hanged up
		LogWarn("ICall::MakeCall - Timeout.");
		HangupCall();
		return API_TIMEOUT;
	}

	if (IW_FAILURE(res))
	{
		LogWarn("ICall::MakeCall - failure res:" << res);
		CALL_RESET_STATE(CALL_STATE_UNKNOWN);
		return API_SERVER_FAILURE;
	}


	switch (response->message_id)
	{
	case MSG_MAKE_CALL_OK:
		{
			break;
		}
	case MSG_MAKE_CALL_NACK:
		{
			CALL_RESET_STATE(CALL_STATE_UNKNOWN);
			return API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}


	shared_ptr<MsgMakeCallOk> make_call_ok = 
		dynamic_pointer_cast<MsgMakeCallOk>(response);

	_iwCallHandle = make_call_ok->stack_call_handle;
	_remoteMedia = make_call_ok->remote_media;

	MediaFormatsList accepted_media_formats;
	MediaFormat speech_media_format = MediaFormat::PCMU;

	res = this->NegotiateMediaFormats(
		make_call_ok->offered_codecs, 
		accepted_media_formats, 
		speech_media_format);

	if (IW_FAILURE(res))
	{
		HangupCall();
		CALL_RESET_STATE(CALL_STATE_UNKNOWN);
		return API_SERVER_FAILURE;
	};

	_acceptedMediaFormats = accepted_media_formats;
	_acceptedSpeechFormat = speech_media_format;

	CALL_RESET_STATE(CALL_STATE_CONNECTED);

	MsgMakeCallAckReq* ack_req = new MsgMakeCallAckReq();
	ack_req->stack_call_handle = _iwCallHandle;

	res = GetCurrLightWeightProc()->SendMessage(_stackHandleId,
		ack_req);

	if (IW_FAILURE(res))
	{
		HangupCall();
		CALL_RESET_STATE(CALL_STATE_UNKNOWN);
		return API_SERVER_FAILURE;
	};

	_dnis = destination_uri;
	_ani  = make_call_ok->ani;

	CALL_RESET_STATE(CALL_STATE_CONNECTED);

	return res;
}

CnxInfo 
ICall::RemoteMedia() const 
{ 
	return _remoteMedia; 
}

void 
ICall::RemoteMedia(CnxInfo &val) 
{ 
	_remoteMedia = val; 
}


CnxInfo 
ICall::LocalMedia() const 
{ 
	return _localMedia; 
}

void 
ICall::LocalMedia(CnxInfo &val) 
{ 
	_localMedia = val; 
}

}
