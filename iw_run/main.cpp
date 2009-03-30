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
		ProcSystemStarter(LpHandlePair pair,Configuration &conf)
			:LightweightProcess(pair,__FUNCTION__),
			_conf(conf)
		{

		}

		void real_run()
		{
			START_FORKING_REGION;

			//
			// Start IMS 
			//
			DECLARE_NAMED_HANDLE_PAIR(ims_pair);
			FORK(ImsFactory::CreateProcIms(ims_pair, _conf));
			assert(IW_SUCCESS(WaitTillReady(Seconds(5), ims_pair)));
			assert(IW_SUCCESS(Ping(IMS_Q)));

			//
			// Start IVR
			//
			CnxInfo vcs_media = CnxInfo(
				_conf.IvrCnxInfo().inaddr(),5060);
			DECLARE_NAMED_HANDLE_PAIR(ivr_pair);
			FORK(IvrFactory::CreateProcIvr(ivr_pair,_conf));
			assert(IW_SUCCESS(WaitTillReady(Seconds(5), ivr_pair)));
			assert(IW_SUCCESS(Ping(IVR_Q)));


			END_FORKING_REGION;


		}

	private:

		Configuration &_conf;

	};

}

using namespace ivrworx;


int _tmain(int argc, _TCHAR* argv[])
{

	if (argc < 2)
	{
		std::cerr << "Usage: ivrworx.exe [configuration file]" << std::endl;
		return -1;
	}

	wstring conf_file(argv[1]);

	ConfigurationPtr conf = 
		ConfigurationFactory::CreateJsonConfiguration(WStringToString(conf_file));

	InitLog(*conf);

	SetLogLevel(LOG_LEVEL_DEBUG);
	SetLogMask(IX_LOG_MASK_CONSOLE|IX_LOG_MASK_DEBUGVIEW|IX_LOG_MASK_SYSLOG);

	LogInfo(">>>>>> IVRWORX START <<<<<<");

	Start_CPPCSP();

	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(starter_pair);
	FORK(new ProcSystemStarter(starter_pair,*conf));

	END_FORKING_REGION;


 	End_CPPCSP();

	return 0;
	
}


