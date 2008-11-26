#include "StdAfx.h"
#include "CallWithRtpManagmentTest.h"
#include "ProcStackStub.h"
#include "ProcRtpRelayStub.h"
#include "ProcImsStub.h"
#include "CallWithRTPManagment.h"


CallWithRtpManagmentTest::CallWithRtpManagmentTest(void)
{
}

CallWithRtpManagmentTest::~CallWithRtpManagmentTest(void)
{
}

void
CallWithRtpManagmentTest::test()
{
	CallWithRtpTester *tester = 
		new CallWithRtpTester();

	RunInThisThread(tester);
}

CallWithRtpTester::CallWithRtpTester()
:LightweightProcess(HANDLE_PAIR)
{

}

CallWithRtpTester::~CallWithRtpTester()
{

}

void
CallWithRtpTester::real_run()
{
	
	START_FORKING_REGION;
	DECLARE_NAMED_HANDLE_PAIR(rtp_stub_pair);
	FORK(new ProcRTPRelayStub(rtp_stub_pair));

	DECLARE_NAMED_HANDLE_PAIR(stack_stub_pair);
	FORK(new ProcStackStub(stack_stub_pair));

	DECLARE_NAMED_HANDLE_PAIR(ims_stub_pair);
	FORK(new ProcImsStub(ims_stub_pair));


	// test make call
	CallWithRTPManagment call(stack_stub_pair,*this);
	assert(CCU_SUCCESS(call.MakeCall(L"xxx@example.com")));

	assert(call.RemoteMedia() == 
		CcuMediaData(DUMMY_STACK_REMOTE_IP, (int)DUMMY_STACK_REMOTE_PORT));

	assert(call.LocalMedia() == 
		CcuMediaData(DUMMY_RTP_ADDRESS, (int)DUMMY_RTP_PORT));

	assert(CCU_SUCCESS(call.PlayFile(L"dummy.wav")));

	
	Shutdown(Seconds(5),stack_stub_pair);
	Shutdown(Seconds(5),rtp_stub_pair);
	Shutdown(Seconds(5),ims_stub_pair);

	END_FORKING_REGION

}
