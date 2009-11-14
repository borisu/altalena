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
		method(ConfBridge, getstring),
		{0,0}
	};

	// needed to compile but must never be called
	ConfBridge::ConfBridge(lua_State *L):
	_conf(NULL)
	{
		throw;
	}

	ConfBridge::ConfBridge(Configuration *conf):
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

	

}

