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
#include "ProcIms.h"
#include "ProcPipeIPCDispatcher.h"
#include "StreamingObject.h"
#include "ProcStreamer.h"
#include "CcuLogger.h"
#include "Ims.h"

#define CCU_DEFAULT_IMS_TOP_PORT	6000
#define CCU_DEFAULT_IMS_BOTTOM_PORT 5000

static int 
GetNewImsHandle()
{
	FUNCTRACKER;

	static int i = 700000;

	return i++; 
}

StreamingCtx::StreamingCtx(StreamingObject *object, CcuMsgPtr req):
streaming_object(object),
orig_req(req)
{

}

StreamingCtx::StreamingCtx(const StreamingCtx &other)
{
	streaming_object = other.streaming_object;
	orig_req = other.orig_req;
}


ProcIms::ProcIms(LpHandlePair pair, CcuMediaData local_media)
:LightweightProcess(pair,IMS_Q,__FUNCTIONW__),
_localMedia(local_media),
_portManager(CCU_DEFAULT_IMS_TOP_PORT,CCU_DEFAULT_IMS_BOTTOM_PORT)
{
	FUNCTRACKER;

}

ProcIms::~ProcIms(void)
{
	FUNCTRACKER;
}

void
ProcIms::real_run()
{
	FUNCTRACKER;

	WSADATA dat;
	if (WSAStartup(MAKEWORD(2,2),&dat)!=0)
	{
		LogSysError("Error starting up WIN socket");
		throw;
	}

	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(ipc_pair);

	//
	// Start IPC
	//
	FORK(new ProcPipeIPCDispatcher(ipc_pair,IMS_Q));
	if (CCU_FAILURE(WaitTillReady(Seconds(5), ipc_pair)))
	{
		LogCrit("Cannot start IMS IPC interface exiting...");
		throw;
	}

	//
	// Start streamer
	// 
	DECLARE_NAMED_HANDLE_PAIR(streamer_pair);
	_streamerInbound = streamer_pair.inbound;

	FORK(new ProcStreamer(streamer_pair, _inbound));
	if (CCU_FAILURE(WaitTillReady(Seconds(5), streamer_pair)))
	{
		LogCrit("Cannot start Streamer process exiting...");
		throw;
	}


	I_AM_READY;

	BOOL shutdownFlag = FALSE;
	while (shutdownFlag  == FALSE)
	{
		CcuApiErrorCode err_code = CCU_API_SUCCESS;
		CcuMsgPtr ptr =  _inbound->Wait(Seconds(60), err_code);


		if (err_code == CCU_API_TIMEOUT)
		{
			LogDebug("IMS >>Keep Alive<<");
			continue;
		}

		switch (ptr->message_id)
		{
		case CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST:
			{
				AllocatePlaybackSession(ptr);
				break;
			}
		case CCU_MSG_START_PLAYBACK_REQUEST:
			{
				StartPlayback(ptr);
				break;
			}
		case CCU_MSG_PROC_SHUTDOWN_REQ:
			{
				shutdownFlag = TRUE;
				
				// release objects only if process that uses them is down. 
				// Leak is better than crash or unstable behavior
				if (Shutdown(Seconds(5),streamer_pair) == CCU_API_SUCCESS)
				{
					FreeResources();
				} 
				else 
				{
					LogWarn("Cannot stop streamer in a timely fashion");
				}

				SendResponse(ptr,new CcuMsgShutdownAck());
				continue;
			}
		case CCU_MSG_STREAMING_STOPPED_EVT:
			{
				//UponPlaybackStopped();
				break;
			}
		default:
			{
				BOOL  oob_res = HandleOOBMessage(ptr);

				if (oob_res == FALSE)
				{
					LogWarn("Received OOB message id=[" << ptr->message_id << "]");
					_ASSERT	(false);
				}
			}
		}
	}

	END_FORKING_REGION

	WSACleanup();

}

void 
ProcIms::UponPlaybackStopped(CcuMsgPtr msg)
{
	FUNCTRACKER;

	shared_ptr<CcuMsgStreamingStopped> stopped_msg = 
		dynamic_pointer_cast<CcuMsgStreamingStopped> (msg);

	StreamingCtxsMap::iterator iter = 
		_streamingObjectSet.find(stopped_msg->obj->ImsHandle());

	_streamingObjectSet.erase(iter);

	



}

void 
ProcIms::FreeResources()
{
	FUNCTRACKER;

	for ( StreamingCtxsMap::iterator iter = _streamingObjectSet.begin();
		 iter != _streamingObjectSet.end();
		 iter ++)
	{
		StreamingCtx ctx = (*iter).second;

		SendResponse(ctx.orig_req, new CcuMsgImsPlayStopped());

		ctx.streaming_object->Close(_portManager);
		delete ctx.streaming_object;

		iter = _streamingObjectSet.erase(iter);
		if (iter == _streamingObjectSet.end())
		{	
			break;
		}
	}

}

