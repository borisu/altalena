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

#include "CcuUtils.h"
#include "CcuLogger.h"
#include "CcuConfiguration.h"
#include "LightweightProcess.h"
#include "ProcPipeIPCDispatcher.h"
#include "LocalProcessRegistrar.h"

#include "PortManager.h"

//
// RTP library
//
// #include <mipconfig.h>
// #include <mipcomponentchain.h>
// #include <mipcomponent.h>
// #include <miptime.h>
// #include <mipaveragetimer.h>
// #include <mipwavinput.h>
// #include <mipsamplingrateconverter.h>
// #include <mipsampleencoder.h>
// #ifndef WIN32
// #include <mipossinputoutput.h>
// #else
// #include <mipwinmmoutput.h>
// #endif 
// #include <mipulawencoder.h>
// #include <miprtpulawencoder.h>
// #include <miprtpcomponent.h>
// #include <miprtpdecoder.h>
// #include <miprtpulawdecoder.h>
// #include <mipulawdecoder.h>
// #include <mipaudiomixer.h>
// #include <miprawaudiomessage.h> // Needed for MIPRAWAUDIOMESSAGE_TYPE_S16LE etc
// #include <rtpsession.h>
// #include <rtpsessionparams.h>
// #include <rtpipv4address.h>
// #include <rtpudpv4transmitter.h>
// #include <rtperrors.h>
// #include <rtpsourcedata.h>



#pragma warning( pop )


