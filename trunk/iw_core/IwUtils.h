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

#pragma warning( push )
#pragma warning (disable: 4311)
#pragma warning (disable: 4312)
#pragma warning (disable: 4345)
#pragma warning (disable: 4996)
#pragma warning (disable: 4267)
#pragma warning (disable: 4018)
#pragma warning (disable: 4099)

#ifndef _WIN32_WINNT		//Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						


// windows
#include <wchar.h>
#include <WinSock2.h>
#include <Mstcpip.h>
#include <Windows.h>

// console
#include "Console.h"

// std
#include <iostream>
#include <fstream>
#include <strstream>
#include <string>
#include <strsafe.h>
#include <vector>

// boost
#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/variant/variant.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/archive/text_woarchive.hpp>
#include <boost/archive/text_wiarchive.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/static_assert.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp> 
#include <boost/regex.hpp>

// Kent CSP
#include <cppcsp.h>

#pragma warning( pop )

using namespace std;

namespace ivrworx
{

	#define _QUOTE(x) # x
	#define QUOTE(x) _QUOTE(x)
	#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

	#define NOTE( x )  message( x )
	#define FILE_LINE  message( __FILE__LINE__ )

	#define TODO( x )  message( __FILE__LINE__"\n"           \
		" ------------------------------------------------\n" \
		"|  TODO :   " #x "\n" \
		" -------------------------------------------------\n" )

	#define FIXME( x )  message(  __FILE__LINE__"\n"           \
		" ------------------------------------------------\n" \
		"|  FIXME :  " #x "\n" \
		" -------------------------------------------------\n" )

	#define todo( x )  message( __FILE__LINE__" TODO :   " #x "\n" ) 
	#define fixme( x )  message( __FILE__LINE__" FIXME:   " #x "\n" ) 

	#define NAME(x) #x

	#define START_FORKING_REGION { ScopedForking forking
	#define FORK(x) forking.fork( x )
	#define FORK_IN_THIS_THREAD(x) forking.forkInThisThread( x )
	#define END_FORKING_REGION }

	wstring StringToWString(const string& s);

	string WStringToString(const wstring& ws);

	void StringToUpper(string strToConvert);

	void StringToLower(string strToConvert);

	void WStringToUpper(wstring strToConvert);

	void WStringToLower(wstring strToConvert);

	#define  NON_FIBEROUS_THREAD ((PVOID)0x1E00)

	#define DISABLE_SRC_REF
//#define NOLOGS
//#define PROFILE

}