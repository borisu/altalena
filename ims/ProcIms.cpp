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
#include "Ims.h"

namespace ivrworx 
{

#define CCU_DEFAULT_IMS_TOP_PORT	6000
#define CCU_DEFAULT_IMS_BOTTOM_PORT 5000

#define	CCU_DEFAULT_IMS_TIMEOUT		INFINITE

	static int GetNewImsHandle()
	{
		static volatile int handle_counter = 70000;
		return handle_counter;
	}

	StreamingCtx::StreamingCtx()
		:stream(NULL)
	{
	}

	StreamingCtx::~StreamingCtx()
	{
		if (stream->session!=NULL) rtp_session_destroy(stream->session);
		if (stream->rtpsend!=NULL) ms_filter_destroy(stream->rtpsend);
		if (stream->rtprecv!=NULL) ms_filter_destroy(stream->rtprecv);
		if (stream->soundread!=NULL) ms_filter_destroy(stream->soundread);
		if (stream->soundwrite!=NULL) ms_filter_destroy(stream->soundwrite);
		if (stream->encoder!=NULL) ms_filter_destroy(stream->encoder);
		if (stream->decoder!=NULL) ms_filter_destroy(stream->decoder);
		if (stream->dtmfgen!=NULL) ms_filter_destroy(stream->dtmfgen);
		if (stream->ec!=NULL)	ms_filter_destroy(stream->ec);
		//if (stream->ticker!=NULL) ms_ticker_destroy(stream->ticker);
		ms_free(stream);
	}



	static void 
	on_dtmf_received(RtpSession *s, int dtmf, void * user_data)
	{

	}

	boost::mutex g_args_mutes;
	
	struct EventArgs 
		: OVERLAPPED
	{
		EventArgs():
		iocp_handle(NULL),ims_handle(IX_UNDEFINED){}

		HANDLE iocp_handle;

		ImsHandleId ims_handle;

	};

	static void 
	on_file_filter_event(void *userdata , unsigned int id, void *arg)
	{

		EventArgs * args = (EventArgs*)userdata;

		switch (id)
		{
		case MS_FILE_PLAYER_EOF:
			{
				ULONG dwNumberOfBytesTransferred = 0;


				BOOL res = ::PostQueuedCompletionStatus(
					args->iocp_handle,			//A handle to an I/O completion port to which the I/O completion packet is to be posted.
					dwNumberOfBytesTransferred,	//The value to be returned through the lpNumberOfBytesTransferred parameter of the GetQueuedCompletionStatus function.
					MS_FILE_PLAYER_EOF,			//The value to be returned through the lpCompletionKey parameter of the GetQueuedCompletionStatus function.
					args						//The value to be returned through the lpOverlapped parameter of the GetQueuedCompletionStatus function.
					);

				if (res == FALSE)
				{
					LogSysError("::PostQueuedCompletionStatus");
					throw;
				}

			}
		default:
			{

			}
		}
	}




	ProcIms::ProcIms(IN LpHandlePair pair, IN CcuConfiguration &conf)
		:LightweightProcess(pair, IMS_Q, __FUNCTIONW__),
		_conf(conf),
		_localMedia(conf.ImsCnxInfo()),
		_portManager(CCU_DEFAULT_IMS_TOP_PORT,CCU_DEFAULT_IMS_BOTTOM_PORT)
	{
		FUNCTRACKER;

		_iocpPtr = IocpInterruptorPtr(new IocpInterruptor());
		_inbound->HandleInterruptor(_iocpPtr);


		_payloadTypeMap[L"PCMA"] = &payload_type_pcma8000;
		_payloadTypeMap[L"PCMU"] = &payload_type_pcmu8000;
	}

	ProcIms::~ProcIms(void)
	{
		FUNCTRACKER;

		ms_ticker_destroy(_ticker);
		ms_exit();
		rtp_profile_destroy(av_profile);
		ortp_exit();
	}



	void
	ProcIms::InitCodecs()
	{
		for (CodecsList::const_iterator iter = _conf.CodecList().begin();
			iter != _conf.CodecList().end();iter++)
		{

			const IxCodec *codec = *(iter);
			rtp_profile_set_payload(
				av_profile,
				codec->sdp_mapping(),
				_payloadTypeMap[codec->sdp_name()]);
		}

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

		//
		// initialize ortp
		//
		ortp_init();
		ms_init();
		ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR|ORTP_FATAL);

		_ticker = ms_ticker_new();
		if (_ticker == NULL)
		{
			LogCrit("Cannot start ms ticker");
			throw;
		}


		av_profile=rtp_profile_new("ivrworx profile");

		// 
		// set master RTP profile
		//
		InitCodecs();


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

