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
#include "BridgeMacros.h"
#include "SipCallBridge.h"
#include "LuaStaticApi.h"


namespace ivrworx
{
const char sipcall::className[] = "sipcall";
Luna<sipcall>::RegType sipcall::methods[] = {
	method(sipcall, answer),
	method(sipcall, cleandtmfbuffer),
	method(sipcall, waitfordtmf),
	method(sipcall, waitforhangup),
	method(sipcall, ani),
	method(sipcall, dnis),
	method(sipcall, makecall),
	method(sipcall, blindxfer),
	method(sipcall, hangup),
	method(sipcall, sendinfo),
	method(sipcall, waitforinfo),
	method(sipcall, remoteoffer),
	method(sipcall, localoffer),
	method(sipcall, accept),
	method(sipcall, startregister),
	method(sipcall, unregister),
	method(sipcall, reoffer),
	method(sipcall, subscribe),
	method(sipcall, unsubscribe),
	{0,0}
};

sipcall::sipcall(lua_State *L)
{

}

sipcall::sipcall(SipMediaCallPtr call):
_call(call)
{

}

sipcall::~sipcall(void)
{
	_call.reset();
}

int
sipcall::accept(lua_State *L)
{
	FUNCTRACKER;

	int timeout = 15;
	BOOL paramres = GetTableNumberParam(L,-1,&timeout,"timeout",15);

	string service;
	paramres = GetTableStringParam(L,-1,service,"service");
	if (paramres == FALSE)
	{
		lua_pushnumber (L, API_WRONG_PARAMETER);
		return 1;
	}

	
	LpHandlePtr service_handle = ivrworx::GetHandle(service);
	if (!service_handle)
	{
		
		lua_pushnumber (L, API_UNKNOWN_DESTINATION);
		return 1;
	}

	DECLARE_NAMED_HANDLE(listener_handle);
	if (IW_FAILURE(SubscribeToIncomingCalls(service_handle,listener_handle)))
	{
		lua_pushnumber (L, API_FAILURE);
		return 1;
	}

	
	//
	// Message Loop
	// 
	ApiErrorCode res = API_SUCCESS;
	IwMessagePtr msg = listener_handle->Wait(Seconds(timeout),res);

	if (IW_FAILURE(res))
		return res;

	switch (msg->message_id)
	{
	case MSG_CALL_OFFERED:
		{

			shared_ptr<MsgCallOfferedReq> call_offered = 
				shared_polymorphic_cast<MsgCallOfferedReq> (msg);

			_call.reset(new SipMediaCall(*CTX_FIELD(_forking),call_offered));
			break;
		}
	default:
		{
			lua_pushnumber (L, API_UNKNOWN_RESPONSE);
			return 1;
		}
	}


	lua_pushnumber (L, API_SUCCESS);
	return 1;


}


int
sipcall::hangup(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	ApiErrorCode res = _call->HangupCall();
	lua_pushnumber (L, res);

	return 1;
}

int
sipcall::remoteoffer(lua_State *L)
{
	FUNCTRACKER;
	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	lua_pushstring(L, _call->RemoteOffer().body.c_str());
	return 1;
}

int
sipcall::localoffer(lua_State *L)
{
	FUNCTRACKER;
	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	lua_pushstring(L, _call->LocalOffer().body.c_str());
	return 1;
}
int
sipcall::unsubscribe(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}
	return 0;
}

