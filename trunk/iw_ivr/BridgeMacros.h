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


#define LUA_STRING_PARAM(L,N,I) if (lua_isstring(L, I) != 1 ) \
	{ lua_pushnumber (L, API_WRONG_PARAMETER); return 1;}    \
	const char *N = NULL; {	size_t string_length = 0;		 \
	N = lua_tolstring(L, I, &string_length);};		 

#define LUA_INT_PARAM(L,N,I) if (lua_isnumber(L, I) != 1 ) \
	{ lua_pushnumber (L, API_WRONG_PARAMETER); return 1;}  \
	lua_Integer N = lua_tointeger(L, I);	

#define LUA_DOUBLE_PARAM(L,N,I) if (lua_isnumber(L, I) != 1 ) \
	{ lua_pushnumber (L, API_WRONG_PARAMETER); return 1;}  \
	lua_Number N = lua_tonumber(L, I);

#define LUA_BOOL_PARAM(L,N,I) if (lua_isboolean(L, I) != 1 ) \
	{ lua_pushnumber (L, API_WRONG_PARAMETER); return 1;}  \
	BOOL N = lua_toboolean(L, I);	


	
	