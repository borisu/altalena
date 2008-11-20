#pragma once
#include "LightweightProcess.h"



class RTPRelayTest
{
public:
	RTPRelayTest(CcuMediaData local_addr);
	~RTPRelayTest(void);

	void test();
};

class ProcRTPTester:
	public LightweightProcess
{
	
public:

	ProcRTPTester(LpHandlePair pair);

	void real_run();

	void test_early_media();

	void test_late_media();

};