		I_AM_READY;

		BOOL shutdown_flag = FALSE;
		while (shutdown_flag == FALSE)
		{

			DWORD number_of_bytes    = 0;
			ULONG_PTR completion_key = 0;
			LPOVERLAPPED lpOverlapped = NULL;

			BOOL res = ::GetQueuedCompletionStatus(
				_iocpPtr->Handle(),		// A handle to the completion port. To create a completion port, use the CreateIoCompletionPort function.
				&number_of_bytes,		// A pointer to a variable that receives the number of bytes transferred during an I/O operation that has completed.
				&completion_key,		// A pointer to a variable that receives the completion key value associated with the file handle whose I/O operation has completed. A completion key is a per-file key that is specified in a call to CreateIoCompletionPort.
				&lpOverlapped,			// A pointer to a variable that receives the address of the OVERLAPPED structure that was specified when the completed I/O operation was started. 
				CCU_DEFAULT_IMS_TIMEOUT // The number of milliseconds that the caller is willing to wait for a completion packet to appear at the completion port. If a completion packet does not appear within the specified time, the function times out, returns FALSE, and sets *lpOverlapped to NULL.
				);

			// error during overlapped I/O?
			int err = ::GetLastError() ;
			if (res == FALSE && err != WAIT_TIMEOUT)
			{
				LogSysError("::GetQueuedCompletionStatus");
				throw;
			} // timeout ?
			else if (err == WAIT_TIMEOUT)
			{
				LogDebug("Ims keep alive.");
				continue;
			}

			// oRTP event?
			switch (completion_key)
			{
			case MS_FILE_PLAYER_EOF:
				{
					UponPlaybackStopped(lpOverlapped);
					continue;
				}
			default:
				{
					if (completion_key!=IOCP_UNIQUE_COMPLETION_KEY)
					{
						LogCrit("Unknonwn overlapped structure received");
						throw;
					}
				}
			}


			IxApiErrorCode err_code = CCU_API_SUCCESS;
			IxMsgPtr ptr =  _inbound->Wait(Seconds(0), err_code);

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

			case CCU_MSG_IMS_TEARDOWN_REQ:
				{
					TearDown(ptr);
					break;
				}
			case CCU_MSG_PROC_SHUTDOWN_REQ:
				{
					shutdown_flag = TRUE;
					SendResponse(ptr,new CcuMsgShutdownAck());
					break;
				}
			default:
				{
					BOOL oob_res = HandleOOBMessage(ptr);
					if (oob_res == FALSE)
					{
						LogCrit("Received unknown OOB message id=[" << ptr->message_id << "]");
						throw;
					}// if
				}// default
			}// switch
		}// while

		FreeResources();

		END_FORKING_REGION
		WSACleanup();
	}


	void 
	ProcIms::AllocatePlaybackSession(IxMsgPtr msg)
	{
		FUNCTRACKER;

		shared_ptr<CcuMsgAllocateImsSessionReq> req  =
			dynamic_pointer_cast<CcuMsgAllocateImsSessionReq> (msg);

		//
		// create new context
		//
		StreamingCtxPtr ctx(new StreamingCtx());

		//
		// create ortp stream with and initialize with available port
		//
		do 
		{
			int local_port = _portManager.GetNextPort();
			if (local_port == IX_UNDEFINED)
			{
				break;
			};

			// creates stream and session
			ctx->stream = audio_stream_new(local_port, ms_is_ipv6(_localMedia.iptoa()));
			if (ctx->stream!=NULL)
			{
				break;
			}

			_portManager.Return(local_port);

		} while (ctx->stream!=NULL);

		// failed to find available port
		if (ctx->stream == NULL)
		{
			LogWarn("Failed to find available port");
			goto error;
		}

		//
		// create local rtp profile and associate it with session
		//
		RtpSession *rtps=ctx->stream->session;
		RtpProfile *profile=rtp_profile_clone_full(av_profile);
		rtp_session_set_profile(rtps,profile);

		//
		// create remote rtp session
		//
		int remport  = req->remote_media_data.port_ho();
		char *remip  = (char *)req->remote_media_data.iptoa();
		int res = rtp_session_set_remote_addr(rtps,remip,remport);
		if (res < 0) 
		{
			LogWarn("error:rtp_session_set_remote_addr");
			goto error;
		}


		res = rtp_session_set_payload_type(rtps,req->codec.sdp_mapping());
		if (res < 0) 
		{
			LogWarn("error:rtp_session_set_payload_type ");
			goto error;
		}

		rtp_session_set_jitter_compensation(rtps,0/*jitt_comp*/);

		//
		// create sending filter
		//
		MSFilter *rtp_send =ms_filter_new(MS_RTP_SEND_ID);
		ctx->stream->rtpsend = rtp_send;
		ms_filter_call_method(ctx->stream->rtpsend,MS_RTP_SEND_SET_SESSION,rtps);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_call_method");
			goto error;
		}

		//
		// DTMF detection filter
		//
		ctx->stream->dtmfgen=ms_filter_new(MS_DTMF_GEN_ID);

