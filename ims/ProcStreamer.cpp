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
#include "ProcStreamer.h"
#include "CcuLogger.h"


#define CCU_DEFAULT_POLLING_TIME_MS 20

ProcStreamer::ProcStreamer(LpHandlePair pair, LpHandlePtr ims_inbound):
LightweightProcess(pair, __FUNCTIONW__),
_imsInbound(ims_inbound)
{
	FUNCTRACKER;
}

ProcStreamer::~ProcStreamer(void)
{
	FUNCTRACKER;
}

void
ProcStreamer::real_run()
{
	FUNCTRACKER;

	//BOOL os_res = ::SetThreadPriority(::GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
// 	if (os_res == FALSE)
// 	{
// 		LogSysError("::SetThreadPriority");
// 		throw;
// 	}

	I_AM_READY;

	while (true)
	{
		csp::SleepFor(Seconds(100));
	}
	

	// polling loop
	bool shutdownFlag = FALSE;

	long check_point		= 0;
	long prev_check_point   = 0;

	while (shutdownFlag != TRUE)
	{
		CcuApiErrorCode res = CCU_API_SUCCESS;

		// we should hit the check point once in 
		// polling period. If not - adapt sleeping 
		// time.
		check_point = ::GetTickCount();
		long working_period = check_point-prev_check_point;
		long delta  = working_period - CCU_DEFAULT_POLLING_TIME_MS;

		int sleep_time = CCU_DEFAULT_POLLING_TIME_MS;

		// it took exactly polling period
		if (delta == 0)
		{
			sleep_time = CCU_DEFAULT_POLLING_TIME_MS;
		} 
		// it took more than polling period
		else if( delta > 0 )
		{
			sleep_time = delta < CCU_DEFAULT_POLLING_TIME_MS ? CCU_DEFAULT_POLLING_TIME_MS - delta : 0;
		}
		// it took less than polling period
		else if (delta < 0)
		{
			sleep_time = CCU_DEFAULT_POLLING_TIME_MS + ::abs(delta);
		}


		//LogDebug(" wp:" << working_period);
		prev_check_point = check_point;
	
		::Sleep(sleep_time);
		
		   

		if (InboundPending())
		{
			CcuMsgPtr ptr = GetInboundMessage(Seconds(0),res);
			if (CCU_FAILURE(res))
			{
				throw;
			}

			

			switch (ptr->message_id)
			{
			case CCU_MSG_STREAMER_ADD_REQ:
				{
					AddStreamingObject(ptr);
					break;
				}
			case CCU_MSG_STREAMER_REMOVE_REQ:
				{
					RemoveStreamingObject(ptr);
					break;
				}
			case CCU_MSG_PROC_SHUTDOWN_REQ:
				{
					shutdownFlag = TRUE;
					SendResponse(ptr, new CcuMsgShutdownAck());
					break;
				}
			default:
				{
					LogCrit("Unknown message");
					throw;
				}
			} // switch
		}// if

		
		for (StreamingObjectSet::iterator iter = _streamingObjectsSet.begin(); 
			iter != _streamingObjectsSet.end(); 
			iter++)
		{
			StreamingObject *obj = (*iter); 

			CcuApiErrorCode streaming_result = obj->Process();
			if (streaming_result != CCU_API_OPERATION_IN_PROGRESS)
			{
				iter = _streamingObjectsSet.erase(iter);

				CcuMsgStreamingStopped *stopped = new CcuMsgStreamingStopped();
				stopped->obj = obj;
				_imsInbound->Send(stopped);

				if (iter == _streamingObjectsSet.end())
				{
					stopped->error = streaming_result;
					break;
				} // if
			} // if

		} // for
	}
}

void
ProcStreamer::AddStreamingObject(CcuMsgPtr req)
{
	shared_ptr<CcuMsgAddStreamingObjectReq> add_req 
		= dynamic_pointer_cast<CcuMsgAddStreamingObjectReq> (req);

	_streamingObjectsSet.insert(add_req->obj);
}

void 
ProcStreamer::RemoveStreamingObject(CcuMsgPtr req)
{
	shared_ptr<CcuMsgAddStreamingObjectReq> remove_req 
		= dynamic_pointer_cast<CcuMsgAddStreamingObjectReq> (remove_req);

	_streamingObjectsSet.erase(remove_req->obj);

	SendResponse(req, new CcuMsgRemoveStreamingObjectAck());

}
