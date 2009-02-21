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
#include "CcuLogger.h"


namespace ivrworx
{



Call::Call(IN LpHandlePair stack_pair):
_stackPair(stack_pair),
_stackCallHandle(IX_UNDEFINED),
_hangupDetected(FALSE),
_handlerPair(HANDLE_PAIR)
{
	_callState = IX_CALL_NONE;
	throw;

}

Call::Call(
	 IN LpHandlePair stack_pair,
	 IN ScopedForking &forking,
	 IN shared_ptr<CcuMsgCallOfferedReq> offered_msg):
 _stackPair(stack_pair),
 _stackCallHandle(offered_msg->stack_call_handle),
 _remoteMedia(offered_msg->remote_media),
 _handlerPair(offered_msg->call_handler_inbound)
{
	FUNCTRACKER;

	_callState = IX_CALL_OFFERED;

	Start(forking,_handlerPair,__FUNCTIONW__);

	LogDebug("Creating call session - ix stack handle=[" << _stackCallHandle << "].");

}

void
Call::EnableMediaFormat(const MediaFormat& media_format)
{
	FUNCTRACKER;

	LogDebug("Media format " << media_format << "  enabled for call " << _stackCallHandle );

	_supportedMediaFormatsList.insert(
		MediaFormatMapPair(media_format.sdp_mapping(), media_format));
}

IxApiErrorCode
Call::NegotiateMediaFormats(IN const MediaFormatsList &offered_medias, 
					  OUT MediaFormatsList &accepted_media)
{
	FUNCTRACKER;

	IX_PROFILE_FUNCTION();

	if (_supportedMediaFormatsList.empty() || offered_medias.empty())
	{
		LogWarn("Illegal parameters (empty?) for codecs negotiation.");
		return CCU_API_FAILURE;
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
					_acceptedSpeechFormat = media_format2;
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
		return CCU_API_FAILURE;
	}

	return CCU_API_SUCCESS;

}

Call::~Call(void)
{
	HagupCall();
}

void Call::UponActiveObjectEvent(IxMsgPtr ptr)
{
	switch (ptr->message_id)
	{
	case CCU_MSG_CALL_HANG_UP_EVT:
		{
			_callState = IX_CALL_TERMINATED;
		}
	default:
		{

		}
	}

	ActiveObject::UponActiveObjectEvent(ptr);

}

IxApiErrorCode
Call::WaitForDtmf(IN wstring &dtmf_digit, IN Time timeout)
{
	IxApiErrorCode res = CCU_API_SUCCESS;
	IxMsgPtr ptr = _dtmfChannel.Wait(timeout,res);

	if (CCU_FAILURE(res))
	{
		return res;
	}

	shared_ptr<CcuMsgCallDtmfEvt> dtmfEvt = 
		dynamic_pointer_cast<CcuMsgCallDtmfEvt> (ptr);

	dtmf_digit = dtmfEvt->dtmf_digit;

	return res;


}

IxApiErrorCode
Call::RejectCall()
{
	FUNCTRACKER;

	LogDebug("Rejecting the call - ix stack handle=[" << _stackCallHandle << "].");

	CcuMsgCallOfferedNack *msg = new CcuMsgCallOfferedNack();

	msg->stack_call_handle = _stackCallHandle;

	_stackPair.inbound->Send(msg);

	return CCU_API_SUCCESS;

}

IxApiErrorCode
Call::HagupCall()
{
	
	FUNCTRACKER;

	LogDebug("Hanging up the call - ix stack handle=[" << _stackCallHandle << "].");

	if (_stackCallHandle == IX_UNDEFINED || 
		_callState == IX_CALL_NONE		  || 
		_callState == IX_CALL_TERMINATED)
	{
		return CCU_API_SUCCESS;
	}

	CcuMsgHangupCallReq *msg = new CcuMsgHangupCallReq(_stackCallHandle);

	_stackPair.inbound->Send(msg);

	_stackCallHandle = IX_UNDEFINED;

	_hangupDetected = TRUE;

	return CCU_API_SUCCESS;
}

IxApiErrorCode
Call::AcceptCall(IN const CnxInfo &local_media, 
				 IN const MediaFormatsList &accepted_codec)
{
	
	FUNCTRACKER;
	IX_PROFILE_FUNCTION();

	LogDebug("Accepting call - ix stack handle=[" << _stackCallHandle << "].");

	IxMsgPtr response = CCU_NULL_MSG;
	
	CcuMsgCalOfferedlAck *ack = new CcuMsgCalOfferedlAck();
	ack->stack_call_handle = _stackCallHandle;
	ack->local_media = local_media;
	ack->accepted_codecs = accepted_codec;

	
	IxApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		_stackPair.inbound,
		IxMsgPtr(ack),
		response,
		MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
		L"Accept Call TXN");

	if (CCU_FAILURE(res))
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_CALL_CONNECTED:
		{
			shared_ptr<CcuMsgNewCallConnected> make_call_sucess = 
				dynamic_pointer_cast<CcuMsgNewCallConnected>(response);

			_stackCallHandle = make_call_sucess->stack_call_handle;

			break;
		}
	default:
		{
			throw;
		}
	}

	_callState = IX_CALL_CONNECTED;

	return res;
}

int 
Call::StackCallHandle() const 
{ 
	return _stackCallHandle; 
}


IxApiErrorCode
Call::MakeCall(IN wstring destination_uri, 
			   IN CnxInfo local_media)
{
	FUNCTRACKER;

	_localMedia = local_media;

	IxMsgPtr response = CCU_NULL_MSG;

	CcuMsgMakeCallReq *msg = new CcuMsgMakeCallReq();
	msg->local_media = local_media;
	msg->destination_uri = destination_uri;
	msg->call_handler_inbound = _handlerPair.inbound;


	IxApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
		_stackPair.inbound,
		IxMsgPtr(msg),
		response,
		Seconds(60),
		L"Make Call TXN");

	if (res != CCU_API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_MAKE_CALL_ACK:
		{
			shared_ptr<CcuMsgMakeCallAck> make_call_sucess = 
				dynamic_pointer_cast<CcuMsgMakeCallAck>(response);

			_stackCallHandle = make_call_sucess->stack_call_handle;

			_remoteMedia = make_call_sucess->remote_media;

			break;
		}
	case CCU_MSG_MAKE_CALL_NACK:
		{
			res = CCU_API_SERVER_FAILURE;
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
