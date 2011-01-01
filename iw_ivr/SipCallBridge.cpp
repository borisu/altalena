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
	

	BOOL paramres = GetTableStringParam(L,-1,dest,"dest");
	if (paramres == FALSE)
		goto error_param;
	

	int timeout = 15;
	paramres = GetTableNumberParam(L,-1,&timeout,"timeout",15);
	if (paramres == FALSE)
		goto error_param;
	

	
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
		_call->MakeCall(dest,offer,freemap,Seconds(timeout));


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

	LUA_STRING_PARAM(L,dest,-1);

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


	LUA_INT_PARAM(L,timeout,-1);

	LogDebug("sipcall::waitfordtmf iwh:" << _call->StackCallHandle() << ", timeout:" << timeout);

	string signal;
#pragma TODO ("CSP++ Does not handle correctly the large timeouts")
#pragma warning (suppress:4244)
	ApiErrorCode res = 	_call->WaitForDtmf(signal, MilliSeconds(timeout));

	lua_pushnumber (L, res);
	if (IW_SUCCESS(res))
	{
		lua_pushstring(L, signal.c_str());
	} 
	else
	{
		lua_pushnil(L);
	}

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

	ApiErrorCode res = _call->WaitForInfo();

	lua_pushnumber (L, res);
	return 1;
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

	LUA_STRING_PARAM(L,type,-2);
	LUA_STRING_PARAM(L,body,-1);

	throw;


// 	ApiErrorCode res = _call->SendInfo(body,type);
// 	lua_pushnumber (L, res);
// 
// 	return 1;

}

}