int
sipcall::subscribe(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	string eventserver;
	string realm;
	string username;
	string password;
	string eventpackage;
	list<string> contactslist;
	int timeout = 15;
	int refresh = 160;
	int subscription_interval = 380;
	AbstractOffer offer;

	BOOL paramres = TRUE;
	GetTableStringParam(L,-1,eventserver,"server");
	GetTableStringParam(L,-1,eventpackage,"package");
	GetTableStringParam(L,-1,username,"user");
	GetTableStringParam(L,-1,realm,"realm");
	GetTableStringParam(L,-1,password,"password");

	Credentials cred;
	cred.username = username;
	cred.realm = realm;
	cred.password = password;

	GetTableNumberParam(L,-1,&timeout,"timeout",15);
	GetTableNumberParam(L,-1,&subscription_interval,"interval",160);
	GetTableNumberParam(L,-1,&refresh,"refresh",380);

	
	GetTableStringParam(L,-1,offer.body,"offer");
	GetTableStringParam(L,-1,offer.type,"type");

	

	ApiErrorCode res = 
		_call->Subscribe(
			eventserver,
			contactslist,
			cred, 
			offer, 
			eventpackage, 
			refresh, 
			subscription_interval, 
			Seconds(timeout));


	lua_pushnumber (L, res);
	return 1;


// error_param:
// 	lua_pushnumber (L, API_WRONG_PARAMETER);
// 	return 1;

}

int
sipcall::startregister(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	string registrar;
	string realm;
	string username;
	string password;
	string contacts;
	list<string> contactslist;
	vector<string> strs;
	Credentials cred;
	
	BOOL paramres = GetTableStringParam(L,-1,registrar,"registrar");
	if (paramres == FALSE)
		goto error_param;

	paramres = GetTableStringParam(L,-1,username,"username");
	if (paramres == FALSE)
		goto error_param;

	
	paramres = GetTableStringParam(L,-1,contacts,"contacts");
	if (paramres != FALSE)
	{
		split(strs, contacts, is_any_of(","));

		contactslist.resize(strs.size());
		std::copy(strs.begin(), strs.end(), contactslist.begin());
	}


	paramres = GetTableStringParam(L,-1,realm,"realm");
	if (paramres == FALSE)
		goto error_param;

	GetTableStringParam(L,-1,password,"password");

	int timeout = 15;
	GetTableNumberParam(L,-1,&timeout,"timeout",15);


	cred.username = username;
	cred.realm = realm;
	cred.password = password;
	

	ApiErrorCode res = 
		_call->StartRegistration(contactslist,registrar,cred,Seconds(timeout));


	lua_pushnumber (L, res);
	return 1;

error_param:
	lua_pushnumber (L, API_WRONG_PARAMETER);
	return 1;
}

int
sipcall::unregister(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	ApiErrorCode res = 
		_call->StopRegistration();


	lua_pushnumber (L, res);
	return 1;

}

int
sipcall::reoffer(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	MapOfAny freemap;
	AbstractOffer offer;
	string dest;


	int timeout = 15;
	GetTableNumberParam(L,-1,&timeout,"timeout",15);


	BOOL paramres = GetTableStringParam(L,-1,offer.body,"sdp");
	if (paramres == FALSE)
	{
		GetTableStringParam(L,-1,offer.body,"offer");
		GetTableStringParam(L,-1,offer.type,"type");
	}
	else
	{
		offer.type = "sdp";
	}

	FillTable(L,-1,freemap);

	ApiErrorCode res = 
		_call->ReOffer(offer,freemap,Seconds(timeout));


	lua_pushnumber (L, res);
	return 1;

}

int
sipcall::makecall(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	MapOfAny freemap;
	AbstractOffer offer;
	string dest;
	string username;
	string realm;
	string password;


	GetTableStringParam(L,-1,username,"username");
	GetTableStringParam(L,-1,realm,"realm");
	GetTableStringParam(L,-1,password,"password");

	Credentials cred;
	cred.username	= username;
	cred.realm		= realm;
	cred.password	= password;
	

	BOOL paramres = GetTableStringParam(L,-1,dest,"dest");
	if (paramres == FALSE)
		goto error_param;
	

	int timeout = 15;
	GetTableNumberParam(L,-1,&timeout,"timeout",15);

	
	paramres = GetTableStringParam(L,-1,offer.body,"sdp");
	if (paramres == FALSE)
	{
		GetTableStringParam(L,-1,offer.body,"offer");
		GetTableStringParam(L,-1,offer.type,"type");
	}
	else
	{
		offer.type = "sdp";
	}

	FillTable(L,-1,freemap);

	ApiErrorCode res = 
		_call->MakeCall(dest,offer,cred,freemap,Seconds(timeout));


	lua_pushnumber (L, res);
	return 1;
	

error_param:
	lua_pushnumber (L, API_WRONG_PARAMETER);
	return 1;

}

