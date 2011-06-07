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

	#define IW_MAX_MESSAGES_IN_QUEUE	1000


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

	HANDLE		g_queueSemaphore = NULL;

	HANDLE		g_IocpLogger	= NULL;
	HANDLE		g_loggerThread	= NULL;

	IW_CORE_API extern LogLevel g_MaxLogLevel 	= LOG_LEVEL_INFO;
	DWORD	 g_logMask		= IW_LOG_MASK_CONSOLE;
	BOOL	 g_LogSyncMode  = FALSE;

// 	__declspec( thread ) debug_dostream *tls_logger = NULL;
// 
// 	__declspec( thread ) BOOL script_log = FALSE;
// 
// 	__declspec( thread ) HANDLE tls_completion_port = NULL;
// 
// 	__declspec( thread ) HANDLE tls_queue_semaphore = NULL;
// 
// 	__declspec( thread ) BOOL tls_sync_mode= FALSE;
	
#define LOGGER_SLOT 1
	IW_CORE_API debug_dostream *GetTlsLogger()
	{
		DWORD os_res = ::GetLastError();
		LPVOID res = NULL;
		if (GetCoreData(LOGGER_SLOT,&res) && res != NULL)
		{
			::SetLastError(os_res);
			return (debug_dostream *)res;
			
		}
		else
		{
			StoreCoreData(LOGGER_SLOT, new debug_dostream());
			return  GetTlsLogger();
		}

	}

#define SCRIPT_LOG_SLOT 2
	IW_CORE_API BOOL GetScriptLog()
	{
		LPVOID res = NULL;
		if (GetCoreData(SCRIPT_LOG_SLOT,&res))
		{
			return (BOOL)res;

		}
		else
		{
			StoreCoreData(SCRIPT_LOG_SLOT, FALSE);
			return  GetScriptLog();
		}
		
	}

#define TLS_COMPLETION_PORT 3
	IW_CORE_API HANDLE GetTlsCompletionPort()
	{
		LPVOID res = NULL;
		if (GetCoreData(TLS_COMPLETION_PORT,&res))
		{
			return (HANDLE)res;

		}
		else
		{
			StoreCoreData(TLS_COMPLETION_PORT, NULL);
			return  GetTlsCompletionPort();
		}



	}

#define QUEUE_SEMAPHORE_SLOT 4
	IW_CORE_API HANDLE GetTlsQueueSemaphore()
	{
		LPVOID res = NULL;
		if (GetCoreData(QUEUE_SEMAPHORE_SLOT,&res))
		{
			return (HANDLE)res;

		}
		else
		{
			StoreCoreData(QUEUE_SEMAPHORE_SLOT, NULL);
			return  GetTlsQueueSemaphore();
		}



	}

