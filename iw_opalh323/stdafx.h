// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0502	// Change this to the appropriate value to target other versions of Windows.
#endif						


// TODO: reference additional headers your program requires here


#include "IwUtils.h"
#include "IwBase.h"
#include "ActiveObject.h"
#include "Logger.h"
#include "Configuration.h"
#include "LightweightProcess.h"
#include "LocalProcessRegistrar.h"
#include "MediaCallSession.h"
#include "Profiler.h"
#include "TelephonyProvider.h"


#pragma warning( push )
#pragma warning (disable: 4995)

#include <ptlib.h>
#include <opal/manager.h>
// #include <opal/ivr.h>
// #include <sip/sipep.h>
#include <h323/h323.h>
#include <h323/h323ep.h>
#include <h323/gkclient.h>

#include <opal/transcoders.h>
#include <lids/lidep.h>
#include <ptclib/pstun.h>
#include <ptlib/config.h>
#include <codec/opalpluginmgr.h>

#pragma warning( pop )

