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
const char SipCallBridge::className[] = "SipCallBridge";
Luna<SipCallBridge>::RegType SipCallBridge::methods[] = {
	method(SipCallBridge, answer),
	method(SipCallBridge, cleandtmfbuffer),
	method(SipCallBridge, waitfordtmf),
	method(SipCallBridge, waitforhangup),
	method(SipCallBridge, ani),
	method(SipCallBridge, dnis),
	method(SipCallBridge, mediaformat),
	method(SipCallBridge, makecall),
	method(SipCallBridge, blindxfer),
	method(SipCallBridge, hangup),
	method(SipCallBridge, sendinfo),
	{0,0}
};

SipCallBridge::SipCallBridge(lua_State *L)
{

}

SipCallBridge::SipCallBridge(SipMediaCallPtr call):
_call(call)
{

}

SipCallBridge::~SipCallBridge(void)
{
	_call.reset();
}


int
SipCallBridge::hangup(lua_State *L)
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
SipCallBridge::makecall(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	LUA_STRING_PARAM(L,dest,-2);
	LUA_STRING_PARAM(L,sdp,-1);
	

	_call->OfferType("sdp");

	ApiErrorCode res = 
		(shared_polymorphic_cast<CallSession>(_call))->MakeCall(dest,sdp,Seconds(15));

	lua_pushnumber (L, res);

	return 1;
}

int
SipCallBridge::blindxfer(lua_State *L)
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
SipCallBridge::cleandtmfbuffer(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	LogDebug("SipCallBridge::cleandtmfbuffer iwh:" << _call->StackCallHandle());

	_call->CleanDtmfBuffer();

	lua_pushnumber (L, API_SUCCESS);
	return 1;

}

int
SipCallBridge::waitfordtmf(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}


	LUA_INT_PARAM(L,timeout,-1);

	LogDebug("SipCallBridge::waitfordtmf iwh:" << _call->StackCallHandle() << ", timeout:" << timeout);

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
SipCallBridge::ani(lua_State *L)
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
SipCallBridge::mediaformat(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	lua_pushstring(L,_call->AcceptedSpeechCodec().sdp_name_tos().c_str());

	return 1;

}

int
SipCallBridge::dnis(lua_State *L)
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
SipCallBridge::waitforhangup(lua_State *L)
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
SipCallBridge::answer(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	LogDebug("SipCallBridge::answer  iwh:" << _call->StackCallHandle());

	ApiErrorCode res = API_SERVER_FAILURE; // _call->AcceptInitialOffer();
	lua_pushnumber (L, res);

	return 1;
}


int
SipCallBridge::sendinfo(lua_State *L)
{
	FUNCTRACKER;

	if (!_call)
	{
		lua_pushnumber (L, API_WRONG_STATE);
		return 1;
	}

	LUA_STRING_PARAM(L,body,-2);
	LUA_STRING_PARAM(L,type,-1);


	ApiErrorCode res = _call->SendInfo(body,type);
	lua_pushnumber (L, res);

	return 1;

}


}


