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
#include "../../porttypes.h"
#include "Profiler.h"
#include "Logger.h"

using namespace std;

#define PROFILE_INIT() if (gt_ProfileMap == NULL){InitProfile();}

#define MAX_PROFILE_HISTORY 0xF

#define MAX_PROFILE_STACK_SIZE 2048

namespace ivrworx
{
	struct ProfileData
	{
		int64_t hits;

		int64_t all;

		int64_t avg;	

		int64_t counter;

		int64_t history[MAX_PROFILE_HISTORY];
		ProfileData()
		{
			hits = 0;
			avg	 = 0;
			all	 = 0;
			counter = 0;
			::memset(history,0,MAX_PROFILE_HISTORY*sizeof(int64_t));
		}

	} ;

	
	typedef 
	map<std::string,ProfileData*> ProfileMap;

	IW_THREAD_LOCAL ProfileMap *gt_ProfileMap = NULL;

	IW_THREAD_LOCAL LARGE_INTEGER gt_ticksPerSecond;

	IW_THREAD_LOCAL int64_t gt_ticksPerUs;

	IW_THREAD_LOCAL int64_t gt_ProfLastCheck;

	IW_THREAD_LOCAL int64_t gt_overhead[MAX_PROFILE_STACK_SIZE];

	IW_THREAD_LOCAL int gt_stackDepth;

	boost::mutex gb_print_mutex;

	void InitProfile()
	{
		gt_ProfileMap = new ProfileMap();

		// get the high resolution counter's accuracy
		BOOL res = ::QueryPerformanceFrequency(&gt_ticksPerSecond);
		if (res == FALSE)
		{
			std::cerr << "Error reading performance frequency, err:" << ::GetLastError() << endl;
			throw;
		}

		gt_ticksPerUs = (gt_ticksPerSecond.QuadPart/1000000);

		gt_ProfLastCheck = ::GetTickCount();

		gt_stackDepth = 0;

		gt_overhead[0] = 0;
		gt_overhead[1] = 0;

	}

#define  CALCULATE_DELTA(start, end) ((end.QuadPart - start.QuadPart)/gt_ticksPerUs)

	
	void CheckInterval(int64_t interval)
	{
		
		PROFILE_INIT();

		if (interval == 0 || interval < 0 || interval == INFINITE )
		{
			return;
		}
		
		int64_t currTime  = ::GetTickCount();
		if ( (currTime - gt_ProfLastCheck) >= interval)
		{
			PrintProfile();
			gt_ProfLastCheck = ::GetTickCount();
		}
	}

	ProfileData *
	AddProfilingData(const string &name)
	{
		PROFILE_INIT();


		ProfileMap::iterator iter = gt_ProfileMap->find(name);

		ProfileData *data = NULL;

		if (iter == gt_ProfileMap->end())
		{
			data = new ProfileData();
			(*gt_ProfileMap)[name] = data;
		} else
		{
			data = (*iter).second;
		}

		return data;

	}

	
	void PrintProfile()
	{
		PROFILE_INIT();

		boost::mutex::scoped_lock lock(gb_print_mutex);

		const long MAX_LONG_PRINT_BUF = 2048;
		char buf[MAX_LONG_PRINT_BUF];
		_snprintf_s(buf,  MAX_LONG_PRINT_BUF,MAX_LONG_PRINT_BUF,"Statistics for thread id %d", ::GetCurrentThreadId());
		::OutputDebugStringA(buf);

		_snprintf_s(buf,  MAX_LONG_PRINT_BUF,MAX_LONG_PRINT_BUF,"--------------------------------------------------------------------------------------------------");
		::OutputDebugStringA(buf);

		_snprintf_s(buf,  MAX_LONG_PRINT_BUF,MAX_LONG_PRINT_BUF,"Profiled Name												            Hits                           Cumulative" );
		::OutputDebugStringA(buf);

		_snprintf_s(buf,  MAX_LONG_PRINT_BUF,MAX_LONG_PRINT_BUF,"--------------------------------------------------------------------------------------------------");
		::OutputDebugStringA(buf);

		for (ProfileMap::iterator iter = gt_ProfileMap->begin();
			iter != gt_ProfileMap->end();
			iter ++)
		{
			ProfileData *data = (*iter).second;

			// prepare history string
			//
			stringstream s;
			for (int x = 0; x< std::min(data->counter,(int64_t)MAX_PROFILE_HISTORY); x++)
			{
				s << data->history[x] << " ";
				if (x < (MAX_PROFILE_HISTORY-1) )
				{
					s << "\t";
				}
			}


			_snprintf_s(buf,  MAX_LONG_PRINT_BUF,MAX_LONG_PRINT_BUF,"%-100.100s %-10I64d %-10I64d %s", (*iter).first.c_str(), data->hits, data->avg, s.str().c_str());
			::OutputDebugStringA(buf);
		}

		_snprintf_s(buf,  MAX_LONG_PRINT_BUF, MAX_LONG_PRINT_BUF,"=================================================================================================");
		::OutputDebugStringA(buf);
	}



