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
#include "ConfBridge.h"

namespace ivrworx
{
	const char ConfBridge::className[] = "ConfBridge";
	Luna<ConfBridge>::RegType ConfBridge::methods[] = {
		method(ConfBridge, getstring),
		method(ConfBridge, getint),
		method(ConfBridge, getboolean),
		method(ConfBridge, getarray),
		{0,0}
	};

	// needed to compile but must never be called
	ConfBridge::ConfBridge(lua_State *L)
	{
		throw;
	}

	ConfBridge::ConfBridge(ConfigurationPtr conf):
	_conf(conf)
	{
	}

	ConfBridge::~ConfBridge(void)
	{
	}


	int 
	ConfBridge::getstring(lua_State *L)
	{
		if (lua_isstring(L, -1) != 1 )
		{
			LogWarn("ConfBridge::getstring wrong param.");
			return 0;
		}

		size_t string_length = 0;
		const char *log_string = lua_tolstring(L, -1, &string_length);

		string value;
		try
		{
			value = _conf->GetString(log_string);
		}
		catch (exception* e)
		{
			LogWarn("ConfBridge::getstring e:" << e->what());
		}

		lua_pushstring(L, value.c_str());
		return 1;
	

	}

	int 
	ConfBridge::getboolean(lua_State *L)
	{
		if (lua_isstring(L, -1) != 1 )
		{
			LogWarn("ConfBridge::getstring wrong param.");
			return 0;
		}

		size_t string_length = 0;
		const char *log_string = lua_tolstring(L, -1, &string_length);

		BOOL value = FALSE;
		try
		{
			value = _conf->GetBool(log_string);
		}
		catch (exception* e)
		{
			LogWarn("ConfBridge::getboolean e:" << e->what());
		}

		lua_pushboolean(L, value);
		return 1;


	}

	int 
	ConfBridge::getint(lua_State *L)
	{
		if (lua_isstring(L, -1) != 1 )
		{
			LogWarn("ConfBridge::getint wrong param.");
			return 0;
		}

		size_t string_length = 0;
		const char *log_string = lua_tolstring(L, -1, &string_length);


		string value;
		try
		{
			value = _conf->GetInt(log_string);
		}
		catch (exception* e)
		{
			LogWarn("ConfBridge::getint e:" << e->what());
		}

		lua_pushstring(L, value.c_str());
		return 1;

	}

	int 
	ConfBridge::getarray(lua_State *L)
	{
		if (lua_isstring(L, -1) != 1 )
		{
			LogWarn("ConfBridge::getarray wrong param.");
			return 0;
		}

		size_t string_length = 0;
		const char *log_string = lua_tolstring(L, -1, &string_length);


		string value;
		try
		{
			ListOfAny l;
			_conf->GetArray(log_string,l);

			lua_newtable(L);
			int index = 0;
			for (ListOfAny::iterator iter = l.begin(); iter != l.end(); ++iter)
			{

				lua_pushnumber(L,index);

				any &value = (*iter);
				if (typeid(int) == value.type())
				{
					lua_pushnumber(L, any_cast<int>(value));
				} 
				else if (typeid(string) == (*iter).type())
				{
					lua_pushstring(L, (any_cast<string>(value)).c_str());

				} 
				else if (typeid(BOOL) == (*iter).type())
				{
					lua_pushboolean(L, any_cast<BOOL>(value));
				}
				else
				{
					lua_pop(L,1);
					continue;
				}

				lua_settable(L,-3);
				++index;

			}

			return 1;
		}
		catch (exception* e)
		{
			LogWarn("ConfBridge::getarray e:" << e->what());
			return 0;
		}

		lua_pushstring(L, value.c_str());
		return 1;


	}
	

	

}