void 
ProcIms::AllocatePlaybackSession(CcuMsgPtr msg)
{
	FUNCTRACKER;

	shared_ptr<CcuMsgAllocateImsSessionReq> req  =
		dynamic_pointer_cast<CcuMsgAllocateImsSessionReq> (msg);

	StreamingObject *streaming_obj = 
		new StreamingObject(req->remote_media_data,req->file_name, 0);

	if (CCU_FAILURE(streaming_obj->Init(_portManager)))
	{
		CcuMsgAllocateImsSessionNack *nack = 
			new CcuMsgAllocateImsSessionNack();

		delete streaming_obj;

		SendResponse(req,nack);

		return;

	}

	CcuMsgAddStreamingObjectReq *str_req = new CcuMsgAddStreamingObjectReq();
	str_req->obj = streaming_obj;

	StreamingCtx ctx(streaming_obj,msg);

	int handle = GetNewImsHandle();
	_streamingObjectSet.insert(pair<ImsHandleId,StreamingCtx>(handle,ctx));

	CcuMsgAllocateImsSessionAck *ack = 
		new CcuMsgAllocateImsSessionAck();

	ack->playback_handle = handle;
	ack->ims_media = CcuMediaData("127.0.0.1", streaming_obj->Port());

	SendResponse(req,ack);

}

void 
ProcIms::StartPlayback(CcuMsgPtr msg)
{

	FUNCTRACKER;

	shared_ptr<CcuMsgStartPlayReq> req  =
		dynamic_pointer_cast<CcuMsgStartPlayReq> (msg);

	StreamingCtxsMap::iterator iter = 
		_streamingObjectSet.find(req->playback_handle);

	if (iter == _streamingObjectSet.end())
	{
		SendResponse(msg, new CcuMsgStartPlayReqNack());
		return;
	}

	CcuMsgAddStreamingObjectReq *strm_req = new CcuMsgAddStreamingObjectReq();
	strm_req->id = (*iter).first; 
	StreamingObject *streaming_object = (*iter).second.streaming_object;
	strm_req->obj = streaming_object;


	LogDebug("Started >>streaming<< to dest=[" << streaming_object->RemoteMediaData().ipporttos() << "]")


	_streamerInbound->Send(strm_req);

}

void 
ProcIms::StopPlayback(CcuMsgPtr msg, ScopedForking &forking)
{
	FUNCTRACKER;

// 	shared_ptr<CcuMsgStopPlayback> req  =
// 		dynamic_pointer_cast<CcuMsgStopPlayback> (msg);
// 
// 	StreamingObjectHandlesMap::iterator iter = _streamingObjectSet.find(req->playback_handle);	
// 	if (iter == _streamingObjectSet.end())	
// 	{
// 		return;
// 	}
// 
// 	_streamingObjectSet.erase(req->playback_handle);
// 
// 	// we have to receive ack on RTP remove in order
// 	// to reclaim port but we cannot wait for response
// 	// that's why we spawn another project
// 	//
// 	HANDLE_PAIR_DECLARE_(remover_pair);
// 	forking.forkInThisThread(
// 		new ProcStreamingObjectRemover(
// 		remover_pair,
// 		_streamerInbound,
// 		req->playback_handle,
// 		_portManager));

}



ProcStreamingObjectRemover::ProcStreamingObjectRemover(
	IN LpHandlePair pair, 
	IN LpHandlePtr streamer_handle,
	IN int handle,
	IN PortManager &ports_map
	):
LightweightProcess(pair,__FUNCTIONW__ ),
_streamerInbound(streamer_handle),
_portsManager(ports_map),
_streamHandle(handle)
{

}

ProcStreamingObjectRemover::~ProcStreamingObjectRemover()
{
	
}

void 
ProcStreamingObjectRemover::real_run()
{
	
	
	CcuMsgRemoveStreamingObjectReq *msg = 
		new CcuMsgRemoveStreamingObjectReq();

	msg->handle = _streamHandle;

	CcuMsgPtr response_ptr;
	EventsSet responses;
	responses.insert(CCU_MSG_STREAMER_REMOVE_ACK);

	CcuApiErrorCode res = DoRequestResponseTransaction(
		_streamerInbound,
		CcuMsgPtr(msg),
		responses,
		response_ptr,
		Seconds(60),
		L"Close RTP Connection TXN"
		);

	if (res!=CCU_API_SUCCESS)
	{
		LogCrit("Couldn't delete IMS streamer object in timely fashion - consider restarting IMS.");
		throw;
	}

	shared_ptr<CcuMsgRemoveStreamingObjectAck> ack = 
		dynamic_pointer_cast<CcuMsgRemoveStreamingObjectAck> (response_ptr);


	_portsManager.MarkAvailable(ack->obj->Port()); 

	delete ack->obj;


}
