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
#include "CcuLogger.h"



using namespace boost;
using namespace std;

mutex g_loggerMutex;

volatile IxLogLevel g_LogLevel = IX_LOG_LEVEL_INFO;

volatile DWORD g_logMask = IX_LOG_MASK_CONSOLE;

wdostream dbgout;

void
IxSetLogLevel(IxLogLevel log_level)
{
	g_LogLevel = log_level;
}

void 
IxSetLogMask(int log_mask)
{
	g_logMask = log_mask;
}

void 
LoggerTracker::DispatchLog(boolean exit)
{
 	if (g_logMask & IX_LOG_MASK_CONSOLE) 
 	{ 
 		Print(wcout,exit);
 	}
 	if (g_logMask & IX_LOG_MASK_DEBUGVIEW) 
 	{  
 		Print(dbgout,exit);
 	}

}

#pragma TODO("Log level of function tracker should be configurable")
void 
LoggerTracker::Print(wostream &stream, 
					 boolean exit)
{
	mutex::scoped_lock scoped_lock(g_loggerMutex);
	if (!exit)
	{
		stream <<  PREFIX << "[TRC]:" << _funcname << " Enters -->" << endl;
	} else 
	{
		stream <<  PREFIX << "[TRC]:" <<_funcname << " <-- Exits" << endl;
	}
	
}


LoggerTracker::LoggerTracker(wchar_t *log_function)
{
	if (g_LogLevel < IX_LOG_LEVEL_TRACE)
	{
		_funcname[0] = '\0';
		return;
	}
	wcsncpy_s(_funcname, 
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

wostream& 
operator << (wostream& os, const Time &time)
{
	return os << "time=[" << GetMilliSeconds(time) << "] ms.";
}


#define CCU_MAX_SYSTEM_ERROR_MSG_LENGTH  1024

wstring 
FormatLastSysError(LPTSTR lpszFunction) 
{ 
	
	// Retrieve the system error message for the last-error code
	DWORD last_error = ::GetLastError(); 

	wchar_t translated_err_buffer[CCU_MAX_SYSTEM_ERROR_MSG_LENGTH];

	DWORD res = ::FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		last_error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&translated_err_buffer, 
		CCU_MAX_SYSTEM_ERROR_MSG_LENGTH, 
		NULL);


	if (res == 0)
	{
		return wstring(L"Error while formating system error");
	}


	// Display the error message and exit the process

	wchar_t formatted_buffer[CCU_MAX_SYSTEM_ERROR_MSG_LENGTH];

	::StringCchPrintfW (
		(LPWSTR)formatted_buffer, 
		CCU_MAX_SYSTEM_ERROR_MSG_LENGTH,
		TEXT("%s failed with error %d: %s"), 
		lpszFunction, 
		last_error, 
		translated_err_buffer); 

#ifdef DEBUG
	MessageBox(NULL, (LPCTSTR)formatted_buffer, TEXT("Error"), MB_OK); 
#endif

	return wstring((LPCTSTR)formatted_buffer);
}