int
sipcall::blindxfer(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	string dest;
	BOOL paramres = GetTableStringParam(L,-1,dest,"dest");
	if (paramres == FALSE)
	{
		lua_pushnumber (L, API_WRONG_PARAMETER);
		return 1;
	}

	ApiErrorCode res = _call->BlindXfer(dest);
	lua_pushnumber (L, res);

	return 1;
}




int
sipcall::cleandtmfbuffer(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	LogDebug("sipcall::cleandtmfbuffer iwh:" << _call->StackCallHandle());

	_call->CleanDtmfBuffer();

	lua_pushnumber (L, API_SUCCESS);
	return 1;

}

int
sipcall::waitfordtmf(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}


	int timeout = 3;
	if (GetTableNumberParam(L,-1,&timeout,"timeout",3000))
	{
		timeout *= 1000;
	}

	

	LogDebug("sipcall::waitfordtmf iwh:" << _call->StackCallHandle() << ", timeout:" << timeout);

	ApiErrorCode res = API_SUCCESS;
	
	string signal;
	
	res = 	_call->WaitForDtmf(signal, MilliSeconds(timeout));
	
	lua_pushnumber(L, res);
	lua_pushstring(L, signal.c_str());
	
	return 2;

}


int
sipcall::ani(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	lua_pushstring(L,_call->Ani().c_str());

	return 1;

}



int
sipcall::dnis(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	lua_pushstring(L,_call->Dnis().c_str());

	return 1;
}

int
sipcall::waitforinfo(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	AbstractOffer remote_offer;
	ApiErrorCode res = _call->WaitForInfo(remote_offer);

	lua_pushnumber (L, res);
	lua_pushstring(L, remote_offer.body.c_str());
	return 2;
}

int
sipcall::waitforhangup(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	_call->WaitTillHangup();

	lua_pushnumber (L, API_SUCCESS);
	return 1;
}

int
sipcall::answer(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	LogDebug("sipcall::answer  iwh:" << _call->StackCallHandle());

	MapOfAny params;
	AbstractOffer offer;
	BOOL paramres = GetTableStringParam(L,-1,offer.body,"sdp");
	if (paramres == FALSE)
	{
		GetTableStringParam(L,-1,offer.body,"offer");
		GetTableStringParam(L,-1,offer.type,"type");
	}
	else
	{
		offer.type = "sdp";
	}

	if (offer.body.empty())
		goto error_param;


	int timeout = 15;
	paramres = GetTableNumberParam(L,-1,&timeout,"timeout",15);

	
	ApiErrorCode res = API_SUCCESS;  
	res = _call->Answer(offer, params,Seconds(timeout));

	lua_pushnumber (L, res);
	return 1;

error_param:
	LogWarn("sipcall::answer - API_WRONG_PARAMETER");
	lua_pushnumber (L, API_WRONG_PARAMETER);
	return 1;
}


int
sipcall::sendinfo(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	AbstractOffer offer;
	GetTableStringParam(L,-1,offer.body,"offer");
	GetTableStringParam(L,-1,offer.type,"type");

	bool async = false;
	GetTableBoolParam(L,-1,&async,"async");

	


 	AbstractOffer remote_offer;
	ApiErrorCode res = _call->SendInfo(offer,remote_offer,async);
 	lua_pushnumber (L, res);
	lua_pushstring(L, remote_offer.body.c_str());
 
 	return 2;

}

}



