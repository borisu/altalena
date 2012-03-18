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
#include "LuaUtils.h"
#include "Logger.h"

using namespace ivrworx;

namespace ivrworx
{


LuaUtils::LuaUtils(void)
{
}

LuaUtils::~LuaUtils(void)
{
}

static int buf_writer( lua_State *L, const void* b, size_t n, void* B ) 
{
	(void)L;
	luaL_addlstring((luaL_Buffer*) B, (const char *)b, n);
	return 0;
}


ApiErrorCode 
LuaUtils::Precompile(const string &filename, char **buffer, size_t *size)
{
	lua_State* L=lua_open();
	if (L==NULL) 
	{
		LogWarn("LuaUtils::Precompile - not enough memory for state");
		return API_FAILURE;
	};

	
	// compile file the precompiled chunk is stored
	// as cclosure on top of the stack
	if (luaL_loadfile(L,filename.c_str())!=0) 
	{
		goto error;
	}
 
	// dump cclosure file which is now
	// on the top of the stack
	luaL_Buffer b;
	luaL_buffinit(L,&b);
	if (lua_dump(L, buf_writer, &b) != 0)
	{
		goto error;
	}

	luaL_pushresult(&b);    // pushes dumped string
	size_t sz = 0;
	const char *s= lua_tolstring(L,-1,&sz);

	
	*buffer = (char *)::malloc(sz+1);
	::memcpy(*buffer,s,sz+1);
	*size = sz;

	lua_pop(L,1);
	lua_close(L);
	return API_SUCCESS;
	
error:

	LogWarn("LuaUtils::Precompile - " << lua_tostring(L,-1));
	lua_close(L);
	return API_FAILURE;
}

}