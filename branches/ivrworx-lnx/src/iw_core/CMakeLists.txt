include_directories(. ../kentcsp ../kentcsp/src)
#link_directories(${ivrworx_BINARY_DIR}/iw_core)

set(iw_core_SOURCES ActiveObject.cpp JSONConfiguration.cpp ProcSipStack.cpp
CmdLine.cpp               LightweightProcess.cpp     Profiler.cpp
Configuration.cpp         LocalProcessRegistrar.cpp  StdAfx.cpp
ConfigurationFactory.cpp  Logger.cpp                 syslog-client.cpp
DllHelpers.cpp            LpHandle.cpp               UIDOwner.cpp
IwBase.cpp                Message.cpp
IwUtils.cpp               ProcHandleWaiter.cpp)


add_library(iw_core SHARED ${iw_core_SOURCES})
