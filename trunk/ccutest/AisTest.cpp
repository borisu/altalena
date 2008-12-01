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

#include "stdafx.h"
#include "AisTest.h"
#include "ProcAIS.h"

static CnxInfo g_localAddr;

AisTest::AisTest(const CnxInfo &local_addr)
{
	g_localAddr = local_addr;
}

AisTest::~AisTest(void)
{

}

void
AisTest::test()
{
	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(tester_pair);
	FORK(new AisTester(tester_pair));

	END_FORKING_REGION;
}

AisTester::AisTester(LpHandlePair pair):
LightweightProcess(pair,__FUNCTIONW__)
{

}


void 
AisTester::real_run()
{
	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(ais_pair);
	FORK(new ProcAis(ais_pair));

	if (CCU_FAILURE(WaitTillReady(Seconds(5),ais_pair)))
	{
		throw;
	}

	testLoginLogout();
	testMatch();
	

	assert(Shutdown(Seconds(5),ais_pair) == CCU_API_SUCCESS);


	END_FORKING_REGION;	

}

void 
AisTester::testLoginLogout()
{
	AgentInfo info1;
	info1.media_address = g_localAddr;
	info1.name = L"agent1";
	info1.state = CCU_AGENT_STATE_AVAILABLE;


	// check login logout
	AisSession aisSession(*this);
	assert(aisSession.Login(info1) == CCU_API_SUCCESS);
	// login twice should fail
	assert(aisSession.Login(info1) != CCU_API_SUCCESS);

	assert(aisSession.Logout(info1.name) == CCU_API_SUCCESS);
	// no problem to logout twice
	assert(aisSession.Logout(info1.name) == CCU_API_SUCCESS);
}

void 
AisTester::testMatch()
{
	AgentInfo info1;
	info1.media_address = g_localAddr;
	info1.name = L"agent1";
	info1.state = CCU_AGENT_STATE_AVAILABLE;


	// check login logout
	AisSession aisSession(*this);
	assert(aisSession.Login(info1) == CCU_API_SUCCESS);

	AgentInfo allocated_info;
	assert(aisSession.AllocateAgent(allocated_info,Seconds(5)) == CCU_API_SUCCESS);
	assert(allocated_info.name == info1.name);

	assert(aisSession.Logout(info1.name) == CCU_API_SUCCESS);
	assert(aisSession.AllocateAgent(allocated_info,Seconds(5)) == CCU_API_TIMEOUT);

}

AisTester::~AisTester()
{

}