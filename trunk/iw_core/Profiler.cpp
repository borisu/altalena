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
#include "Profiler.h"
#include "Logger.h"

using namespace std;

#define PROFILE_INIT() if (gt_ProfileMap == NULL){InitProfile();}

#define MAX_PROFILE_HISTORY 20

namespace ivrworx
{
	struct ProfileData
	{
		__int64 hits;

		__int64 all;

		__int64 avg;	

		__int64 counter;

		__int64 history[MAX_PROFILE_HISTORY];
		ProfileData()
		{
			hits = 0;
			avg	 = 0;
			all	 = 0;
			counter = 0;
			::memset(history,0,MAX_PROFILE_HISTORY*sizeof(__int64));
		}

	} ;

	typedef 
		map<std::string,ProfileData*> ProfileMap;

	__declspec(thread) ProfileMap *gt_ProfileMap = NULL;

	__declspec(thread) LARGE_INTEGER gt_ticksPerSecond;

	__declspec(thread) __int64 gt_ticksPerUs;

	__declspec(thread) __int64 gt_ProfLastCheck;


	void InitProfile()
	{
		gt_ProfileMap = new ProfileMap();

		// get the high resolution counter's accuracy
		::QueryPerformanceFrequency(&gt_ticksPerSecond);

		gt_ticksPerUs = (gt_ticksPerSecond.QuadPart/1000000);

		gt_ProfLastCheck = ::GetTickCount();

	}

	__int64 calculateDelta(LARGE_INTEGER start, LARGE_INTEGER end)
	{
		PROFILE_INIT();

		__int64 delta_us = (end.QuadPart - start.QuadPart)/gt_ticksPerUs;
		return delta_us;
	}

	void CheckInterval(__int64 interval)
	{
		
		PROFILE_INIT();

		if (interval == 0 || interval < 0 || interval == INFINITE )
		{
			return;
		}
		
		__int64 currTime  = ::GetTickCount();
		if ( (currTime - gt_ProfLastCheck) >= interval)
		{
			PrintProfile();
			gt_ProfLastCheck = ::GetTickCount();
		}
	}

	void AddProfilingData(string name, LARGE_INTEGER start)
	{
		PROFILE_INIT();

		LARGE_INTEGER end;
		if (::QueryPerformanceCounter(&end) == FALSE)
		{
			throw;
		};
		AddProfilingData(name,start,end);

	}

	void AddProfilingData(string name, LARGE_INTEGER start, LARGE_INTEGER end)
	{
		PROFILE_INIT();


		__int64 delta_us = calculateDelta(start, end);

		ProfileMap::iterator iter = gt_ProfileMap->find(name);

		if (iter == gt_ProfileMap->end())
		{
			ProfileData *data = new ProfileData();
			data->hits = 1;
			data->all = delta_us;
			data->avg = delta_us;
			data->history[(data->counter++)%MAX_PROFILE_HISTORY] = delta_us;
			(*gt_ProfileMap)[name] = data;
		} else
		{
			ProfileData *data = (*iter).second;
			data->hits++;
			data->all += delta_us;
			data->avg = data->all/(data->hits);
			(*gt_ProfileMap)[name] = data;
			data->history[(data->counter++)%MAX_PROFILE_HISTORY] = delta_us;
		}


	}

	
	void PrintProfile()
	{
		PROFILE_INIT();

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
			for (int x = 0; x< std::min(data->counter,(__int64)MAX_PROFILE_HISTORY); x++)
			{
				s << data->history[x] << " ";
				if (x < (MAX_PROFILE_HISTORY-1) )
				{
					s << ", ";
				}
			}



			
			_snprintf_s(buf,  MAX_LONG_PRINT_BUF,MAX_LONG_PRINT_BUF,"%-100.100s %-30I64d %-30I64d %s", (*iter).first.c_str(), data->hits, data->avg, s.str().c_str());
			::OutputDebugStringA(buf);
		}

		_snprintf_s(buf,  MAX_LONG_PRINT_BUF, MAX_LONG_PRINT_BUF,"=================================================================================================");
		::OutputDebugStringA(buf);
	}

	FuncProfiler::FuncProfiler(string string)
	{
		PROFILE_INIT();

		_funcName = string;

		// what time is it?
		if (::QueryPerformanceCounter(&_start) == FALSE)
		{
			throw;
		};
	}

	FuncProfiler::~FuncProfiler(void)
	{
		
		AddProfilingData(_funcName,_start);
		
	}
}




