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



using namespace boost::assign;

using namespace JadedHoboConsole;
namespace con = JadedHoboConsole;

#define IW_WRONG_NUMBER_OF_ARGUMENTS -1
#define IW_ERROR_PARSING_CONF		 -2


namespace ivrworx
{
	class ProcSystemStarter: 
		public LightweightProcess
	{
	public:
		ProcSystemStarter(LpHandlePair pair,Configuration &conf)
			:LightweightProcess(pair,"SystemStarter"),
			_conf(conf)
		{

		}

		void real_run()
		{
			START_FORKING_REGION;

			//
			// Start Waiter
			//
//			DECLARE_NAMED_HANDLE_PAIR(waiter_pair);
//
// 			DECLARE_NAMED_HANDLE(waiter_shutdown_handle);
// 			AddShutdownListener(waiter_pair,waiter_shutdown_handle);
// 
// 			FORK(new ProcHandleWaiter(waiter_pair));
// 			if (IW_FAILURE(WaitTillReady(Seconds(15), waiter_pair)))
// 			{
// 				LogCrit("Cannot start Waiter process.");
// 				return;
// 			};

	
			//
			// Start IMS 
			//
			DECLARE_NAMED_HANDLE_PAIR(ims_pair);

			DECLARE_NAMED_HANDLE(ims_shutdown_handle);
			AddShutdownListener(ims_pair,ims_shutdown_handle);

			FORK(ImsFactory::CreateProcIms(ims_pair, _conf));
			if (IW_FAILURE(WaitTillReady(Seconds(15), ims_pair)))
			{
				LogCrit("Cannot start Ims process.");
				return;
			};


			//
			// Start IVR
			//
			DECLARE_NAMED_HANDLE_PAIR(ivr_pair);

			DECLARE_NAMED_HANDLE(ivr_shutdown_handle);
			AddShutdownListener(ivr_pair, ivr_shutdown_handle);

			FORK(IvrFactory::CreateProcIvr(ivr_pair,_conf));
			if (IW_FAILURE(WaitTillReady(Seconds(15), ivr_pair)))
			{
				LogCrit("Cannot start Ivr process.");
				Shutdown(Seconds(5),ims_pair);
				return;
			}

			I_AM_READY;

			while(true)
			{
				HandlesList selected_handles =
					list_of(_inbound)(ivr_shutdown_handle)(ims_shutdown_handle);

				IwMessagePtr msg;
				int index = 0;

				ApiErrorCode code = 
					SelectFromChannels(selected_handles,Seconds(60),index,msg);

				if (code == API_TIMEOUT)
				{
					ApiErrorCode ping_res = Ping(ivr_pair);
					if (IW_FAILURE(ping_res))
					{
						LogWarn("Ivr process did not respond in timely fashion to keep alive request, consider restarting the application.")
					}
					ping_res = Ping(ims_pair);
					if (IW_FAILURE(ping_res))
					{
						LogWarn("Ims process did not respond in timely fashion to keep alive request, consider restarting the application.")
					}

					continue;
				}

				switch (index)
				{
				case 1:
					{
						LogWarn("Ivr process terminated unexpectedly. Shutting down.")
						Shutdown(Seconds(5),ims_pair);
						return;
					}
				case 2:
					{
						LogWarn("Ims process terminated. Shutting down.")
						Shutdown(Seconds(5),ivr_pair);
						return;
					}
				default:
					{
						LogCrit("Selected unknown index");
						throw;
					}
				}
			}
			
			END_FORKING_REGION;

		}

	private:

		Configuration &_conf;

	};

}

using namespace ivrworx;

int _tmain(int argc, _TCHAR* argv[])
{

 
	wstring conf_file;

	if (argc < 2)
	{
		cerr << "Usage: ivrworx.exe [configuration_file]" << std::endl;
		cerr << "\tStarts lightweight IVR application." << std::endl;
		cerr << "Trying default conf.json file...";

		conf_file = L"conf.json";

	} 
	else
	{
		conf_file = argv[1];

	}

	

	
	ApiErrorCode err_code = API_SUCCESS;
	ConfigurationPtr conf = 
		ConfigurationFactory::CreateJsonConfiguration(WStringToString(conf_file),err_code);
	if (IW_FAILURE(err_code))
	{
		return IW_ERROR_PARSING_CONF;
	}
	
	
	InitLog(*conf);
	LogInfo(">>>>>> IVRWORX START <<<<<<");

	Start_CPPCSP();

	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(starter_pair);
	FORK(new ProcSystemStarter(starter_pair, *conf));
	END_FORKING_REGION;

 	End_CPPCSP();

	ExitLog();
	return 0;
	
}


