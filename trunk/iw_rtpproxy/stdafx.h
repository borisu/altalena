// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

// windows
#include <wchar.h>
#include <WinSock2.h>
#include <Mstcpip.h>
#include <Windows.h>

#include <liveMedia.hh>
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"


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

#include "IwUtils.h"
#include "IwBase.h"
#include "LightweightProcess.h"
#include "Logger.h"

#include "Message.h"

#pragma warning( pop )



