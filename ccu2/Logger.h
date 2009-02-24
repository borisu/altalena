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

using namespace boost;
using namespace csp;
using namespace std;
using namespace JadedHoboConsole;
namespace con = JadedHoboConsole;

namespace ivrworx
{

	template <class CharT, class TraitsT = std::char_traits<CharT> >
	class basic_debugbuf : 
		public std::basic_stringbuf<CharT, TraitsT>
	{
	public:

		virtual ~basic_debugbuf()
		{
			sync();
		}

	protected:

		int sync()
		{
			output_debug_string(str().c_str());
			str(std::basic_string<CharT>());    // Clear the string buffer


			return 0;
		}

		void output_debug_string(const CharT *text) {}
	};

	template<>
	void basic_debugbuf<char>::output_debug_string(const char *text)
	{
		::OutputDebugStringA(text);
	}

	template<>
	void basic_debugbuf<wchar_t>::output_debug_string(const wchar_t *text)
	{
		::OutputDebugStringW(text);
	}

	template<class CharT, class TraitsT = std::char_traits<CharT> >
	class basic_dostream : 
		public std::basic_ostream<CharT, TraitsT>
	{
	public:

		basic_dostream() : std::basic_ostream<CharT, TraitsT>
			(new basic_debugbuf<CharT, TraitsT>()) {}
		~basic_dostream() 
		{
			delete rdbuf(); 
		}
	};

	typedef basic_dostream<char>    dostream;
	typedef basic_dostream<wchar_t> wdostream;

	extern wdostream wdbgout;
	extern dostream	 dbgout;

#define IX_LOG_MASK_CONSOLE		0x1
#define IX_LOG_MASK_DEBUGVIEW	0x10

	// global logging mutex
	extern mutex g_loggerMutex;

	extern volatile DWORD g_logMask;

	void
	SetLogMask(IN  int mask);


	string 
	FormatLastSysError(IN char *lpszFunction);

	class LoggerTracker
	{
	public:
		LoggerTracker(char *log_function);
		~LoggerTracker();
	private:
		void DispatchLog(boolean exit);
		void Print(ostream &stream, boolean exit);
		static const int MAX_LENGTH = 100; 
		char _funcname[MAX_LENGTH];
	};


#define __STR2WSTR(str)    L##str
#define _STR2WSTR(str)     __STR2WSTR(str)

#define __FILEW__          _STR2WSTR(__FILE__)
#define __FUNCTIONW__      _STR2WSTR(__FUNCTION__)


#define IX_THREAD_ID		"[" << dec << setw(5) << ::GetCurrentThreadId() << "," << ::GetCurrentFiber() << "]"

#ifndef DISABLE_SRC_REF
	#define IX_PREFIX				IX_THREAD_ID << __FUNCTION__ << "(" << dec << __LINE__ << "):"
	#define IX_PREFIX_WITH_LINE		IX_THREAD_ID << __FUNCTION__ << "(" << dec << __LINE__ << "):"
#else
	#define IX_PREFIX				IX_THREAD_ID 
	#define IX_PREFIX_WITH_LINE		IX_THREAD_ID 
#endif

#define LOG_HANDLE(x) hex << (x)

	enum LogLevel
	{
		LOG_LEVEL_OFF,
		LOG_LEVEL_CRITICAL,
		LOG_LEVEL_WARN,
		LOG_LEVEL_INFO,
		LOG_LEVEL_DEBUG,
		LOG_LEVEL_TRACE
	};

	extern volatile LogLevel g_LogLevel;

	void
	SetLogLevel(LogLevel log_level);

#define IX_SCOPED_LOG(level ,x, color){ \
	mutex::scoped_lock scoped_lock(g_loggerMutex);\
	if (g_logMask & IX_LOG_MASK_CONSOLE)   { std::cout << color;(std::cout)  << IX_PREFIX << level << x << endl; std::cout << con::bg_black;}\
	if (g_logMask & IX_LOG_MASK_DEBUGVIEW) { std::cout << color;(dbgout)	 << IX_PREFIX << level << x << endl; std::cout << con::bg_black;}\
	}

#define IX_SCOPED_LOG_RAW(x){ \
	mutex::scoped_lock scoped_lock(g_loggerMutex);\
	if (g_logMask & IX_LOG_MASK_CONSOLE)   { (std::cout)  << x << endl; }\
	if (g_logMask & IX_LOG_MASK_DEBUGVIEW) { (dbgout)	  << x << endl; }\
	}

#define IsDebug()   (g_LogLevel >= IX_LOG_LEVEL_DEBUG)

#ifdef PROFILE
	#define LogProfile(x)SCOPED_LOG("[PRF]:",x, con::bg_black)
#else
	#define LogProfile(x)
#endif

#ifndef NOLOGS
	#define LogTrace(x) if (g_LogLevel >= LOG_LEVEL_TRACE)		IX_SCOPED_LOG("[TRC]:",x, con::bg_black)
	#define LogDebug(x) if (g_LogLevel >= LOG_LEVEL_DEBUG)		IX_SCOPED_LOG("[DBG]:",x, con::bg_black)
	#define LogInfo(x)  if (g_LogLevel >= LOG_LEVEL_INFO)		IX_SCOPED_LOG("[INF]:",x, con::bg_black)
	#define LogWarn(x)  if (g_LogLevel >= LOG_LEVEL_WARN)		IX_SCOPED_LOG("[WRN]:",x, con::bg_red)
	#define LogCrit(x)	if (g_LogLevel >= LOG_LEVEL_CRITICAL)	IX_SCOPED_LOG("[CRT]:",x, con::bg_red)

	#define LogTraceRaw(x) if (g_LogLevel >= LOG_LEVEL_TRACE)		IX_SCOPED_LOG_RAW(x)
	#define LogDebugRaw(x) if (g_LogLevel >= LOG_LEVEL_DEBUG)		IX_SCOPED_LOG_RAW(x)
	#define LogInfoRaw(x)  if (g_LogLevel >= LOG_LEVEL_INFO)		IX_SCOPED_LOG_RAW(x)
	#define LogWarnRaw(x)  if (g_LogLevel >= LOG_LEVEL_WARN)		IX_SCOPED_LOG_RAW(x)
	#define LogCritRaw(x)  if (g_LogLevel >= LOG_LEVEL_CRITICAL)	IX_SCOPED_LOG_RAW(x)
	#define LogSysError(x) { \
		mutex::scoped_lock scoped_lock(g_loggerMutex);\
		(std::cout) << con::bg_red << IX_PREFIX_WITH_LINE  << " " << x << " " << FormatLastSysError(__FUNCTION__) << con::fg_white << endl;\
	}
	#define FUNCTRACKER LoggerTracker _ltTag(__FUNCTION__) 
#else
	#define LogTrace(x) 
	#define LogDebug(x) 
	#define LogInfo(x)  
	#define LogWarn(x)  
	#define LogCrit(x)	

	#define LogTraceRaw(x) 
	#define LogDebugRaw(x) 
	#define LogInfoRaw(x)  
	#define LogWarnRaw(x)  
	#define LogCritRaw(x)

	#define LogSysError(x)

	#define FUNCTRACKER  
#endif

extern ostream& operator << (ostream& os, const Time &time);

}