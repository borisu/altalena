// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

// TODO: reference additional headers your program requires here



#include <ortp\ortp.h>
#include <ortp\payloadtype.h>
#include <mediastreamer2\mediastream.h>
#include <mediastreamer2\msrtp.h>
#include <mediastreamer2\msfileplayer.h>
#include <mediastreamer2\dtmfgen.h>
#include <ortp\telephonyevents.h>

#include "IwUtils.h"
#include "IwBase.h"
#include "ActiveObject.h"
#include "Logger.h"
#include "Configuration.h"
#include "LightweightProcess.h"
#include "LocalProcessRegistrar.h"
#include "Profiler.h"






#pragma warning( pop )


