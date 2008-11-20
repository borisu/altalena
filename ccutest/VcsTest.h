#pragma once
#include "LightweightProcess.h"


class VcsTest
{
public:

	VcsTest(CcuMediaData local_addr);

	 void test();

	virtual ~VcsTest(void);

private:

	CcuMediaData _localAddr;
};


class ProcVcsTester:
	public LightweightProcess
{
public:

	ProcVcsTester(IN LpHandlePair pair);

	void real_run();

	void testNormalLifeCycle();

};