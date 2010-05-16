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
#include "IwUtils.h"
#include "IwBase.h"
#include "Logger.h"
#include "Message.h"
#include "LpHandle.h"
#include "ActiveObject.h"

#include "TelephonyProvider.h"
#include "MediaCallSession.h"


#include <ws2tcpip.h>
#include <stdarg.h>

// resiprocate
#include "resip/stack/SdpContents.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/stack/ShutdownMessage.hxx"
#include "resip/stack/SipStack.hxx"
#include "resip/stack/SelectInterruptor.hxx"
#include "resip/stack/InterruptableStackThread.hxx"
#include "resip/stack/StackThread.hxx"
#include "resip/stack/Contents.hxx"
#include "resip/dum/ClientAuthManager.hxx"
#include "resip/dum/ClientInviteSession.hxx"
#include "resip/dum/ClientRegistration.hxx"
#include "resip/dum/ClientSubscription.hxx"
#include "resip/dum/SubscriptionHandler.hxx"
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
#include "resip/dum/DumThread.hxx"
#include "resip/dum/BaseCreator.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "rutil/Random.hxx"
#include "rutil/WinLeakCheck.hxx"
#include "rutil/Data.hxx"
#include "rutil/Subsystem.hxx"


#pragma warning( pop )



