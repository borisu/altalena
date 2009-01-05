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
#include "VcsTest.h"
#include "VcsFactory.h"
#include "ImsFactory.h"
#include "RtpRelay.h"

static CnxInfo g_localAddr;

VcsTest::VcsTest(CnxInfo local_addr)
{
	g_localAddr = local_addr;
}

VcsTest::~VcsTest(void)
{
}

void
VcsTest::test()
{

	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(pair);
	Run(new ProcVcsTester(pair));

	END_FORKING_REGION

}

ProcVcsTester::ProcVcsTester(LpHandlePair pair):
LightweightProcess(pair,__FUNCTIONW__)
{

}

void
ProcVcsTester::real_run()
{

	START_FORKING_REGION;

	//
	// to make debugging easy
	//
	TransactionTimeout(6000);

	//
	// Start RTP relay
	//
	DECLARE_NAMED_HANDLE_PAIR(rtp_pair);
	FORK(RtpRelayFactory::CreateProcRtpRelay(rtp_pair, g_localAddr));
	assert(CCU_SUCCESS(WaitTillReady(Seconds(5), rtp_pair)));
	assert(CCU_SUCCESS(Ping(RTP_RELAY_Q)));

	//
	// Start IMS 
	//
	DECLARE_NAMED_HANDLE_PAIR(ims_pair);
	FORK(ImsFactory::CreateProcIms(ims_pair, g_localAddr));
	assert(CCU_SUCCESS(WaitTillReady(Seconds(5), ims_pair)));
	assert(CCU_SUCCESS(Ping(IMS_Q)));


 	//
 	// Start VCS
 	//
 	CnxInfo vcs_media = CnxInfo(g_localAddr.inaddr(),5060);
 	DECLARE_NAMED_HANDLE_PAIR(vcs_pair);
 	FORK(VcsFactory::CreateProcVcs(vcs_pair,vcs_media));
 	assert(CCU_SUCCESS(WaitTillReady(Seconds(5), vcs_pair)));
 	assert(CCU_SUCCESS(Ping(VCS_Q)));
 
 
// 	//
// 	// Start client sip stack
// 	//
// 	DECLARE_HANDLE_PAIR(client_stack_pair);
// 	FORK(new ProcSipStack(
// 		client_stack_pair, 
// 		CcuMediaData(g_localAddr.ip_addr,5065)));
// 	assert(CCU_SUCCESS(WaitTillReady(Seconds(500),client_stack_pair)));
// 	assert(CCU_SUCCESS(Ping(client_stack_pair)));
// 
// 	//////////////////////////////////
// 	//
// 	// make call from client to VCS
// 	// 
// 	///////////////////////////////////
// 
// 	//
// 	// firstly allocate connection
// 	//
// 	CcuMediaData client_media;
// 	CcuConnectionId client_conn_id = CCU_UNDEFINED;
// 	assert(CCU_SUCCESS(AllocateRTPConnection(client_media, client_conn_id)));
// 	
// 
// 	
// 	//
// 	// issue make call request
// 	// 
// 	wstring vcs_sip_addr = L"sip:1234@" + StringToWString(vcs_media.ipporttos());
// 	ScopedRTPConnectionGuard conn_guard(this, client_conn_id);
// 	int client_call_handle = CCU_UNDEFINED;
// 	assert(
// 		CCU_SUCCESS(MakeCall(
// 		client_stack_pair,
// 		vcs_sip_addr,
// 		client_media,
// 		client_call_handle)));
// 	
// 	::Sleep(4000);
// 
// 	assert(
// 		CCU_SUCCESS(HagupCall(
// 		client_stack_pair,
// 		client_call_handle)));
// 
// 	::Sleep(INFINITE);


	END_FORKING_REGION;

}

