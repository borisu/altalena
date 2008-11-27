/*
*	The Altalena Project File
*	Copyright (C) 2009  Boris Ouretskey
*
*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "StdAfx.h"
#include "RTPRelayTest.h"
#include "SimpleRTPStreamer.h"
#include "CcuRTPSession.h"


static CcuMediaData g_localAddr;

RTPRelayTest::RTPRelayTest(CcuMediaData local_addr)
{
	g_localAddr = local_addr;

}

RTPRelayTest::~RTPRelayTest(void)
{
}

void
RTPRelayTest::test()
{
	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(pair);
	Run(new ProcRTPTester(pair));

	END_FORKING_REGION
}

ProcRTPTester::ProcRTPTester(LpHandlePair pair)
:LightweightProcess(pair,__FUNCTIONW__)
{

}

void
ProcRTPTester::real_run()
{
	test_early_media();
	test_late_media();
}

void
ProcRTPTester::test_early_media()
{
	FUNCTRACKER;

	START_FORKING_REGION;

	SimpleRTPStreamer streamer;
	
	TransactionTimeout(60000);

	DECLARE_NAMED_HANDLE_PAIR(rtp_pair);
	FORK(RtpRelayFactory::CreateProcRtpRelay(rtp_pair,g_localAddr));
	if (CCU_FAILURE(WaitTillReady(Seconds(5), rtp_pair)))
	{
		return;
	}
	assert(Ping(RTP_RELAY_Q) == CCU_API_SUCCESS);

	//
	// Allocate first connection
	//
	CcuRtpSession session1(*this);
	CcuMediaData remote_end1;

	assert (CCU_SUCCESS(
		session1.AllocateRTPConnection(
				CcuMediaData(g_localAddr.inaddr(), 60010)))) ;


	//
	// Allocate second connection
	//
	CcuRtpSession session2(*this);
	CcuMediaData remote_end2;
	
	assert (CCU_SUCCESS(
		session2.AllocateRTPConnection(	
			CcuMediaData(g_localAddr.inaddr(), 60012))));

	//
	// Bridge Connections
	//
	assert (CCU_SUCCESS(session1.BridgeRTPConnection(session2)));
	

	LogDebug("Started streaming to ip=[" << remote_end1 << "] ms.");
	assert (CCU_SUCCESS(streamer.SyncPlay("chimes.wav", remote_end1)));
	LogDebug("Stopped playing file after [" << streamer.playTime() << "] ms.");

	assert (CCU_SUCCESS(session1.CloseRTPConnection()));
	assert (CCU_SUCCESS(session2.CloseRTPConnection()));
	

	assert (CCU_SUCCESS(Shutdown(Time(Seconds(5)),rtp_pair)));
	

	END_FORKING_REGION;
}

void
ProcRTPTester::test_late_media()
{
	FUNCTRACKER;

	START_FORKING_REGION;

	SimpleRTPStreamer streamer;

	TransactionTimeout(60000);

	DECLARE_NAMED_HANDLE_PAIR(rtp_pair);
	FORK(RtpRelayFactory::CreateProcRtpRelay(rtp_pair,g_localAddr));
	if (CCU_FAILURE(WaitTillReady(Seconds(5), rtp_pair)))
	{
		return;
	}
	assert(Ping(RTP_RELAY_Q) == CCU_API_SUCCESS);

	//
	// Allocate first connection
	//
	CcuRtpSession session1(*this);
	CcuMediaData remote_end1;
	

	// early media
	assert(CCU_SUCCESS(session1.AllocateRTPConnection(remote_end1)));

	//
	// Allocate first connection
	//
	CcuRtpSession session2(*this);
	CcuMediaData remote_end2;

	
	assert(CCU_SUCCESS(session2.AllocateRTPConnection(CcuMediaData(g_localAddr.inaddr(), 60012))));

	//
	// Modify first connection
	//
	assert(CCU_SUCCESS(session1.ModifyRTPConnection(CcuMediaData(g_localAddr.inaddr(), 60010))));
	


	//
	// Bridge Connections
	//
	assert(CCU_SUCCESS(session1.BridgeRTPConnection(session2)));
	


	LogDebug("Started streaming to ip=[" << remote_end1 << "] ms.");
	assert (CCU_SUCCESS(streamer.SyncPlay("chimes.wav", remote_end1)));
	LogDebug("Stopped playing file after [" << streamer.playTime() << "] ms.");

	assert (CCU_SUCCESS(session1.CloseRTPConnection()));
	assert (CCU_SUCCESS(session2.CloseRTPConnection()));


	assert (CCU_SUCCESS(Shutdown(Time(Seconds(5)),rtp_pair)));


	END_FORKING_REGION;

}
