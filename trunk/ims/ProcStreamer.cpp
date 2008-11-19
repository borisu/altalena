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

	I_AM_READY;

	// polling loop
	bool shutdownFlag = FALSE;
	while (shutdownFlag != TRUE)
	{
		CcuApiErrorCode res = CCU_API_SUCCESS;

		CcuMsgPtr ptr = _inbound->Wait(
			MilliSeconds(CCU_DEFAULT_POLLING_TIME_MS),
			res);

		if (res != CCU_API_TIMEOUT)
		{
			switch (ptr->message_id)
			{
			case CCU_MSG_STREAMER_ADD:
				{
					AddStreamingObject(ptr);
					break;
				}
			case CCU_MSG_STREAMER_REMOVE:
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
	shared_ptr<CcuMsgAddStreamingObject> add_req 
		= dynamic_pointer_cast<CcuMsgAddStreamingObject> (req);

	_streamingObjectsSet.insert(add_req->obj);
}

void 
ProcStreamer::RemoveStreamingObject(CcuMsgPtr req)
{
	shared_ptr<CcuMsgAddStreamingObject> remove_req 
		= dynamic_pointer_cast<CcuMsgAddStreamingObject> (remove_req);

	_streamingObjectsSet.erase(remove_req->obj);

	SendResponse(req, new CcuMsgRemoveStreamingObjectAck());

}
