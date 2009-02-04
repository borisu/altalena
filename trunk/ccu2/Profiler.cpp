#include "StdAfx.h"
#include "Profiler.h"

using namespace ivrworx;
using namespace std;

inline unsigned __int64 RDTSC(void)
{
	_asm _emit 0x0F
	_asm _emit 0x31
}

struct ProfileData
{
	__int64 hits;

	__int64 all;

	__int64 avg;	

	ProfileData()
	{
		hits = 0;
		avg	 = 0;
	}

} ;

typedef 
map<std::wstring,ProfileData*> ProfileMap;

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
		wcout << L"Statistics for thread id " << ::GetCurrentThreadId() << std::endl;
		wcout << L"-------------------------------------------------------------"<< std::endl;
		wcout << L"name\t\thits\t\tavg" << std::endl;

		for (ProfileMap::iterator iter = g_ProfileMap->begin();
			iter != g_ProfileMap->end();
			iter ++)
		{
			ProfileData *data = (*iter).second;
			wcout << std:: setw(19) << (*iter).first << "\t\t" << data->hits << "\t\t" << data->avg << std::endl;
		}

	}
}




FuncProfiler::FuncProfiler(wstring string)
{

	_start = RDTSC();;
	_funcName = string;
}

FuncProfiler::~FuncProfiler(void)
{
	__int64 lEnd = RDTSC();

	
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