#pragma warning (suppress :4311)
		rtp_session_signal_connect(rtps,"telephone-event",(RtpCallback)on_dtmf_received,(unsigned long)ctx->stream->dtmfgen);

		if (res < 0) 
		{
			LogWarn("error:rtp_session_signal_connect");
			goto error;
		}


		//
		// creates encoder
		//
		PayloadType *pt = NULL;
		pt = rtp_profile_get_payload(av_profile,req->codec.sdp_mapping());
		if (pt==NULL)
		{
			LogWarn("error:rtp_profile_get_payload");
			goto error;
		}

		ctx->stream->encoder = ms_filter_create_encoder(pt->mime_type);
		if (ctx->stream->encoder == NULL) 
		{
			LogWarn("error:ms_filter_create_encoder");
			goto error;
		}


		// 		res = ms_filter_call_method(ctx->stream->encoder,MS_FILTER_SET_SAMPLE_RATE,&pt->clock_rate);
		// 		if (res < 0) 
		// 		{
		// 			LogWarn("error:ms_filter_call_method");
		// 			goto error;
		// 		}

		// 		if (pt->normal_bitrate>0)
		// 		{
		// 			LogDebug("Setting audio encoder network bitrate to " << pt->normal_bitrate);
		// 			res = ms_filter_call_method(ctx->stream->encoder,MS_FILTER_SET_BITRATE,&pt->normal_bitrate);
		// 			if (res < 0) 
		// 			{
		// 				LogWarn("error:ms_filter_call_method");
		// 				goto error;
		// 			}
		// 		}

		//
		// initialize file reading
		//
		ctx->stream->soundread = ms_filter_new(MS_FILE_PLAYER_ID);
		if (ctx->stream->soundread  == NULL) 
		{
			LogWarn("error:ms_filter_new(MS_FILE_PLAYER_ID)");
			goto error;
		}

		ctx->stream->soundread->notify = &on_file_filter_event;

		long handle = GetNewImsHandle();

		EventArgs *args = new EventArgs();
		SecureZeroMemory(args, sizeof(EventArgs));
		args->iocp_handle = _iocpPtr->Handle();
		args->ims_handle = handle;

		ctx->stream->soundread->notify_ud = args;

		

		// 		res = ms_filter_call_method(ctx->stream->soundread,MS_FILTER_SET_SAMPLE_RATE,&pt->clock_rate);
		// 		if (res < 0) 
		// 		{
		// 			LogWarn("error:ms_filter_call_method");
		// 			goto error;
		// 		}

		res = ms_filter_link(ctx->stream->soundread,0,ctx->stream->encoder,0);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_link");
			goto error;
		}

		ms_filter_link(ctx->stream->encoder,0,ctx->stream->rtpsend,0);
		// 		ms_filter_link(ctx->stream->rtprecv,0,stream->decoder,0);
		// 		ms_filter_link(stream->decoder,0,stream->dtmfgen,0);


		// 		res = ms_filter_link(ctx->stream->dtmfgen,0,ctx->stream->soundwrite,0);
		// 		if (res < 0) 
		// 		{
		// 			LogWarn("error:ms_filter_link");
		// 			goto error;
		// 		}


		// use stream pointer as a handle
		
		_streamingObjectSet[handle] = ctx;

		ctx->curr_txn_handler = req->session_handler;

		CcuMsgAllocateImsSessionAck *ack = 
			new CcuMsgAllocateImsSessionAck();
		ack->playback_handle = handle;

		SendResponse(req,ack);
		return;

