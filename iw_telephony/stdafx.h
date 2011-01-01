// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "resip/stack/SdpContents.hxx"

// TODO: reference additional headers your program requires here
#include "IwUtils.h"
#include "Logger.h"
#include "Profiler.h"
#include "LightweightProcess.h"
#include "Configuration.h"
#include "Message.h"
#include "LpHandle.h"
#include "ActiveObject.h"
#include "LocalProcessRegistrar.h"