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

namespace ivrworx
{



BOOL GetTableNumberParam(lua_State *L, int tableIndex, double *value, char *name, double defaultValue )
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

	*value = lua_tonumber(L,-1);
	return TRUE;

};

BOOL GetTableBoolParam(lua_State *L, int tableIndex, bool *value, char *name, bool defaultValue )
{
	CLuaRestoreStack l(L);

	if (name == NULL || !lua_istable(L,tableIndex))
	{
		*value = defaultValue;
		return FALSE;
	}

	lua_pushstring(L, name);
	lua_gettable(L, tableIndex - 1);

	if (lua_isboolean(L,-1) == FALSE)
	{
		*value = defaultValue;
		return FALSE;
	}

	*value = lua_toboolean(L,-1);
	return TRUE;

};

BOOL GetTableStringParam(lua_State *L, BOOL tableIndex, string &value, char *name, const string &defaultValue)
{
	CLuaRestoreStack l(L);

	if (name == NULL || !lua_istable(L,tableIndex))
	{
		value = defaultValue;
		return FALSE;
	}

	lua_pushstring(L, name);
	lua_gettable(L, tableIndex - 1);

	if (lua_isstring(L,-1) == FALSE)
	{
		value = defaultValue;
		return FALSE;
	}

	value = lua_tostring(L,-1);
	return TRUE;

};

BOOL FillTable(lua_State *L, int tableIndex, OUT MapOfAny &valuesmap)
{
	if (L == NULL || !lua_istable(L,tableIndex))
		return FALSE;

	lua_pushnil(L);
	while(lua_next( L, tableIndex - 1)) 
	{
		if (lua_isstring(L,-2))
		{
			string key = lua_tostring(L,-2);
			switch (lua_type(L,-1))
			{

			case LUA_TNUMBER:
				valuesmap[lua_tostring(L,-2)] = lua_tonumber(L,-1);
				break;


			case LUA_TBOOLEAN:
				valuesmap[lua_tostring(L,-2)] = lua_toboolean(L,-1);
				break;

			case LUA_TSTRING:
				valuesmap[lua_tostring(L,-2)] = string(lua_tostring(L,-1));
			case LUA_TNIL:
			case LUA_TTABLE:
			case LUA_TFUNCTION:; 
			case LUA_TUSERDATA:
			case LUA_TTHREAD:
			case LUA_TLIGHTUSERDATA:
			default:
				{

				}
			}

		}

		lua_pop(L, 1);

	}

	return TRUE;
}

}