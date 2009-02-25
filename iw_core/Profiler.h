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

#ifdef PROFILE

	#pragma message ("+----------------------------+")
	#pragma message ("| Compiling with profiler on |")
	#pragma message ("+----------------------------+")

	#define IX_PROFILE_FUNCTION() ivrworx::FuncProfiler _dummyIxProfiler(__FUNCTION__)
	#define IX_PROFILE_PRINT() ivrworx::PrintProfile()
	#define IX_PROFILE_CHECK_INTERVAL(x) ivrworx::CheckInterval(x)
	#define IX_PROFILE_ADD_DATA(x,y) ivrworx::AddProfilingData(x,y)
	#define IX_PROFILE_CODE( code )												\
		{																		\
			{ ivrworx::FuncProfiler _dummyIxProfiler(#code);										\
			code; }															\
		}
#else 

	#define IX_PROFILE_FUNCTION() 
	#define IX_PROFILE_PRINT()
	#define IX_PROFILE_CODE( code )	code
	#define IX_PROFILE_ADD_DATA(x,y) 
	#define IX_PROFILE_CHECK_INTERVAL(x);

#endif

namespace ivrworx
{
	
	void InitProfile();

	void PrintProfile();

	void CheckInterval(__int64 interval);

	void AddProfilingData(string name, LARGE_INTEGER start, LARGE_INTEGER end);

	void AddProfilingData(string name, LARGE_INTEGER start);

	class FuncProfiler
	{
	public:

		FuncProfiler(string function);
		virtual ~FuncProfiler(void);

	private:

		LARGE_INTEGER _start;

		string _funcName;
	};

	

}

