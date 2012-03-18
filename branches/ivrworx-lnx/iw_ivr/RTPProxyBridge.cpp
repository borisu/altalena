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
#include "RTPProxyBridge.h"
#include "BridgeMacros.h"


namespace ivrworx
{
	const char rtpproxy::className[] = "rtpproxy";
	Luna<rtpproxy>::RegType rtpproxy::methods[] = {
		method(rtpproxy, allocate),
		method(rtpproxy, teardown),
		method(rtpproxy, modify),
		method(rtpproxy, bridge),
		method(rtpproxy, localoffer),
		method(rtpproxy, remoteoffer),
		method(rtpproxy, waitfordtmf),
		method(rtpproxy, cleandtmfbuffer),
		method(rtpproxy, dtmfbuffer),
		{0,0}
	};

	rtpproxy::rtpproxy(lua_State *L)
	{
	}

	rtpproxy::rtpproxy(RtpProxySessionPtr rtpProxySession):
	_rtpProxySession(rtpProxySession)
	{
	}

	rtpproxy::~rtpproxy(void)
	{
		
	}

	int
	rtpproxy::cleandtmfbuffer(lua_State *L)
	{
		FUNCTRACKER;

		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		_rtpProxySession->CleanDtmfBuffer();
		lua_pushnumber (L, API_SUCCESS);

		return 1;

	}

	int
	rtpproxy::dtmfbuffer(lua_State *L)
	{
		FUNCTRACKER;

		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		lua_pushstring(L, _rtpProxySession->DtmfBuffer().c_str());
		return 1;
	}


	int
	rtpproxy::waitfordtmf(lua_State *L)
	{
		FUNCTRACKER;

		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		int timeout = 3;
		if (GetTableNumberParam(L,-1,&timeout,"timeout",3000))
		{
			timeout *= 1000;
		}

		ApiErrorCode res = API_SUCCESS;

		string signal;

		res = 	_rtpProxySession->WaitForDtmf(signal, MilliSeconds(timeout));

		lua_pushnumber(L, res);
		lua_pushstring(L, signal.c_str());

		return 2;

	}

	// Lua interface
	int 
	rtpproxy::allocate(lua_State *L)
	{
		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		};

		AbstractOffer offer;
		offer.type = "application/sdp";
		BOOL paramres = GetTableStringParam(L,-1,offer.body,"sdp");
		if (paramres == FALSE)
		{
			lua_pushnumber (L, API_WRONG_PARAMETER);
			return 1;
		}

		ApiErrorCode res  = _rtpProxySession->Allocate(offer);
		lua_pushnumber (L, res);

		return 1;
	}

	int 
	rtpproxy::bridge(lua_State *L)
	{
		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		};

		rtpproxy *other = NULL;
		BOOL paramres = GetLunaUserData<rtpproxy>(L,-1,&other,"other");
		if (paramres == FALSE)
		{
			lua_pushnumber (L, API_WRONG_PARAMETER);
			return 1;
		}

		string duplex;
		GetTableStringParam(L,-1,duplex, "duplex","half");
	
		ApiErrorCode res  = _rtpProxySession->Bridge(*(other->_rtpProxySession),(duplex == "full"));
		lua_pushnumber (L, res);

		return 1;
	}
	
	int 
	rtpproxy::teardown(lua_State *L)
	{
		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		};

		ApiErrorCode res  = _rtpProxySession->TearDown();
		lua_pushnumber (L, res);

		return 1;

	}

	ActiveObjectPtr 
	rtpproxy::get_active_object()
	{

		return dynamic_pointer_cast <ActiveObject>(_rtpProxySession);

	}


	int 
	rtpproxy::modify(lua_State *L)
	{

		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		};

		AbstractOffer offer;
		offer.type = "application/sdp";
		BOOL paramres = GetTableStringParam(L,-1,offer.body,"sdp");
		if (paramres == FALSE)
		{
			lua_pushnumber (L, API_WRONG_PARAMETER);
			return 1;
		}


		ApiErrorCode res  = _rtpProxySession->Modify(offer);
		lua_pushnumber (L, res);

		return 1;

	}

	int
	rtpproxy::remoteoffer(lua_State *L)
	{
		FUNCTRACKER;
		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		lua_pushstring(L, _rtpProxySession->RemoteOffer().body.c_str());
		return 1;

	}

	int 
	rtpproxy::localoffer(lua_State *L)
	{
		FUNCTRACKER;
		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		lua_pushstring(L, _rtpProxySession->LocalOffer().body.c_str());
		return 1;

	}




}

