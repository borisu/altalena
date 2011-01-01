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
#include "MediaCallSession.h"
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


CallSession::CallSession(IN ScopedForking &forking, 
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

	StartActiveObjectLwProc(forking,_handlerPair,"CallSession Session Handler");

	_iwCallHandle = GenerateCallHandle();


}

CallSession::CallSession(IN ScopedForking &forking,
			 IN HandleId stack_handle_id,
			 IN shared_ptr<MsgCallOfferedReq> offered_msg):
	_stackHandleId(stack_handle_id),
	_iwCallHandle(offered_msg->stack_call_handle),
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

ApiErrorCode
SubscribeToIncomingCalls(IN LpHandlePtr stackIncomingHandle, IN LpHandlePtr listenerHandle)
{
	FUNCTRACKER;

	MsgCallSubscribeReq *msg = new MsgCallSubscribeReq();
	msg->listener_handle = listenerHandle;

	IwMessagePtr response = NULL_MSG;
	

	// wait for ok or nack
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		stackIncomingHandle,
		IwMessagePtr(msg),
		response,
		Seconds(5),
		"SubscribeToIncomingCalls TXN");


	if (res == API_TIMEOUT)
	{
		// just to be sure that timeout-ed call
		// eventually gets hanged up
		LogWarn("SubscribeToIncomingCalls - Timeout.");
		return API_TIMEOUT;
	}

	if (IW_FAILURE(res))
	{
		LogWarn("SubscribeToIncomingCalls - failure res:" << res);
		return API_FAILURE;
	}

	switch (response->message_id)
	{
	case MSG_CALL_SUBSCRIBE_ACK:
		{
			res = API_SUCCESS;
			break;
		}
	default:
		{
			res = API_SERVER_FAILURE;
		}
	}


	return res; 

}

const string&
CallSession::Dnis()
{
	return _dnis;
}

const string&
CallSession::Ani()
{
	return _ani;
}

void 
CallSession::OfferType(const string &offer_type)
{
	_offerType = offer_type;
}


void
CallSession::ResetState(CallState state, const char *state_str)
{
	LogDebug("CallSession::ResetState iwh:" << _iwCallHandle << ", transition from state:" << _callState << " to state:" << state << ", " << state_str);
	_callState = state;
}

ApiErrorCode
CallSession::MakeCall(IN const std::string &destination_uri, 
				 IN const std::string &offer,
				 IN csp::Time ring_timeout)
{
	FUNCTRACKER;

	LogDebug("CallSession::MakeCall dst:" << destination_uri <<", offer:\n" << offer);

	if (_callState != CALL_STATE_UNKNOWN)
	{
		return API_FAILURE;
	}

	CALL_RESET_STATE(CALL_STATE_OFFERING);


	IwMessagePtr response = NULL_MSG;

	MsgMakeCallReq *msg = new MsgMakeCallReq();
	msg->destination_uri = destination_uri;
	msg->offer = offer;
	msg->offer_type = _offerType;
	msg->call_handler_inbound = _handlerPair.inbound;
	msg->stack_call_handle = _iwCallHandle;

	_localOffer = offer;


	// wait for ok or nack
	ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		_stackHandleId,
		IwMessagePtr(msg),
		response,
		ring_timeout,
		"CallSession::MakeCall TXN");


	if (res == API_TIMEOUT)
	{
		// just to be sure that timeout-ed call
		// eventually gets hanged up
		LogWarn("CallSession::MakeCall - Timeout.");
		HangupCall();
		return API_TIMEOUT;
	}

	if (IW_FAILURE(res))
	{
		LogWarn("CallSession::MakeCall - failure res:" << res);
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

	_iwCallHandle = make_call_ok->stack_call_handle;
	_dnis = destination_uri;
	_ani  = make_call_ok->ani;

	CALL_RESET_STATE(CALL_STATE_CONNECTED);

	return res;

}

CallSession::~CallSession(void)
{
 FUNCTRACKER;

 StopActiveObjectLwProc();

 HangupCall();


}

 void 
CallSession::UponDtmfEvt(IwMessagePtr ptr)
 {
	 FUNCTRACKER;

	 // just proxy the event
	 _dtmfChannel->Send(ptr);

 }

 void 
	 CallSession::CleanDtmfBuffer()
 {
	 FUNCTRACKER;

	 // the trick we are using
	 // to clean the buffer is just replace the handle
	 // with  new one
	 _dtmfChannel->Poison();
	 _dtmfChannel = LpHandlePtr(new LpHandle());

	 LogDebug("CallSession::CleanDtmfBuffer iwh:" << _iwCallHandle);

 }


 ApiErrorCode 
CallSession::WaitForDtmf(OUT string &signal, IN const Time timeout)
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

	 LogDebug("CallSession::WaitForDtmf iwh:" << _iwCallHandle << " received signal:" << signal);

	 return API_SUCCESS;


 }



 void 
CallSession::UponActiveObjectEvent(IwMessagePtr ptr)
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
	 CallSession::UponCallTerminated(IwMessagePtr ptr)
 {
	 _hangupChannel->Send(new MsgCallHangupEvt());
	 CALL_RESET_STATE(CALL_STATE_TERMINATED);
 }


ApiErrorCode
CallSession::RejectCall()
 {
	 FUNCTRACKER;

	 LogDebug("CallSession::RejectCall iwh:" << _iwCallHandle );

	 MsgCallOfferedNack *msg = new MsgCallOfferedNack();
	 msg->stack_call_handle = _iwCallHandle;

	 ApiErrorCode res = GetCurrLightWeightProc()->SendMessage(_stackHandleId,msg);
	 return res;

 }

 ApiErrorCode
