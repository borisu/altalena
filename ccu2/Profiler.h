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
#define IX_PROFILE_FUNCTION() ivrworx::FuncProfiler(__FUNCTION__)
#define IX_PROFILE_INIT() ivrworx::InitProfile()
#define IX_PROFILE_PRINT() ivrworx::PrintProfile()
#define IX_PROFILE_CODE( code )												\
	{																		\
		{ ivrworx::FuncProfiler(#code);										\
		 code; }															\
	}
#else 
#define IX_PROFILE_FUNCTION() 
#define IX_PROFILE_INIT() 
#define IX_PROFILE_PRINT()
#define IX_PROFILE_CODE( code )	code
#endif

namespace ivrworx
{
	
	void InitProfile();

	void PrintProfile();

	class FuncProfiler
	{
	public:

		FuncProfiler(string function);
		virtual ~FuncProfiler(void);

	private:

		__int64 _start;

		string _funcName;
	};

	

}

