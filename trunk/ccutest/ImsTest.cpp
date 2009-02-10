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
#include "ImsTest.h"
#include "ImsFactory.h"


static CnxInfo g_localAddr;

ImsTest::ImsTest(CnxInfo local_addr)
{
	g_localAddr = local_addr;
}

ImsTest::~ImsTest(void)
{

}

void
ImsTest::test()
{
	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(tester_pair);
	FORK(new ImsTester(tester_pair));

	END_FORKING_REGION;
}

ImsTester::ImsTester(LpHandlePair pair):
LightweightProcess(pair,__FUNCTIONW__)
{

}


void 
ImsTester::real_run()
{
 	I_AM_READY;
 
 	START_FORKING_REGION;

	CcuConfigurationPtr conf = ConfigurationFactory::CreateJsonConfiguration(L"conf.json");
	
  	DECLARE_NAMED_HANDLE_PAIR(tester_pair);
 	FORK(ImsFactory::CreateProcIms(tester_pair, *conf));
	if (CCU_FAILURE(WaitTillReady(Seconds(5),tester_pair)))
	{
		assert(false);
	}

 	TransactionTimeout(600000);
 
 	ImsSession ims_session(*this);
 
 	CcuConnectionId id = IX_UNDEFINED;

 	assert(CCU_SUCCESS(ims_session.AllocateIMSConnection(
		CnxInfo("192.168.100.228",60555), 
		IxCodec(L"pcmu",8000,0))));

	assert(CCU_SUCCESS(ims_session.PlayFile(L"C:\\SOUNDS\\welcome.wav", FALSE)));
 	
 
 	END_FORKING_REGION;	

}

ImsTester::~ImsTester()
{

}