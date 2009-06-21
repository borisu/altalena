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
#include "syslog.h"

using namespace boost;
using namespace std;

namespace ivrworx
{

	#define IW_MAX_SYSTEM_ERROR_MSG_LENGTH  1024
	#define IW_SINGLE_LOG_BUCKET_LENGTH 2048


	#define IW_LOG_LOG_COMPLETION_KEY	0
	#define IW_LOG_EXIT_COMPLETION_KEY	1

	static const char *g_LogLevelStrings[] = {"OFF", "CRT", "WRN", "INF", "DBG", "TRC"};

	string 
	FormatLastSysError(const char *lpszFunction) 
	{ 

		// Retrieve the system error message for the last-error code
		DWORD last_error = ::GetLastError(); 

		char translated_err_buffer[IW_MAX_SYSTEM_ERROR_MSG_LENGTH];
		translated_err_buffer[0]='\0';

		DWORD res = ::FormatMessageA (
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			last_error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			translated_err_buffer, 
			IW_MAX_SYSTEM_ERROR_MSG_LENGTH, 
			NULL);

		if (res == 0)
		{
			return string("Error while formating system error");
		}

		// Display the error message and exit the process
		char formatted_buffer[IW_MAX_SYSTEM_ERROR_MSG_LENGTH];

		::StringCchPrintfA(
			formatted_buffer, 
			IW_MAX_SYSTEM_ERROR_MSG_LENGTH,
			"%s failed with error %d: %s", 
			lpszFunction, 
			last_error, 
			translated_err_buffer); 

		return string(formatted_buffer);
	}

	mutex		g_loggerMutex;
	HANDLE		g_IocpLogger	= NULL;
	HANDLE		g_loggerThread	= NULL;

	LogLevel volatile g_LogLevel	= LOG_LEVEL_INFO;
	DWORD		g_logMask			= IX_LOG_MASK_CONSOLE;

	__declspec( thread ) debug_dostream *tls_logger = NULL;

	struct LogBucket :
		public OVERLAPPED
	{
		LogLevel log_level;

		string log_str;

		DWORD thread_id;

		PVOID fiber_id;

		DWORD timestamp;

	};

	DWORD WINAPI LoggerThread(LPVOID lpParam);
	
	BOOL InitLog(Configuration &conf)
	{
		mutex::scoped_lock scoped_lock(g_loggerMutex);

		SetLogLevelFromString(conf.DebugLevel());
		SetLogMaskFromString(conf.DebugOutputs());


		openlog(conf.SyslogHost().c_str(),conf.SyslogPort(),"ivrworx",0,LOG_USER );
		
		if (g_IocpLogger != NULL)
		{
			return TRUE;
		}

		g_IocpLogger = 	::CreateIoCompletionPort(
				INVALID_HANDLE_VALUE,
				NULL,
				0,
				1);

		if (g_IocpLogger == NULL)
		{
			string err = FormatLastSysError("CreateIoCompletionPort");
			std::cerr << "Cannot init logging - " << err;

			return FALSE;
		}

		DWORD tid = 0;
		g_loggerThread = ::CreateThread(
			NULL,
			0,
			LoggerThread,
			NULL,
			NULL,
			&tid
			);

		if (g_loggerThread == NULL)
		{
			string err = FormatLastSysError("CreateThread");
			std::cerr << "Cannot init logging - " << err;

			return FALSE;
		}

		return TRUE;
	}

	void ExitLog()
	{

		mutex::scoped_lock scoped_lock(g_loggerMutex);

		if (g_IocpLogger == NULL)
		{
			return;
		}

		::PostQueuedCompletionStatus(
			g_IocpLogger,
			0,
			IW_LOG_EXIT_COMPLETION_KEY,
			NULL);

		::Sleep(100);

		if (g_loggerThread != NULL) 
		{
			::CloseHandle(g_loggerThread);
			g_loggerThread = NULL;
		}

		if (g_IocpLogger != NULL) 
		{
			::CloseHandle(g_IocpLogger);
			g_IocpLogger = NULL;
		}

	}

	void
	SetLogLevelFromString(const string &level_str)
	{
		LogLevel debugLevel = LOG_LEVEL_INFO;
		if (level_str == "DBG")
		{
			debugLevel = LOG_LEVEL_DEBUG;
		};
		if (level_str == "TRC")
		{
			debugLevel = LOG_LEVEL_TRACE;
		};
		if (level_str == "WRN")
		{
			debugLevel = LOG_LEVEL_WARN;
		};
		if (level_str == "CRT")
		{
			debugLevel = LOG_LEVEL_CRITICAL;
		};
		if (level_str == "INF")
		{
			debugLevel = LOG_LEVEL_INFO;
		};
		if (level_str == "OFF")
		{
			debugLevel = LOG_LEVEL_OFF;
		};

		SetLogLevel(debugLevel);


	}

	void
	SetLogMaskFromString(const string &mask_str)
	{
		int mask = 0;

		size_t found = mask_str.find("console");
		if (found != string::npos)
		{
			mask |= IX_LOG_MASK_CONSOLE;
		}

		found = mask_str.find("debug");
		if (found != string::npos)
		{
			mask |= IX_LOG_MASK_DEBUGVIEW;
		}

		found = mask_str.find("syslog");
		if (found != string::npos)
		{
			mask |= IX_LOG_MASK_SYSLOG;
		}

		SetLogMask(mask);


	}

