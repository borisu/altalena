#pragma once
#include "LightweightProcess.h"


class VcsTest
{
public:

	VcsTest(CnxInfo local_addr);

	 void test();

	virtual ~VcsTest(void);

private:

	CnxInfo _localAddr;
};


class ProcVcsTester:
	public LightweightProcess
{
public:

	ProcVcsTester(IN LpHandlePair pair);

	void real_run();

	void testNormalLifeCycle();

};