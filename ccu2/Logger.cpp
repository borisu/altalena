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
#include "Logger.h"

using namespace boost;
using namespace std;

namespace ivrworx
{

	mutex g_loggerMutex;

	volatile LogLevel g_LogLevel = LOG_LEVEL_INFO;

	volatile DWORD g_logMask = IX_LOG_MASK_CONSOLE;

	wdostream wdbgout;

	dostream dbgout;

	void
	SetLogLevel(IN LogLevel log_level)
	{
		g_LogLevel = log_level;
	}

	void 
	SetLogMask(IN int log_mask)
	{
		g_logMask = log_mask;
	}

	void 
	LoggerTracker::DispatchLog(IN boolean exit)
	{
		if (g_logMask & IX_LOG_MASK_CONSOLE) 
		{ 
			Print(cout,exit);
		}
		if (g_logMask & IX_LOG_MASK_DEBUGVIEW) 
		{  
			Print(dbgout,exit);
		}

	}

	void 
	LoggerTracker::Print(
		IN ostream &stream, 
		IN boolean exit)
	{
		mutex::scoped_lock scoped_lock(g_loggerMutex);
		if (!exit)
		{
			stream << IX_PREFIX << "[TRC]:" << _funcname << " Enters -->" << endl;
		} else 
		{
			stream << IX_PREFIX << "[TRC]:" << _funcname << " <-- Exits" << endl;
		}

	}


	LoggerTracker::LoggerTracker(IN char *log_function)
	{
		if (g_LogLevel < LOG_LEVEL_TRACE)
		{
			_funcname[0] = '\0';
			return;
		}
		strncpy_s(_funcname, 
			log_function, 
			MAX_LENGTH);

		DispatchLog(false);

	}

	LoggerTracker::~LoggerTracker()
	{
		if (_funcname[0] == '\0')
		{
			return;
		}

		DispatchLog(true);

	}

	ostream& 
	operator << (ostream& os, const Time &time)
	{
		return os << GetMilliSeconds(time) << " ms.";
	}


#define CCU_MAX_SYSTEM_ERROR_MSG_LENGTH  1024

	string 
	FormatLastSysError(char *lpszFunction) 
	{ 

		// Retrieve the system error message for the last-error code
		DWORD last_error = ::GetLastError(); 

		char translated_err_buffer[CCU_MAX_SYSTEM_ERROR_MSG_LENGTH];
		translated_err_buffer[0]='\0';

		DWORD res = ::FormatMessageA (
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			last_error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			translated_err_buffer, 
			CCU_MAX_SYSTEM_ERROR_MSG_LENGTH, 
			NULL);

		if (res == 0)
		{
			return string("Error while formating system error");
		}

		// Display the error message and exit the process
		char formatted_buffer[CCU_MAX_SYSTEM_ERROR_MSG_LENGTH];

		::StringCchPrintfA(
			formatted_buffer, 
			CCU_MAX_SYSTEM_ERROR_MSG_LENGTH,
			"%s failed with error %d: %s", 
			lpszFunction, 
			last_error, 
			translated_err_buffer); 

		return string(formatted_buffer);
	}

}