#define TLS_SYNC_MODE_SLOT 5
	IW_CORE_API  BOOL GetTlsSyncMode()
	{
		LPVOID res = NULL;
		if (GetCoreData(TLS_SYNC_MODE_SLOT,&res))
		{
			return (BOOL)res;
		}
		else
		{
			StoreCoreData(TLS_SYNC_MODE_SLOT, FALSE);
			return  GetTlsSyncMode();
		}
	}




	struct LogBucket :
		public OVERLAPPED
	{
		LogLevel log_level;

		string log_str;

		DWORD thread_id;

		PVOID fiber_id;

		DWORD timestamp;

		BOOL script_log;

	};

	DWORD WINAPI LoggerThread(LPVOID lpParam);

	IW_CORE_API void 
	IwStartScript()
	{
		StoreCoreData(SCRIPT_LOG_SLOT, (LPVOID)TRUE);
	}

	IW_CORE_API void
	IwStopScript()
	{
		StoreCoreData(SCRIPT_LOG_SLOT, (LPVOID)FALSE);
	}
	
	BOOL 
	InitLog(ConfigurationPtr conf)
	{
		mutex::scoped_lock scoped_lock(g_loggerMutex);

		SetLogLevelFromString(conf->GetString("debug_level"));
		SetLogMaskFromString(conf->GetString("debug_outputs"));

		g_LogSyncMode = conf->GetBool("sync_log");

		openlog(conf->GetString("syslogd_host").c_str(),conf->GetInt("syslogd_port"),"ivrworx",0,LOG_USER );

		if (conf->GetBool("sync_log"))
		{
			return TRUE;
		}
		
		if (g_IocpLogger != NULL)
		{
			return TRUE;
		}

		g_queueSemaphore = ::CreateSemaphore(
			NULL,
			IW_MAX_MESSAGES_IN_QUEUE,
			IW_MAX_MESSAGES_IN_QUEUE,NULL);

		if (g_queueSemaphore == NULL)
		{
			string err = FormatLastSysError("CreateSemaphore");
			std::cerr << "Cannot init logging - " << err;

			goto error;
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
			::CloseHandle(g_queueSemaphore);

			goto error;
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

			goto error;
		}

		return TRUE;
error:

		if (g_loggerThread) 
		{
			::CloseHandle(g_loggerThread);
			g_loggerThread = NULL;
		};

		if (g_IocpLogger) 
		{
			::CloseHandle(g_IocpLogger);
			g_IocpLogger = NULL;
		};

		if (g_queueSemaphore) 
		{
			::CloseHandle(g_queueSemaphore);
			g_queueSemaphore = NULL;
		};

		return FALSE;

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
			::TerminateThread(g_loggerThread, -1);
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
			mask |= IW_LOG_MASK_CONSOLE;
		}

		found = mask_str.find("debug");
		if (found != string::npos)
		{
			mask |= IW_LOG_MASK_DEBUGVIEW;
		}

		found = mask_str.find("syslog");
		if (found != string::npos)
		{
			mask |= IW_LOG_MASK_SYSLOG;
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

		g_MaxLogLevel = log_level;

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

	// forward declaration
	void LogBucketAndDelete(LogBucket *lb);

	int 
	basic_debugbuf::sync()
	{
		
		LogBucket *lb = new LogBucket();

		lb->log_level  = log_level;
		lb->thread_id  = ::GetCurrentThreadId();
		lb->fiber_id   = ::GetCurrentFiber();
		lb->timestamp  = ::GetTickCount();
		lb->log_str	   = str();
		lb->script_log = GetScriptLog();

		// Clear the string buffer
		str(std::basic_string<char>());

		if (GetTlsSyncMode() == TRUE)
		{
			mutex::scoped_lock scoped_lock(g_loggerMutex);
			LogBucketAndDelete(lb);
		}
		else
		{
			if (GetTlsQueueSemaphore() == NULL || 
				GetTlsCompletionPort() == NULL)
			{
				return 0;
			}

			DWORD res = ::WaitForSingleObject(
				GetTlsQueueSemaphore(),
				INFINITE);

			if (res != WAIT_OBJECT_0)
			{
				delete lb;
				return 0;
			}

			if (GetTlsCompletionPort() == NULL)
			{
				return 0;
			}

			res = ::PostQueuedCompletionStatus(
				GetTlsCompletionPort(),
				IW_LOG_LOG_COMPLETION_KEY,
				0,
				lb);
		}

		    

		return 0;
	}

	debug_dostream::debug_dostream() 
		:char_stream(new basic_debugbuf()) 
	{

		mutex::scoped_lock scoped_lock(g_loggerMutex);

		StoreCoreData(TLS_SYNC_MODE_SLOT,(LPVOID)g_LogSyncMode);

		if (g_IocpLogger == NULL		||
			g_queueSemaphore == NULL	||
			GetTlsQueueSemaphore() != NULL ||
			GetTlsCompletionPort() !=NULL)
		{
			return;
		}

		HANDLE queue_handle = GetTlsQueueSemaphore();
		int res  = ::DuplicateHandle(
						::GetCurrentProcess(), 
						g_queueSemaphore, 
						::GetCurrentProcess(),
						&queue_handle, 
						0,
						FALSE,
						DUPLICATE_SAME_ACCESS);

		if (res == 0)
		{
			DWORD os_res = ::GetLastError();
			string msg = "Cannot initiate logging (g_queueSemaphore) GetLastError=" + os_res;
			throw std::exception(msg.c_str());
		}

		HANDLE io_handle = GetTlsQueueSemaphore();
		res  = ::DuplicateHandle(
			::GetCurrentProcess(), 
			g_IocpLogger, 
			::GetCurrentProcess(),
			&io_handle, 
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS);

		if (res == 0)
		{
			DWORD os_res = ::GetLastError();
			string msg = "Cannot initiate logging (g_IocpLogger) GetLastError=" + os_res;
			throw exception(msg.c_str());
		}

	};

	debug_dostream::~debug_dostream() 
	{
		delete rdbuf(); 
	}

	void 
	LogBucketAndDelete(LogBucket *lb)
	{
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

		const char *g_LogLevelStrings[] = {"OFF", "CRT", "WRN", "INF", "DBG", "TRC"};

		_snprintf_s(formatted_log_str,IW_SINGLE_LOG_BUCKET_LENGTH,IW_SINGLE_LOG_BUCKET_LENGTH,"[%s][%-5d,0x%-8x] %s",
			g_LogLevelStrings[lb->log_level],
			lb->thread_id,
			fiber_id,
			lb->log_str.c_str());

		if (g_logMask & IW_LOG_MASK_CONSOLE)   
		{ 
			cout << ((lb->script_log == TRUE)? con::fg_yellow : con::fg_white);

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

		if (g_logMask & IW_LOG_MASK_DEBUGVIEW) 
		{ 
			::OutputDebugStringA(formatted_log_str);
		};

		if (g_logMask & IW_LOG_MASK_SYSLOG)	
		{ 
			syslog(0,"%s",formatted_log_str); 
		};

		delete lb;

		

	}


	DWORD WINAPI 
	LoggerThread(LPVOID lpParam)
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

			LogBucketAndDelete(lb);

			::ReleaseSemaphore(g_queueSemaphore,1,NULL);

			
		}

		return 0;
		
	}

	
	LoggerTracker::LoggerTracker(IN char *log_function)
	{
		if (::InterlockedExchangeAdd((LONG*)&g_MaxLogLevel,0) < LOG_LEVEL_TRACE)
		{
			_funcname[0] = '\0';
			return;
		}

		strncpy_s(_funcname, 
			log_function, 
			MAX_LENGTH);

		COND_LOG(LOG_LEVEL_TRACE,_funcname << " Enters -->")
	}

	LoggerTracker::~LoggerTracker()
	{
		if (_funcname[0] == '\0')
		{
			return;
		}

		COND_LOG(LOG_LEVEL_TRACE,_funcname << " <-- Exits");

	}

	ostream& 
	operator << (ostream& os, const Time &time)
	{
		return os << GetMilliSeconds(time) << " ms.";
	}

}