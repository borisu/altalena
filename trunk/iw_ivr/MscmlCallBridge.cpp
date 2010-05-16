#include "StdAfx.h"
#include "MscmlCallBridge.h"
#include "BridgeMacros.h"

namespace ivrworx
{
	const char MscmlCallBridge::className[] = "MscmlCallBridge";
	Luna<MscmlCallBridge>::RegType MscmlCallBridge::methods[] = {
		method(MscmlCallBridge, setup),
		method(MscmlCallBridge, hangup),
		method(MscmlCallBridge, sendrawrequest),
		{0,0}
	};

	MscmlCallBridge::MscmlCallBridge(lua_State *L)
	{
	}

	MscmlCallBridge::MscmlCallBridge(MscmlCallPtr call_ptr)
	{
		_call = call_ptr;
	}

	MscmlCallBridge::~MscmlCallBridge(void)
	{
	}

	int
	MscmlCallBridge::hangup(lua_State *L)
	{
		return API_SUCCESS;
	}

	int
	MscmlCallBridge::setup(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LUA_STRING_PARAM(L,dest,-2);
		LUA_STRING_PARAM(L,body,-1);
	

		ApiErrorCode res = ((CallSession &)(*_call)).MakeCall(string(dest),string(body),Seconds(15));
		lua_pushnumber (L, res);

		return 1;
	}

	int
	MscmlCallBridge::sendrawrequest(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LUA_STRING_PARAM(L,body,-1);
		
	
		ApiErrorCode res = _call->SendInfo(body,"mediaservercontrol+xml");
		lua_pushnumber (L, res);

		return 1;
	}


}