	FuncProfiler::FuncProfiler(const string &string)
	{
		PROFILE_INIT();

		// what time is it?
		if (::QueryPerformanceCounter(&_start) == FALSE)
		{
			std::cerr << "Error reading performance counters, err:" << ::GetLastError() << endl;
			throw;
		};

		// start counting overhead
		LARGE_INTEGER overhead_start;
		if (::QueryPerformanceCounter(&overhead_start) == FALSE)
		{
			std::cerr << "Error reading performance counters, err:" << ::GetLastError() << endl;
			throw;
		};


		// zero the overhead of nested profiling functionality
		gt_overhead[gt_stackDepth+1] = 0;

		_funcName = string;

		LARGE_INTEGER overhead_end;
		if (::QueryPerformanceCounter(&overhead_end) == FALSE)
		{
			std::cerr << "Error reading performance counters, err:" << ::GetLastError() << endl;
			throw;
		};

		// add overhead of ctor
		int64_t overhead = CALCULATE_DELTA(overhead_start, overhead_end);
		_myOverhead =overhead;

		gt_stackDepth++;

	}

	FuncProfiler::~FuncProfiler(void)
	{
		
		LARGE_INTEGER overhead_start;
		if (::QueryPerformanceCounter(&overhead_start) == FALSE)
		{
			std::cerr << "Error reading performance counters, err:" << ::GetLastError() << endl;
			throw;
		};

		// Put as much as possible functionality 
		// inside overhead-measured code.

		ProfileData *data = AddProfilingData(_funcName);
		data->hits++;

		gt_stackDepth--;

		LARGE_INTEGER overhead_end;
		if (::QueryPerformanceCounter(&overhead_end) == FALSE)
		{
			std::cerr << "Error reading performance counters, err:" << ::GetLastError() << endl;
			throw;
		};

		LARGE_INTEGER end;
		if (::QueryPerformanceCounter(&end) == FALSE)
		{
			std::cerr << "Error reading performance counters, err:" << ::GetLastError() << endl;
			throw;
		};

		// VS 2005 does not show thread local arrays in 
		// debugging mode so I assign these values to local vars
   
		 
		int64_t *overhead_arr = gt_overhead;
		int stack_depth = gt_stackDepth;

		// add overhead of dtor unmeasured part of code takes app 5 us.
		int64_t overhead = CALCULATE_DELTA(overhead_start, overhead_end);
		_myOverhead += overhead + 5;
	
		overhead_arr[stack_depth] += _myOverhead + overhead_arr[stack_depth+1];

		int64_t delta = CALCULATE_DELTA(_start, end);

		// when you calculate delta of measured code 
		// you have to reduce the overhead of calling
		// scoped profiler itself and all overheads
		// of nested profilers.

		int64_t delta_without_overhead = delta  - _myOverhead - overhead_arr[stack_depth+1];

		
		delta_without_overhead = delta_without_overhead <  0 ? 0 : delta_without_overhead ;

		
		data->all += delta_without_overhead;
		data->avg = data->all/(data->hits);
		data->history[(data->counter++) & MAX_PROFILE_HISTORY] = delta_without_overhead;


	}
}




