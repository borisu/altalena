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

using namespace JadedHoboConsole;
namespace con = JadedHoboConsole;

namespace ivrworx
{
	class ProcSystemStarter: 
		public LightweightProcess
	{
	public:
		ProcSystemStarter(LpHandlePair pair)
			:LightweightProcess(pair,__FUNCTION__)
		{

		}

		void real_run()
		{

			ConfigurationPtr conf = 
				ConfigurationFactory::CreateJsonConfiguration("conf.json");

			START_FORKING_REGION;

			//
			// Start IMS 
			//
			DECLARE_NAMED_HANDLE_PAIR(ims_pair);
			FORK(ImsFactory::CreateProcIms(ims_pair, *conf));
			assert(IW_SUCCESS(WaitTillReady(Seconds(5), ims_pair)));
			assert(IW_SUCCESS(Ping(IMS_Q)));

			//
			// Start IVR
			//
			CnxInfo vcs_media = CnxInfo(
				conf->DefaultCnxInfo().inaddr(),5060);
			DECLARE_NAMED_HANDLE_PAIR(ivr_pair);
			FORK(IvrFactory::CreateProcIvr(ivr_pair,*conf));
			assert(IW_SUCCESS(WaitTillReady(Seconds(5), ivr_pair)));
			assert(IW_SUCCESS(Ping(IVR_Q)));


			END_FORKING_REGION;


		}

	};

}

using namespace ivrworx;


int _tmain(int argc, _TCHAR* argv[])
{

	SetLogLevel(LOG_LEVEL_TRACE);
	SetLogMask(IX_LOG_MASK_CONSOLE|IX_LOG_MASK_DEBUGVIEW);
	LogInfo(">>>>>> IVRWORX START <<<<<<");

	Start_CPPCSP();

	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(starter_pair);
	FORK(new ProcSystemStarter(starter_pair));

	END_FORKING_REGION;


 	End_CPPCSP();

	return 0;
	
}


