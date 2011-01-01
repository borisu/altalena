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


namespace ivrworx
{
	const char RTPProxyBridge::className[] = "RTPProxyBridge";
	Luna<RTPProxyBridge>::RegType RTPProxyBridge::methods[] = {
		method(RTPProxyBridge, allocate),
		method(RTPProxyBridge, teardown),
		method(RTPProxyBridge, localcnx),
		{0,0}
	};

	RTPProxyBridge::RTPProxyBridge(lua_State *L)
	{
	}

	RTPProxyBridge::RTPProxyBridge(RtpProxySessionPtr rtpProxySession):
	_rtpProxySession(rtpProxySession)
	{
	}

	RTPProxyBridge::~RTPProxyBridge(void)
	{
		



	}

	// Lua interface
	int 
	RTPProxyBridge::allocate(lua_State *L)
	{
		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		};

		ApiErrorCode res  = _rtpProxySession->Allocate();
		lua_pushnumber (L, res);

		return 1;
	}
	
	int 
	RTPProxyBridge::teardown(lua_State *L)
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

	int 
	RTPProxyBridge::modify(lua_State *L)
	{
// 		if (!_rtpProxySession)
// 		{
// 			lua_pushnumber (L, API_WRONG_STATE);
// 			return 1;
// 		};
// 
// 		CnxInfoBridge *a = 
// 			static_cast<CnxInfoBridge*>(lua_touserdata(L, -2));
// 		if (a == NULL)
// 		{
// 			lua_pushnumber (L, API_WRONG_PARAMETER);
// 			return 1;
// 		};
// 
// 		MediaFormatBridge *media_format = 
// 			static_cast<MediaFormatBridge*>(lua_touserdata(L, -1));
// 		if (a == NULL)
// 		{
// 			lua_pushnumber (L, API_WRONG_PARAMETER);
// 			return 1;
// 		};
// 
// 
// 		throw;

		
// 		ApiErrorCode res  = _rtpProxySession->Modify(a->_cnxInfo, media_format->_mediaFormat);
// 		lua_pushnumber (L, res);

		return 1;

	}

	int
	RTPProxyBridge::localcnx(lua_State *L)
	{
		FUNCTRACKER;

		if (!_rtpProxySession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		throw;

// 		Luna<CnxInfoBridge>::PushObject(L, new CnxInfoBridge(_rtpProxySession->LocalCnxInfo()));
// 		return 1;

	}




}

