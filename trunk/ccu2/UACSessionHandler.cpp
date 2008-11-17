#include "StdAfx.h"
#include "UacSessionHandler.h"
#include "UACAppDialogSet.h"
#include "CcuMessage.h"
#include "Ccu.h"

UACSessionHandler::UACSessionHandler(IN CcuMediaData ipAddr)
{
	string sdpStr = "v=0\r\n"
		"o=1900 369696545 369696545 IN IP4  " + ipAddr.iptos() + "\r\n"
		"s=X-Lite\r\n"
		"c=IN IP4 "  + ipAddr.ipporttos() + "\r\n"
		"t=0 0\r\n"
		"m=audio 8000 RTP/AVP 8 3 98 97 101\r\n"
		"a=rtpmap:8 pcma/8000\r\n"
		"a=rtpmap:3 gsm/8000\r\n"
		"a=rtpmap:98 iLBC\r\n"
		"a=rtpmap:97 speex/8000\r\n"
		"a=rtpmap:101 telephone-event/8000\r\n"
		"a=fmtp:101 0-15\r\n";

	_txt = DataPtr(new Data(sdpStr.c_str()));

#pragma warning (push)
#pragma warning (disable:4267)
	_hfv = HeaderFieldValuePtr (new HeaderFieldValue(
		_txt->data(), 
		_txt->size()));
#pragma warning (pop)

	Mime type("application", "sdp");

	_sdp = SdpContentsPtr(new SdpContents(_hfv.get(), type));

}

UACSessionHandler::~UACSessionHandler(void)
{

}

void 
UACSessionHandler::onConnected(ClientInviteSessionHandle is, const SipMessage& msg)
{
// 	FUNCTRACKER;;
// 	LogDebug("ClientInviteSession-onConnected - " << msg.brief());
// 	//LogDebug("Connected now - requestingOffer from UAS" );
// 	//is->requestOffer();
// 	_inviteMap[is.getId()] = is;
// 	UACAppDialogSet *set  =(UACAppDialogSet *)is->getAppDialogSet().get();
// 	
// 	LpHandlePtr handle = set->_process;
// 
//  	CcuMsgMakeCallSuccess *res = new CcuMsgMakeCallSuccess();
//  	res->stack_call_handle = is.getId();
//  
//  	handle->SendToThisProcess(res);

}

void
UACSessionHandler::EndAllSessions()
{
	
	FUNCTRACKER;

	// end invite session
	for (InviteSessionsMap::iterator i = _inviteMap.begin();
		i!=_inviteMap.end();
		 ++i)
	{
		(*i).second->end();
	}

	// end register sessions
	for (RegistrationSessionMap::iterator i = _registrationMap.begin();
		i!=_registrationMap.end();
		++i)
	{
		(*i).second->end();
	}
	
}



void 
UACSessionHandler::onSuccess(ClientRegistrationHandle h, const SipMessage& response)
{
// 	FUNCTRACKER;;
// 	LogDebug(": RegisterSession-onSuccess - " << response.brief());
// 
// 
// 	_registrationMap[h.getId()] = h;
// 	UACAppDialogSet *set  =(UACAppDialogSet *)h->getAppDialogSet().get();
// 
// 	LpHandlePtr handle = set->_process;
// 
// 	CcuMsgRegistered *res = new CcuMsgRegistered();
// 	res->stack_registration_handle = h.getId();
// 
// 	handle->SendToThisProcess(res);
}

void 
UACSessionHandler::onFailure(ClientRegistrationHandle h, const SipMessage& msg)
{
// 	FUNCTRACKER;;
// 	LogDebug(": RegisterSession-onFailure - " << msg.brief());
// 
// 	UACAppDialogSet *set  =(UACAppDialogSet *)h->getAppDialogSet().get();
// 
// 	LpHandlePtr handle = set->_process;
// 
// 	CcuMsgRegistrationFailure *res = new CcuMsgRegistrationFailure();
// 	handle->SendToThisProcess(res);
}

