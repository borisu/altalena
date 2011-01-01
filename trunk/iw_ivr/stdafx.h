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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		//Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include "IwUtils.h"
#include "Logger.h"
#include "Profiler.h"
#include "LightweightProcess.h"
#include "LocalProcessRegistrar.h"
#include "ActiveObject.h"
#include "Configuration.h"
#include "StreamingSession.h"
#include "SipCall.h"
#include "RtpProxySession.h"
#include "MrcpSession.h"


 // ("MUST REDUCE EXPLICIT PROVIDER DEPENDENCIES")


#include "SqliteSession.h"
#include "RtspSession.h"
#include "Live555RtspSession.h"


extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luasql.h"
#include "sqlite3.h"
}

