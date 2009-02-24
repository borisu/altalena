#pragma once

using namespace resip;

class CcuLogSubsystem:
	public Subsystem
{
public:
	// Add new systems below
	static const CcuLogSubsystem CCU;
	static const CcuLogSubsystem CCU_SIP_STACK;
private:
	explicit CcuLogSubsystem(const char* rhs);
	explicit CcuLogSubsystem(const resip::Data& rhs);
	CcuLogSubsystem& operator=(const resip::Data& rhs);
};
