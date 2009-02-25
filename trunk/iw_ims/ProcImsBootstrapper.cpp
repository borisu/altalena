#include "StdAfx.h"
#include "ProcImsBootstrapper.h"
#include "ProcIMS.h"

ProcImsBootstrapper::ProcImsBootstrapper(LpHandlePair pair)
:ProcCcuFacade(pair,__FUNCTIONW__),
{

}



ProcImsBootstrapper::~ProcImsBootstrapper(void)
{
}


void 
ProcImsBootstrapper::real_run()
{
	START_FORKING_REGION;

	HANDLE_PAIR_DECLARE_(ims_pair);
	
	FORK(new ProcIMS(ims_pair));
	if (CCU_FAILURE(WaitTillReady(Seconds(5), ims_pair)))
	{
		return;
	}


	_outbound->Send(new CcuMsgProcReady());


	BOOL shutdown_flag = FALSE;
	while(shutdown_flag == FALSE)
	{
		CcuApiErrorCode err_code = CCU_API_SUCCESS; 
		CcuMsgPtr ptr = GetInboundMessage();
		switch (ptr->message_id)
		{
		case CCU_MSG_PROC_SHUTDOWN_REQUEST:
			{
				err_code = Shutdown(Time(Seconds(5)),ims_pair);
				SendResponse(ptr, new CcuMsgShutdownSuccess());
				shutdown_flag = TRUE;
				continue;
			}
		default:
			{
				LogWarn("Unknown message received id=[" << ptr->message_id << "]");
				_ASSERT(false);
			}
		}
	}

	END_FORKING_REGION;

}


