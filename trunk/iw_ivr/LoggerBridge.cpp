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
#include "LoggerBridge.h"

namespace ivrworx
{

	const char LoggerBridge::className[] = "LoggerBridge";
	Luna<LoggerBridge>::RegType LoggerBridge::methods[] = {
		method(LoggerBridge, loginfo),
		method(LoggerBridge, logwarn),
		method(LoggerBridge, logcrit),
		method(LoggerBridge, logdebug),
		{0,0}
	};


	int 
	LoggerBridge::loginfo(lua_State *L)
	{
		LuaLog(LOG_LEVEL_INFO, L);
		return 0;

	};

	int 
	LoggerBridge::logwarn(lua_State *L)
	{
		LuaLog(LOG_LEVEL_WARN, L);
		return 0;

	};

	int 
	LoggerBridge::logcrit(lua_State *L)
	{
		LuaLog(LOG_LEVEL_CRITICAL, L);
		return 0;

	};

	int 
	LoggerBridge::logdebug(lua_State *L)
	{
		LuaLog(LOG_LEVEL_DEBUG, L);
		return 0;
	};

	LoggerBridge::LoggerBridge(lua_State *L)
	{

	};

	void
	LoggerBridge::LuaLog(LogLevel log_level, lua_State *state)
	{

		if (lua_isstring(state, -1) != 1 )
		{
			LogWarn("Wrong type of parameter for log - logstring");
			return;
		}

		size_t string_length = 0;
		const char *log_string = lua_tolstring(state, -1, &string_length);

		LogStartScriptLog();
		switch(log_level)
		{
		case LOG_LEVEL_OFF:
			{
				break;
			}
		case LOG_LEVEL_CRITICAL:
			{
				LogCrit(log_string);
				break;
			}
		case LOG_LEVEL_WARN:
			{
				LogWarn(log_string);
				break;
			}
		case LOG_LEVEL_INFO:
			{
				LogInfo(log_string);
				break;
			}
		case LOG_LEVEL_DEBUG:
			{
				LogDebug(log_string);
				break;
			}
		case LOG_LEVEL_TRACE:
			{
				LogTrace(log_string);
				break;
			}
		default:
			{
				LogDebug(log_string);
			}
		}

		LogStopScriptLog();

	}

}

