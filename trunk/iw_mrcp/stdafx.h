// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "IwUtils.h"
#include "IwBase.h"
#include "ActiveObject.h"
#include "Logger.h"
#include "Configuration.h"
#include "LightweightProcess.h"
#include "LocalProcessRegistrar.h"
#include "Profiler.h"




#include "mrcp.h"
#include "unimrcp_client.h"
#include "mrcp_application.h"
#include "mrcp_session.h"
#include <apr_getopt.h>
#include <apr_file_info.h>
#include "apt_pool.h"
#include "apt_log.h"
#include "mrcp_message.h"
#include "mrcp_client_session.h"
#include "mrcp_synth_resource.h"
#include "mrcp_generic_header.h"
#include "mrcp_synth_header.h"





// TODO: reference additional headers your program requires here