	int
	GetSyslogPri(IN LogLevel log_level)
	{
		switch(log_level)
		{
		case LOG_LEVEL_OFF:
			{
				return (LOG_MAKEPRI(LOG_USER,LOG_UPTO(LOG_EMERG)));
				break;
			}
		case LOG_LEVEL_CRITICAL:
			{
				return (LOG_MAKEPRI(LOG_USER,LOG_UPTO(LOG_EMERG)));
				break;
			}
		case LOG_LEVEL_WARN:
			{
				return (LOG_MAKEPRI(LOG_USER,LOG_UPTO(LOG_WARNING)));
				break;
			}
		case LOG_LEVEL_INFO:
			{
				return (LOG_MAKEPRI(LOG_USER,LOG_UPTO(LOG_INFO)));
				break;
			}
		case LOG_LEVEL_DEBUG:
			{
				return (LOG_MAKEPRI(LOG_USER,LOG_UPTO(LOG_DEBUG)));
				break;
			}
		case LOG_LEVEL_TRACE:
			{
				return (LOG_MAKEPRI(LOG_USER,LOG_UPTO(LOG_DEBUG)));
				break;
			}
		default:
			{
				throw;
			}
		}

	}

	void
	SetLogLevel(IN LogLevel log_level)
	{
		mutex::scoped_lock scoped_lock(g_loggerMutex);

		g_LogLevel = log_level;

		setlogmask(GetSyslogPri(log_level));

		
	}

	void 
	SetLogMask(IN int log_mask)
	{
		mutex::scoped_lock scoped_lock(g_loggerMutex);

		g_logMask = log_mask;
	}

	
	basic_debugbuf::~basic_debugbuf()
	{
		sync();
	}

	int 
	basic_debugbuf::sync()
	{
		LogBucket *lb = new LogBucket();

		lb->log_level = log_level;
		lb->thread_id = ::GetCurrentThreadId();
		lb->fiber_id  = ::GetCurrentFiber();
		lb->timestamp = ::GetTickCount();
		lb->log_str = str();

		::PostQueuedCompletionStatus(
			g_IocpLogger,
			IW_LOG_LOG_COMPLETION_KEY,
			0,
			lb);

		str(std::basic_string<char>());    // Clear the string buffer

		return 0;
	}

	debug_dostream::debug_dostream() 
		:char_stream(new basic_debugbuf()) 
	{

	};

	debug_dostream::~debug_dostream() 
	{
		delete rdbuf(); 
	}


	DWORD WINAPI LoggerThread(LPVOID lpParam)
	{
		DWORD number_of_bytes = 0;
		ULONG completion_key  = 0;
		OVERLAPPED *olap= NULL;

		while (true)
		{
			BOOL res  = ::GetQueuedCompletionStatus(
				g_IocpLogger,
				&number_of_bytes,
				&completion_key,
				&olap,
				5000
				);

			if (res == FALSE)
			{
				if (::GetLastError() == WAIT_TIMEOUT)
				{
					continue;
				}

				string err = FormatLastSysError("CreateIoCompletionPort");
				std::cerr << "Logger critical error, logging will stop, err:" << err ;
				break;
			}

			if (completion_key == IW_LOG_EXIT_COMPLETION_KEY)
			{
				break;
			}

			LogBucket *lb = (LogBucket *)olap;

			char formatted_log_str[IW_SINGLE_LOG_BUCKET_LENGTH];
			formatted_log_str[0] = '\0';

			int fiber_id = lb->fiber_id == NON_FIBEROUS_THREAD ? -1 : (int)lb->fiber_id;

			if (lb->log_str.length() > (IW_SINGLE_LOG_BUCKET_LENGTH - 1))
			{
				char membuf[IW_SINGLE_LOG_BUCKET_LENGTH];
				membuf[0]='\0';

				_snprintf_s(membuf,IW_SINGLE_LOG_BUCKET_LENGTH,IW_SINGLE_LOG_BUCKET_LENGTH,"Logger string too long, you may not log messages longer than %d bytes", 
					IW_SINGLE_LOG_BUCKET_LENGTH),

				lb->log_str = membuf;
				
			}

			_snprintf_s(formatted_log_str,IW_SINGLE_LOG_BUCKET_LENGTH,IW_SINGLE_LOG_BUCKET_LENGTH,"[%s][%-5d,0x%-8x] %s",
				g_LogLevelStrings[lb->log_level],
				lb->thread_id,
				fiber_id,
				lb->log_str.c_str());

			if (g_logMask & IX_LOG_MASK_CONSOLE)   
			{ 
				switch(lb->log_level)
				{
				case LOG_LEVEL_CRITICAL:
					{
						cout << con::bg_red;
						break;
					}
				case LOG_LEVEL_WARN:
					{
						cout << con::bg_magenta;
						break;
					}
				case LOG_LEVEL_OFF:
				case LOG_LEVEL_INFO:
				case LOG_LEVEL_DEBUG:
				case LOG_LEVEL_TRACE:
					{
						cout << con::bg_black;
						break;
					}
					
				default:
					{
						cout << con::bg_black;
					}
				}
				
				std::cout << formatted_log_str << con::bg_black;
			}

			if (g_logMask & IX_LOG_MASK_DEBUGVIEW) 
			{ 
				::OutputDebugStringA(formatted_log_str);
			};

			if (g_logMask & IX_LOG_MASK_SYSLOG)	
			{ 
				syslog(0,"%s",formatted_log_str); 
			};

			delete lb;

		}

		return 0;
		
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

		LogTrace(_funcname << " Enters -->");

	}

	LoggerTracker::~LoggerTracker()
	{
		if (_funcname[0] == '\0')
		{
			return;
		}

		LogTrace(_funcname << " <-- Exits");

	}

	ostream& 
	operator << (ostream& os, const Time &time)
	{
		return os << GetMilliSeconds(time) << " ms.";
	}

}