CallSession::HangupCall()
{

 FUNCTRACKER;

 LogDebug("CallSession::HangupCall iwh:" << _iwCallHandle);

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

const string &
CallSession::OfferType()
{
	return _offerType;

}

CallState
CallSession::CurrentCallState() const
{
	return _callState;
}


MediaCallSession::~MediaCallSession()
{

}

void
MediaCallSession::EnableMediaFormat(IN const MediaFormat& media_format)
{
	FUNCTRACKER;

	LogDebug("MediaCallSession::EnableMediaFormat - Media format:" << media_format << ", enabled for iwh:" << _iwCallHandle );

	_supportedMediaFormatsMap.insert(
		MediaFormatMapPair(media_format.sdp_mapping(), media_format));
}



ApiErrorCode
MediaCallSession::NegotiateMediaFormats(IN const MediaFormatsList &offered_medias, 
							   OUT MediaFormatsList &accepted_media,
							   OUT MediaFormat &accepted_speech_format)
{
	FUNCTRACKER;

	for (MediaFormatsList::const_iterator i = offered_medias.begin();
		i != offered_medias.end();
		++i)
	{
		LogDebug("MediaCallSession::EnableMediaFormat - offered:" << (*i).sdp_name_tos() << ", iwh:" << _iwCallHandle);
	}

	for (MediaFormatsMap::const_iterator i = _supportedMediaFormatsMap.begin();
		i != _supportedMediaFormatsMap.end();
		++i)
	{
		LogDebug("MediaCallSession::EnableMediaFormat enabled:" << (*i).second.sdp_name_tos() << ", iwh:" << _iwCallHandle);
	}

	if (_supportedMediaFormatsMap.empty() || offered_medias.empty())
	{
		LogWarn("MediaCallSession::EnableMediaFormat - Illegal parameters (empty?) for codecs negotiation.");
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
		LogWarn("MediaCallSession::EnableMediaFormat - negotiation failure iwh:" << _iwCallHandle );
		return API_FAILURE;
	}

	LogDebug("MediaCallSession::EnableMediaFormat speech:" << accepted_speech_format << ", rc2833 dtmf chosen:" << dtmf_chosen << ", iwh:" << _iwCallHandle);

	return API_SUCCESS;

}


MediaCallSession::MediaCallSession(IN ScopedForking &forking, 
		   IN HandleId stack_handle_id)
:CallSession(forking, stack_handle_id),
_useGenericOfferAnswer(FALSE)
{
	FUNCTRACKER;
}

MediaCallSession::MediaCallSession(IN ScopedForking &forking,
		   IN HandleId stack_handle_id,
		   IN shared_ptr<MsgCallOfferedReq> offered_msg):
CallSession(forking,stack_handle_id, offered_msg),
_remoteMedia(offered_msg->remote_media),
_useGenericOfferAnswer(FALSE)
{
	FUNCTRACKER;
}

ApiErrorCode
MediaCallSession::AcceptInitialOffer( IN const CnxInfo &local_connection, 
							IN const MediaFormatsList &accepted_media_formats_list,
							IN const MediaFormat &speech_codec)
{

	FUNCTRACKER;

	LogDebug("MediaCallSession::AcceptInitialOffer iwh:" << _iwCallHandle);

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
		"Accept CallSession TXN");

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
CallSession::StackCallHandle() const 
{ 
	return _iwCallHandle; 
}

void
CallSession::WaitTillHangup()
{
	FUNCTRACKER;

	LogDebug("CallSession::WaitTillHangup iwh:" << _iwCallHandle);

	ApiErrorCode res = API_TIMEOUT;
	IwMessagePtr response;
	while (res == API_TIMEOUT)
	{
		res = GetCurrLightWeightProc()->WaitForTxnResponse(_hangupChannel,response,Seconds(100));
	}

}

ApiErrorCode
CallSession::BlindXfer(IN const string &destination_uri)
{
	FUNCTRACKER;

	LogDebug("CallSession::BlindXfer iwh:" << _iwCallHandle << " dest:" << destination_uri);

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
		"Blind Xfer CallSession TXN");

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
MediaCallSession::MakeCall(IN const string &destination_uri, 
				  IN const CnxInfo &local_media,
				  IN csp::Time ring_timeout)
{
	FUNCTRACKER;

	LogDebug("MediaCallSession::MakeCall dst:" << destination_uri <<", lci:" << local_media);

	if (_useGenericOfferAnswer)
	{
		LogWarn("MediaCallSession::MakeCall - cannot use this function in generic offer/answer mode");
		return API_FEATURE_DISABLED;
	}

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
		"Make CallSession TXN");


	if (res == API_TIMEOUT)
	{
		// just to be sure that timeout-ed call
		// eventually gets hanged up
		LogWarn("CallSession::MakeCall - Timeout.");
		HangupCall();
		return API_TIMEOUT;
	}

	if (IW_FAILURE(res))
	{
		LogWarn("CallSession::MakeCall - failure res:" << res);
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
MediaCallSession::RemoteMedia() const 
{ 
	return _remoteMedia; 
}

void 
MediaCallSession::RemoteMedia(CnxInfo &val) 
{ 
	_remoteMedia = val; 
}


CnxInfo 
MediaCallSession::LocalMedia() const 
{ 
	return _localMedia; 
}

void 
MediaCallSession::LocalMedia(CnxInfo &val) 
{ 
	_localMedia = val; 
}



const MediaFormat& 
MediaCallSession::AcceptedSpeechCodec()
{
	return _acceptedSpeechFormat;
}

void 
MediaCallSession::OfferType(const string &offer_type)
{
	_useGenericOfferAnswer = TRUE;
	CallSession::OfferType(offer_type);

}

}