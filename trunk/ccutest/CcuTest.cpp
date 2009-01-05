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

// CcuTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CcuLogger.h"
#include "CcuMessage.h"
#include "Ccu.h"

#include "ProcPipeIPCDispatcherTest.h"
#include "RTPRelayTest.h"
#include "VcsTest.h"
#include "ImsTest.h"
#include "CmdLine.h"
#include "CcuLpTest.h"
#include "JsonConfigurationTest.h"
#include "ImsFactory.h"
#include "VcsFactory.h"
#include "RtpRelay.h"
#include "ConfigurationFactory.h"
#include "LpHandleTest.h"
#include "CallWithRtpManagmentTest.h"
#include "SimpleRTPStreamer.h"


#pragma TODO("Outsource system starter to a different file")


void
start_test_system();


using namespace JadedHoboConsole;
namespace con = JadedHoboConsole;




int _tmain(int argc, _TCHAR* argv[])
{
	
	CcuSetLogLevel(CCU_LOG_LEVEL_DEBUG);
	CcuSetLogMask(CCU_LOG_MASK_CONSOLE|CCU_LOG_MASK_DEBUGVIEW);
	LogInfo(">>>>>> START TEST <<<<<<");

	Start_CPPCSP();


	// our cmd line parser object
	CCmdLine cmdLine;

	if (cmdLine.SplitLine(argc, argv) < 1)
	{
		// no switches were given on the command line, abort
		LogWarn("ERROR: Missing mandatory options.");
		_ASSERT(0);
		return -1;
	}
	

	
	if ( cmdLine.HasSwitch(L"-ip") == FALSE)
	{
		LogWarn("ERROR: Must supply -ip (local ip) switch");
		return -1;
	}

	wstring ip  = cmdLine.GetArgument(L"-ip",0);
	if ( cmdLine.HasSwitch(L"-p") == FALSE)
	{
		LogWarn("ERROR: Must supply -p (port) switch");
		return -1;
	}
	wstring port_str  = cmdLine.GetArgument(L"-p",0);
	int port = _wtoi(port_str.c_str());




	//////////////////////////////////////////////////////////////////////////
	//
	// TESTS
	//
	//////////////////////////////////////////////////////////////////////////

	start_test_system();

	return 0;


// 	SimpleRTPStreamer r;
// 	r.SyncPlay("C:\\SOLUTIONS\\altalena\\ccutest\\sounds\\welcome.wav", 
// 		CcuMediaData("192.168.150.3",6555));
// 
// 	return 0;
	
	

	


	CallWithRtpManagmentTest().test();
	ProcPipeIPCDispatcherTest().test();
	RTPRelayTest(CnxInfo(ip,port)).test();
	VcsTest(CnxInfo(ip,port)).test();
	ImsTest(CnxInfo(ip,port)).test();
	CcuLightweightProcessTest().test();
	JsonConfigurationTest().test();
	LpHandleTest().test();
	
	End_CPPCSP();
	
}


class ProcSystemStarter: 
	public LightweightProcess
{
public:
	ProcSystemStarter(LpHandlePair pair)
		:LightweightProcess(pair,__FUNCTIONW__)
	{

	}

	void real_run()
	{
		START_FORKING_REGION;

		CcuConfigurationPtr conf = 
			ConfigurationFactory::CreateJsonConfiguration(L"conf.json");
		
		//
		// Start RTP relay
		//
		DECLARE_NAMED_HANDLE_PAIR(rtp_pair);
		FORK(RtpRelayFactory::CreateProcRtpRelay(rtp_pair, conf.get()));
		assert(CCU_SUCCESS(WaitTillReady(Seconds(5), rtp_pair)));
		assert(CCU_SUCCESS(Ping(RTP_RELAY_Q)));

		//
		// Start IMS 
		//
		DECLARE_NAMED_HANDLE_PAIR(ims_pair);
		FORK(ImsFactory::CreateProcIms(ims_pair, conf->DefaultCnxInfo()));
		assert(CCU_SUCCESS(WaitTillReady(Seconds(5), ims_pair)));
		assert(CCU_SUCCESS(Ping(IMS_Q)));

		//
		// Start VCS
		//
		CnxInfo vcs_media = CnxInfo(
			conf->DefaultCnxInfo().inaddr(),5060);
		DECLARE_NAMED_HANDLE_PAIR(vcs_pair);
		FORK(VcsFactory::CreateProcVcs(vcs_pair,*conf));
		assert(CCU_SUCCESS(WaitTillReady(Seconds(5), vcs_pair)));
		assert(CCU_SUCCESS(Ping(VCS_Q)));

		END_FORKING_REGION;


	}

};

void
start_test_system()
{
	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(starter_pair);
	FORK(new ProcSystemStarter(starter_pair));

	END_FORKING_REGION;

}
