include_directories(../json_spirit/json_spirit_v4.05/json_spirit)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -iquote${PROJECT_SOURCE_DIR}/src/kentcsp -iquote${PROJECT_SOURCE_DIR}/src/kentcsp/src")


set(iw_core_SOURCES DllHelpers.cpp ActiveObject.cpp JSONConfiguration.cpp ProcSipStack.cpp
CmdLine.cpp               LightweightProcess.cpp     Profiler.cpp
Configuration.cpp         LocalProcessRegistrar.cpp  StdAfx.cpp
ConfigurationFactory.cpp  Logger.cpp                 syslog-client.cpp
DllHelpers.cpp            LpHandle.cpp               UIDOwner.cpp
IwBase.cpp                Message.cpp
IwUtils.cpp               ProcHandleWaiter.cpp)


add_library(iw_core SHARED ${iw_core_SOURCES})
