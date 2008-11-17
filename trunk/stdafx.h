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
#pragma warning (disable: 4311)
#pragma warning (disable: 4312)
#pragma warning (disable: 4345)
#pragma warning (disable: 4996)
#pragma warning (disable: 4267)
#pragma warning (disable: 4018)

#include <wchar.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#include <Mq.h>
#include <iostream>
#include <signal.h>
#include <strsafe.h>
#include <fstream>
#include <strstream>

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





// Kent CSP
#include "cppcsp/cppcsp.h"

// ReSip
#include "resip/stack/SdpContents.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/stack/ShutdownMessage.hxx"
#include "resip/stack/SipStack.hxx"
#include "resip/stack/SelectInterruptor.hxx"
#include "resip/stack/InterruptableStackThread.hxx"
#include "resip/stack/StackThread.hxx"
#include "resip/dum/ClientAuthManager.hxx"
#include "resip/dum/ClientInviteSession.hxx"
#include "resip/dum/ClientRegistration.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/DumShutdownHandler.hxx"
#include "resip/dum/InviteSessionHandler.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/RegistrationHandler.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/dum/ServerOutOfDialogReq.hxx"
#include "resip/dum/OutOfDialogHandler.hxx"
#include "resip/dum/AppDialog.hxx"
#include "resip/dum/AppDialogSet.hxx"
#include "resip/dum/AppDialogSetFactory.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "rutil/Random.hxx"
#include "rutil/WinLeakCheck.hxx"
#include "rutil/Data.hxx"
#include "rutil/Subsystem.hxx"

#include "Console.h"

#include "json_spirit.h"
#include "CmdLine.h"



#pragma warning( pop )



