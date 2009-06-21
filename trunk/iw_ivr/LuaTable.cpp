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

#include "stdafx.h"
#include "LuaTable.h"
#include "LuaRestoreStack.h"


namespace ivrworx
{
	LuaTable::LuaTable(IN CLuaVirtualMachine& vm): 
		_oldRef (0), 
		_vm (vm),
		_tableRef(0)
	{

	}

	void 
	LuaTable::Create(IN const string &table_name)
	{
		if (table_name == "")
		{
			LogCrit("Cannot create unnamed table");
			throw;
		}

		if (_tableName != "")
		{
			LogCrit("Cannot create table twice");
			throw;
		}

		_tableName = table_name;

		if (_vm.Ok ())
		{
			lua_State *state = (lua_State *) _vm;

			lua_newtable (state);
			_tableRef = luaL_ref (state, LUA_REGISTRYINDEX);

			// Save the old table
			lua_getglobal (state,table_name.c_str());
			_oldRef = luaL_ref (state, LUA_REGISTRYINDEX);

			// replace it with our new one
			lua_rawgeti(state, LUA_REGISTRYINDEX, _tableRef);
			lua_setglobal (state, table_name.c_str());
		}
	}

	void 
	LuaTable::AddParam(IN const string &key, IN const string &value)
	{
		if (_vm.Ok () == false || _tableRef == 0)
		{
			LogCrit("Cannot add parameter to unintialized table");
			throw;
		}
		CLuaRestoreStack rs (_vm);

		lua_State *state = (lua_State *) _vm;
		lua_rawgeti (state, LUA_REGISTRYINDEX, _tableRef);

		// Push the function and parameters
		lua_pushstring (state, key.c_str());
		lua_pushstring (state, value.c_str());
		lua_settable (state, -3);
	}

	void 
	LuaTable::AddFunction(IN const string &key, IN const lua_CFunction func)
	{
		if (_vm.Ok () == false || _tableRef == 0)
		{
			LogCrit("Cannot add parameter to unintialized table");
			throw;
		}

		CLuaRestoreStack rs (_vm);

		lua_State *state = (lua_State *) _vm;
		lua_rawgeti (state, LUA_REGISTRYINDEX, _tableRef);

		// Push the function and parameters
		lua_pushstring (state, key.c_str());
		lua_pushcfunction(state, func);
		lua_settable (state, -3);
	}


	LuaTable::~LuaTable (void)
	{
		if (_vm.Ok () == false  || 
			_tableName == ""	||  
			_tableRef == 0) 
		{
			return;
		}

		lua_State *state = (lua_State *) _vm;
		if (_oldRef > 0 && _vm.Ok ())
		{
			// Replace the old "this" table
			lua_rawgeti(state, LUA_REGISTRYINDEX, _oldRef);
			lua_setglobal (state, _tableName.c_str());
			luaL_unref (state, LUA_REGISTRYINDEX, _oldRef);
		}
	}
}