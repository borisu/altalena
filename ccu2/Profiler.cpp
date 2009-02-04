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
#include <intrin.h>

#pragma intrinsic(__rdtsc)



using namespace ivrworx;
using namespace std;



struct ProfileData
{
	__int64 hits;

	__int64 all;

	__int64 avg;	

	ProfileData()
	{
		hits = 0;
		avg	 = 0;
		all  = 0;
	}

} ;

typedef 
map<std::string,ProfileData*> ProfileMap;

__declspec(thread) ProfileMap *g_ProfileMap;

namespace ivrworx
{
	void 
		InitProfile()
	{
		g_ProfileMap = new ProfileMap();
	}

	void 
		PrintProfile()
	{
		cout << "Statistics for thread id " << ::GetCurrentThreadId() << std::endl;
		cout << "-------------------------------------------------------------------------"<< std::endl;
		cout << std::setw(40)<< std::left << "Profiled Name" << std::setw(10) << "Hits" << std::setw(10) << "Cumulative" << std::endl;
		cout << "-------------------------------------------------------------------------"<< std::endl;

		for (ProfileMap::iterator iter = g_ProfileMap->begin();
			iter != g_ProfileMap->end();
			iter ++)
		{
			ProfileData *data = (*iter).second;
			cout << std::setw(40)<< std::left << (*iter).first.c_str() << std::setw(10) << data->hits << std::setw(10) << data->avg << std::endl;
		}

		cout << "========================================================================="<< std::endl;

	}
}




FuncProfiler::FuncProfiler(string string)
{

	// _start = __rdtsc();
	_start = ::GetTickCount();

	_funcName = string;
}

FuncProfiler::~FuncProfiler(void)
{
//	__int64 lEnd = __rdtsc();
	__int64 lEnd = ::GetTickCount();

	
	ProfileMap::iterator iter = g_ProfileMap->find(_funcName);

	if (iter == g_ProfileMap->end())
	{
		ProfileData *data = new ProfileData();
		data->hits = 1;
		data->all = lEnd - _start;
		data->avg = lEnd - _start;
		(*g_ProfileMap)[_funcName] = data;
	} else
	{
		ProfileData *data = (*iter).second;
		data->hits++;
		data->all += (lEnd - _start);
		data->avg = data->all/(data->hits);
		(*g_ProfileMap)[_funcName] = data;
	}

}
