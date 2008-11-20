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

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						


#include <stdio.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here

#include "CcuUtils.h"

#pragma warning( push )
#pragma warning (disable: 4355)

// mip + rtp 
#include <mipconfig.h>

#if defined(WIN32) || defined(_WIN32_WCE) || defined(MIPCONFIG_SUPPORT_OSS)
#include <mipcomponentchain.h>
#include <mipcomponent.h>
#include <miptime.h>
#include <mipaveragetimer.h>
#include <mipwavinput.h>
#include <mipsamplingrateconverter.h>
#include <mipsampleencoder.h>
#include <mipwinmmoutput.h>
#endif 

#include <mipulawencoder.h>
#include <miprtpulawencoder.h>
#include <miprtpcomponent.h>
#include <miprtpdecoder.h>
#include <miprtpulawdecoder.h>
#include <mipulawdecoder.h>
#include <mipaudiomixer.h>
#include <miprawaudiomessage.h> // Needed for MIPRAWAUDIOMESSAGE_TYPE_S16LE etc
#include <rtpsourcedata.h>
#include <rtppacket.h>
#include <rtpsession.h>
#include <rtpsessionparams.h>
#include <rtpipv4address.h>
#include <rtpudpv4transmitter.h>
#include <rtperrors.h>

#pragma warning( pop )

#include <stdio.h>
#include <iostream>
#include <string>



