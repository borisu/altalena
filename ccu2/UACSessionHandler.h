#pragma once
#include "LpHandle.h"
#include "ccu.h"
#include "ResipCommon.h"
#include "SipSessionHandlerAdapter.h"

using namespace std;
using namespace resip;

typedef map<Handled::Id,ClientInviteSessionHandle> InviteSessionsMap;
typedef map<Handled::Id,ClientRegistrationHandle>  RegistrationSessionMap; 


class UACSessionHandler :
	public SipSessionHandlerAdapter
{
	InviteSessionsMap _inviteMap;

	RegistrationSessionMap _registrationMap;

public:
	UACSessionHandler(CcuMediaData data);

	virtual ~UACSessionHandler(void);

	virtual void EndAllSessions();

	virtual void onSuccess(ClientRegistrationHandle h, const SipMessage& response);

	virtual void onFailure(ClientRegistrationHandle, const SipMessage& msg);
	
	virtual void onConnected(ClientInviteSessionHandle, const SipMessage& msg);

public:

	bool done;

	SdpContentsPtr _sdp;     

	HeaderFieldValuePtr _hfv;      

	DataPtr _txt;      

	LpHandlePtr _process;

};
