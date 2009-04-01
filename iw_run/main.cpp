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

#define IW_WRONG_NUMBER_OF_ARGUMENTS -1
#define IW_ERROR_PARSING_CONF		 -2


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

			LocalProcessRegistrar::Instance().AddShutdownListener(
				ims_pair.inbound->GetObjectUid(), 
				_inbound);

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
			FORK(IvrFactory::CreateProcIvr(ivr_pair,_conf));
			if (IW_FAILURE(WaitTillReady(Seconds(15), ivr_pair)))
			{
				LogCrit("Cannot start Ivr process.");
				Shutdown(Seconds(5),ims_pair);
				return;
			}

			while(true)
			{
				ApiErrorCode code = API_SUCCESS;
				IwMessagePtr msg = _inbound->Wait(Seconds(60),code);

				switch (msg->message_id)
				{
				case MSG_PROC_SHUTDOWN_EVT:
					{
						// it can only be IMS stop IVR 
						Shutdown(Seconds(5),ivr_pair);
					}
				}
			}
			

			I_AM_READY;
	
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
		cerr << "Usage: ivrworx.exe [configuration_file]" << std::endl;
		cerr << "Starts lightweight IVR application." << std::endl;
		return IW_WRONG_NUMBER_OF_ARGUMENTS;
	}

	wstring conf_file(argv[1]);

	
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
	return 0;
	
}


