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

	VAR_DECLSPEC RtpProfile av_profile;

	StreamingCtx::StreamingCtx():
	stream(NULL)
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
		LogCrit("Not implemented");
		throw;

	}

	ProcIms::ProcIms(IN LpHandlePair pair, IN CcuConfiguration &conf)
		:LightweightProcess(pair, IMS_Q, __FUNCTIONW__),
		_conf(conf),
		_localMedia(conf.ImsCnxInfo()),
		_portManager(CCU_DEFAULT_IMS_TOP_PORT,CCU_DEFAULT_IMS_BOTTOM_PORT)
	{
		FUNCTRACKER;

		_payloadTypeMap[L"pcma"] = &payload_type_pcma8000;
		_payloadTypeMap[L"pcmu"] = &payload_type_pcmu8000;
	}

	ProcIms::~ProcIms(void)
	{
		FUNCTRACKER;
	}

	void
	ProcIms::InitCodecs()
	{
		for (CodecsList::const_iterator iter = _conf.CodecList().begin();
			iter != _conf.CodecList().end();iter++)
		{

			const IxCodec *codec = *(iter);
			rtp_profile_set_payload(
				&av_profile,
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
		ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR|ORTP_FATAL);

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

		BOOL shutdownFlag = FALSE;
		while (shutdownFlag  == FALSE)
		{
			IxApiErrorCode err_code = CCU_API_SUCCESS;
			IxMsgPtr ptr =  _inbound->Wait(Seconds(60), err_code);

			if (err_code == CCU_API_TIMEOUT)
			{
				LogDebug("Streamer keep alive.");
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
					FreeResources();
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
					}
				}
			}
		}

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
		_portManager.BeginSearch();
		do 
		{
			int local_port = _portManager.GetNextCandidate();
			if (local_port == IX_UNDEFINED)
			{
				break;
			};

			// creates stream and session
			ctx->stream = audio_stream_new(local_port, ms_is_ipv6(_localMedia.iptoa()));
			if (ctx->stream!=NULL)
			{
				_portManager.MarkUnavailable(local_port);
			}

		} while (ctx->stream!=NULL);

		// failed to find available port
		if (ctx->stream == NULL)
		{
			goto error;
		}

		//
		// create local rtp profile and associate it with session
		//
		RtpSession *rtps=ctx->stream->session;
		RtpProfile *profile=rtp_profile_clone_full(&av_profile);
		rtp_session_set_profile(rtps,profile);

		//
		// create remote rtp session
		//
		int remport  = req->remote_media_data.port_ho();
		char *remip  = (char *)req->remote_media_data.iptoa();
		int res = rtp_session_set_remote_addr(rtps,remip,remport);
		if (res == 0) 
			goto error;
		
		rtp_session_set_payload_type(rtps,req->codec.sdp_mapping());
		if (res == 0) 
			goto error;

		rtp_session_set_jitter_compensation(rtps,0/*jitt_comp*/);
		if (res == 0) 
			goto error;

		//
		// create sending filter
		//
		ctx->stream->rtpsend=ms_filter_new(MS_RTP_SEND_ID);
		ms_filter_call_method(ctx->stream->rtpsend,MS_RTP_SEND_SET_SESSION,rtps);
		if (res == 0) 
			goto error;

		//
		// DTMF detection filter
		//
		ctx->stream->dtmfgen=ms_filter_new(MS_DTMF_GEN_ID);
		rtp_session_signal_connect(rtps,"telephone-event",(RtpCallback)on_dtmf_received,(unsigned long)ctx->stream->dtmfgen);
		if (res == 0) 
			goto error;
		

		//
		// creates encoder
		//
		PayloadType *pt = NULL;
		pt = rtp_profile_get_payload(&av_profile,req->codec.sdp_mapping() );
		if (pt==NULL)
			goto error;

		ctx->stream->encoder = ms_filter_create_encoder(pt->mime_type);
		ms_filter_call_method(ctx->stream->encoder,MS_FILTER_SET_SAMPLE_RATE,&pt->clock_rate);
		if (pt==NULL)
			goto error;

		if (pt->normal_bitrate>0)
		{
			LogDebug("Setting audio encoder network bitrate to " << pt->normal_bitrate);
			ms_filter_call_method(ctx->stream->encoder,MS_FILTER_SET_BITRATE,&pt->normal_bitrate);
			if (pt==NULL)
				goto error;
		}
		
		//
		// initialize file reading
		//
		ctx->stream->soundread = ms_filter_new(MS_FILE_PLAYER_ID);
		ms_filter_call_method(ctx->stream->soundread,MS_FILTER_SET_SAMPLE_RATE,&pt->clock_rate);

		ms_filter_link(ctx->stream->soundread,0,ctx->stream->encoder,0);
		ms_filter_link(ctx->stream->dtmfgen,0,ctx->stream->soundwrite,0);

		int handle = GetNewImsHandle();
		_streamingObjectSet[handle] = ctx;

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
			LogWarn("Ims context with handle=[" << req->playback_handle << "] not found");
			SendResponse(msg, new CcuMsgStartPlayReqNack());
			return;
		}

		StreamingCtxPtr ctx = (*iter).second;

#pragma TODO ("Handle play twice")

		audio_stream_play(ctx->stream,WStringToString(req->file_name).c_str());

		ms_ticker_attach(ctx->stream->ticker,ctx->stream->soundread);
	}

	void 
	ProcIms::UponPlaybackStopped(IxMsgPtr msg)
	{
		FUNCTRACKER;

		// 	shared_ptr<CcuMsgStreamingStopped> stopped_msg = 
		// 		dynamic_pointer_cast<CcuMsgStreamingStopped> (msg);
		// 
		// 	StreamingCtxsMap::iterator iter = 
		// 		_streamingObjectSet.find(stopped_msg->obj->ImsHandle());
		// 
		// 	_streamingObjectSet.erase(iter);
		// 
		// 	

	}


	void 
		ProcIms::StopPlayback(IxMsgPtr msg, ScopedForking &forking)
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

}

