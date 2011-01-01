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

#include "LuaRestoreStack.h"

namespace ivrworx
{



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



template<typename T>
BOOL GetLunaUserData(lua_State *L, int tableIndex, T **value, char *name)
{
	CLuaRestoreStack l(L);

	if (name == NULL || !lua_istable(L,tableIndex))
	{
		return FALSE;
	}

	lua_pushstring(L, name);
	lua_gettable(L, tableIndex - 1);

	if (lua_isuserdata(L,-1) == FALSE)
	{
		return FALSE;
	}

	{
		CLuaRestoreStack l(L);
		lua_getmetatable (L,-1);
		lua_pushstring(L,"className");
		lua_gettable(L,-2);

		if (!lua_isstring(L,-1) || strcmp(T::className,lua_tostring(L,-1)) != 0)
				return FALSE;

	}
	
	

	Luna<T>::userdataType *ut_mf = static_cast<Luna<T>::userdataType*>(lua_touserdata(L,-1));

	*value = ut_mf->pT;
	return TRUE;


}

template<typename T>
BOOL GetTableEnumParam(lua_State *L, int tableIndex, T *value, char *name, T defaultValue = 0)
{
	CLuaRestoreStack l(L);

	if (name == NULL || !lua_istable(L,tableIndex))
	{
		*value = defaultValue;
		return FALSE;
	}

	lua_pushstring(L, name);
	lua_gettable(L, tableIndex - 1);

	if (lua_isnumber(L,-1) == FALSE)
	{
		*value = defaultValue;
		return FALSE;
	}

	*value = static_cast<T>((int)(lua_tonumber(L,-1)));
	return TRUE;

};

template<typename T>
BOOL GetTableNumberParam(lua_State *L, int tableIndex, T *value, char *name, T defaultValue = 0)
{
	CLuaRestoreStack l(L);

	if (name == NULL || !lua_istable(L,tableIndex))
	{
		*value = defaultValue;
		return FALSE;
	}

	lua_pushstring(L, name);
	lua_gettable(L, tableIndex - 1);

	if (lua_isnumber(L,-1) == FALSE)
	{
		*value = defaultValue;
		return FALSE;
	}

	*value = static_cast<T>(lua_tonumber(L,-1));
	return TRUE;

};


BOOL GetTableNumberParam(lua_State *L, int tableIndex, double *value, char *name, double defaultValue = 0);

BOOL GetTableBoolParam(lua_State *L, int tableIndex, bool *value, char *name, bool defaultValue = false);

BOOL GetTableStringParam(lua_State *L, BOOL tableIndex, string &value, char *name, const string &defaultValue = "");

BOOL FillTable(lua_State *L, int tableIndex, OUT map<string,any> &valuesmap);

}


	