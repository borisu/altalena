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

#pragma once
#include "LuaVirtualMachine.h"
#include "LuaTable.h"
#include "IvrDllApi.h"

namespace ivrworx
{
	class IW_IVR_API Context : 
		public AppData
	{
	public:
		ConfigurationPtr _conf;

		ScopedForking *_forking;

		ListOfAny _staticObjects;
	};

	IW_IVR_API BOOL InitStaticTypes(lua_State *L, LuaTable &ivrworxTable, const Context *ctx);

	int LuaWait(lua_State *L);

	int LuaRunLongOperation(lua_State *L);

	int LuaFork(lua_State *L);

	int LuaCreateMscml(lua_State *L);

	int LuaCreateSip(lua_State *L);

	int LuaCreateMrcp(lua_State *L);

	int LuaCreateRtpProxy(lua_State *L);

	int LuaCreateStreamer( lua_State *L);
}

