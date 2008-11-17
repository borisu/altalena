#include "StdAfx.h"
#include "CcuLogSubsystem.h"

using namespace resip;

const CcuLogSubsystem CcuLogSubsystem::CCU("CCU COMMON   ");

const CcuLogSubsystem CcuLogSubsystem::CCU_SIP_STACK("CCU SIP STACK");

CcuLogSubsystem::CcuLogSubsystem(const char* rhs) 
:Subsystem(rhs) 
{
};