error:
		SendResponse(req,new CcuMsgAllocateImsSessionNack());
		return;
	}

	void 
	ProcIms::StartPlayback(IxMsgPtr msg)
	{

		FUNCTRACKER;

		shared_ptr<CcuMsgStartPlayReq> req  =
			dynamic_pointer_cast<CcuMsgStartPlayReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->playback_handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("Invalid ims handle " << req->playback_handle);
			SendResponse(msg, new CcuMsgStartPlayReqNack());
			return;
		}

		StreamingCtxPtr ctx = iter->second;
		ctx->curr_txn_handler = req->source;

		audio_stream_play(ctx->stream,WStringToString(req->file_name).c_str());
		if (ctx->stream->ticker == NULL)
		{
			ctx->stream->ticker = _ticker;
		};

		int loop_param = req->loop ? 0 : -1;
		int res = ms_filter_call_method(ctx->stream->soundread,MS_FILE_PLAYER_LOOP, &loop_param);
		if (res < 0)
		{
			LogWarn("Cannot set loop parameter, ims handle " << req->playback_handle);
			SendResponse(msg, new CcuMsgStartPlayReqNack());
			return;
		}
		

		res = ms_ticker_attach(ctx->stream->ticker,ctx->stream->soundread);
		if (res < 0)
		{
			LogWarn("Cannot start palying (ms_ticker_attach), ims handle=" << req->playback_handle);
			SendResponse(msg, new CcuMsgStartPlayReqNack());
			return;
		}

		if (req->send_provisional)
		{
			SendResponse(msg, new CcuMsgStartPlayReqAck());
		}
	}

	void 
	ProcIms::UponPlaybackStopped(OVERLAPPED *ovlp)
	{
		FUNCTRACKER;

		// pointer guard
		auto_ptr<EventArgs> args = auto_ptr<EventArgs>((EventArgs*)ovlp);

		CcuMsgImsPlayStopped *stopped_msg = new CcuMsgImsPlayStopped();
		stopped_msg->playback_handle = args->ims_handle; 

		StreamingCtxsMap::iterator iter = _streamingObjectSet.find(args->ims_handle);
		if (iter == _streamingObjectSet.end())
		{
			LogWarn("Stopped playback on nen existent handle " << args->ims_handle);
			return;
		}

		StreamingCtxPtr ctx = iter->second;
		stopped_msg->dest = ctx->curr_txn_handler;

		this->SendMessage(IxMsgPtr(stopped_msg));

	}

	void
	ProcIms::TearDown(IxMsgPtr msg)
	{
		FUNCTRACKER;

		shared_ptr<CcuMsgImsTearDownReq> req  =
			dynamic_pointer_cast<CcuMsgImsTearDownReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("Invalid ims handle " << req->handle);
			return;
		}

		LogDebug("Tear down playback session, ims handle:" << req->handle);

		StreamingCtxPtr ctx = iter->second;
		
		TearDown(ctx);
		
		// ctx dtor should release all associated resources
		_streamingObjectSet.erase(iter);
		
	}

	void 
	ProcIms::TearDown(StreamingCtxPtr ctx)
	{
		FUNCTRACKER;

		AudioStream *stream = ctx->stream;

		if (stream->ticker)
		{
			ms_ticker_detach(stream->ticker,stream->soundread);
			//ms_ticker_detach(stream->ticker,stream->rtprecv);

			rtp_stats_display(rtp_session_get_stats(stream->session),"Audio session's RTP statistics");

			if (stream->ec!=NULL){
				ms_filter_unlink(stream->soundread,0,stream->ec,1);
				ms_filter_unlink(stream->ec,1,stream->encoder,0);
				ms_filter_unlink(stream->dtmfgen,0,stream->ec,0);
				ms_filter_unlink(stream->ec,0,stream->soundwrite,0);
			}else{
				ms_filter_unlink(stream->soundread,0,stream->encoder,0);
				ms_filter_unlink(stream->dtmfgen,0,stream->soundwrite,0);
			}

			ms_filter_unlink(stream->encoder,0,stream->rtpsend,0);
			//ms_filter_unlink(stream->rtprecv,0,stream->decoder,0);
			//ms_filter_unlink(stream->decoder,0,stream->dtmfgen,0);
		}

	}


	void 
	ProcIms::StopPlayback(IxMsgPtr msg)
	{
		FUNCTRACKER;

		shared_ptr<CcuMsgStopPlaybackReq> req  =
		 	dynamic_pointer_cast<CcuMsgStopPlaybackReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("Invalid ims handle " << req->handle);
			return;
		}

		LogDebug("Stop playback, ims session:" << req->handle);

		StreamingCtxPtr ctx = iter->second;
		AudioStream *stream = ctx->stream;

		if (stream->ticker)
		{
			int res = ms_ticker_detach(stream->ticker,stream->soundread);
			if (res < 0)
			{
				LogWarn("ms2 error - ms_ticker_detach, res=" << res );
				SendResponse(req,new CcuMsgStopPlaybackNack());
			}
			
		}

		SendResponse(req,new CcuMsgStopPlaybackAck());

	}

	void 
	ProcIms::FreeResources()
	{

		FUNCTRACKER;

		for(StreamingCtxsMap::iterator iter = _streamingObjectSet.begin();
			iter != _streamingObjectSet.end();
			iter++)
		{
			TearDown(iter->second);
		}

		_streamingObjectSet.clear();

		ms_ticker_destroy(_ticker);

	}

}

