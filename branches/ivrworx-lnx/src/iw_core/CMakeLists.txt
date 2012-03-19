include_directories(${ivrworx_SOURCE_DIR}/iw_core)
link_directories(${ivrworx_BINARY_DIR}/iw_core)

add_library(iw_core ActiveObject.cpp IwBase.cpp  Logger.cpp  Profiler.cpp 
CmdLine.cpp               IwUtils.cpp                LpHandle.cpp          stdafx.cpp  
Configuration.cpp         JSONConfiguration.cpp      Message.cpp           syslog-client.cpp 
ConfigurationFactory.cpp  LightweightProcess.cpp     ProcHandleWaiter.cpp  UIDOwner.cpp 
DllHelpers.cpp            LocalProcessRegistrar.cpp  ProcSipStack.cpp)


