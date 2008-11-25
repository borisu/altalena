#pragma once
#include "Ccu.h"
#include "LightweightProcess.h"
#include "RtpRelay.h"

class CallWithRtpManagmentTest
{
public:
	CallWithRtpManagmentTest(void);
	virtual ~CallWithRtpManagmentTest(void);

	virtual void test();
};


class CallWithRtpTester:
	public LightweightProcess
{
public: 
	CallWithRtpTester();
	virtual ~CallWithRtpTester();

	void real_run();
};
	