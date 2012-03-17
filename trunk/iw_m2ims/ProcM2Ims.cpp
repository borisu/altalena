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
#include "ProcM2Ims.h"


namespace ivrworx 
{

#define IW_DEFAULT_IMS_TOP_PORT	60000
#define IW_DEFAULT_IMS_BOTTOM_PORT 50000

#define	IW_DEFAULT_IMS_TIMEOUT		60000 // 1 min
#define IW_MAX_RTP_MSG_LENGTH	1024

	HANDLE g_iocpHandle = NULL;

	static void iw_logger_func(OrtpLogLevel lev, const char *fmt, va_list args) 
	{
		char buf[IW_MAX_RTP_MSG_LENGTH];
		buf[0] = '\0';

		_vsnprintf_s(buf,IW_MAX_RTP_MSG_LENGTH,fmt,args);

		switch (lev)
		{
		case ORTP_MESSAGE:
		case ORTP_DEBUG:
			{
				LogDebug(buf);
				break;
			}
		case ORTP_ERROR:
		case ORTP_WARNING:
			{
				LogWarn(buf);
				break;
			}
		case ORTP_FATAL:
			{
				LogCrit(buf);
				break;
			}
		default:
			{
				LogDebug(buf);
			}
		}
	}

	static int GetNewImsHandle()
	{
		static volatile int handle_counter = 7000000;
		return handle_counter++;
	}

	struct RtpWorkerParams
	{
		RtpWorkerParams():
		shutdown_event(NULL),
		q(NULL){}

		HANDLE shutdown_event;

		OrtpEvQueue *q;
	};

	DWORD WINAPI RtpWorker(LPVOID iValue)
	{
		FUNCTRACKER;

		RtpWorkerParams *params = (RtpWorkerParams*) iValue;

		// currently it is dummy thread which only destroys received packets
		OrtpEvent *ev = NULL;
		DWORD res = WAIT_TIMEOUT;
		do
		{
			while ((ev = ortp_ev_queue_get(params->q))!=NULL)
			{
				ortp_event_destroy(ev);
				res = ::WaitForSingleObject(params->shutdown_event,0);
				if (res != WAIT_TIMEOUT)
				{
					break;
				}
			}

			res = ::WaitForSingleObject(params->shutdown_event,5);

		} while (res == WAIT_TIMEOUT);

		delete params;

		return 0;
	}


	StreamingCtx::StreamingCtx()
		:stream(NULL),
		profile(NULL),
		state(IMS_INITIAL),
		streamer_handle(IW_UNDEFINED),
		correlation_id(IW_UNDEFINED),
		snd_device_type(SND_DEVICE_TYPE_FILE),
		rcv_device_type(RCV_DEVICE_FILE_REC_ID),
		pt(NULL)
	{

	}

	StreamingCtx::~StreamingCtx()
	{
		if (stream)
		{
			if (stream->session!=NULL)		
				rtp_session_destroy(stream->session);

			if (stream->rtpsend!=NULL)		
				ms_filter_destroy(stream->rtpsend);

			if (stream->rtprecv!=NULL)		
				ms_filter_destroy(stream->rtprecv);

			if (stream->soundread!=NULL)	
				ms_filter_destroy(stream->soundread);

			if (stream->soundwrite!=NULL) 
				ms_filter_destroy(stream->soundwrite);

			if (stream->encoder!=NULL) 
				ms_filter_destroy(stream->encoder);

			if (stream->decoder!=NULL) 
				ms_filter_destroy(stream->decoder);

			if (stream->dtmfgen!=NULL) 
				ms_filter_destroy(stream->dtmfgen);

			if (stream->ec!=NULL)	
				ms_filter_destroy(stream->ec);
			
			ms_free(stream);
		}
		if (profile) rtp_profile_destroy(profile);

	}

#define IW_IMS_EOF_EVENT  1

	static void 
	on_file_filter_event(void *userdata , unsigned int id, void *arg)
	{

		StreamerHandle handle = (StreamerHandle) userdata;

		switch (id)
		{
		case MS_FILE_PLAYER_EOF:
			{
			
				ImsOverlapped *olap = new ImsOverlapped();
				olap->streamer_handle_id = handle;

				BOOL res = ::PostQueuedCompletionStatus(
					g_iocpHandle,			//A handle to an I/O completion port to which the I/O completion packet is to be posted.
					0,						//The value to be returned through the lpNumberOfBytesTransferred parameter of the GetQueuedCompletionStatus function.
					IW_IMS_EOF_EVENT,		//The value to be returned through the lpCompletionKey parameter of the GetQueuedCompletionStatus function.
					olap					//The value to be returned through the lpOverlapped parameter of the GetQueuedCompletionStatus function.
					);

				if (res == FALSE)
				{
					LogSysError("::PostQueuedCompletionStatus");
					throw;
				}

				break;

			}
		default:
			{
				LogWarn("Unknown file filter event:" << id);
			}
		}
	}

	ProcM2Ims::ProcM2Ims(IN LpHandlePair pair, IN ConfigurationPtr conf)
		:LightweightProcess(pair, "Ims"),
		_conf(conf),
		_rtp_q(NULL),
		_rtpWorkerShutdownEvt(NULL),
		_avProfile(NULL),
		_ticker(NULL),
		_correlationCounter(0),
		_rtpWorkerHandle(NULL)
	{
		FUNCTRACKER;

		ServiceId(_conf->GetString("m2ims/uri"));

		_iocpPtr = IocpInterruptorPtr(new IocpInterruptor());
		_inbound->HandleInterruptor(_iocpPtr);

	}

	ProcM2Ims::~ProcM2Ims(void)
	{
		FUNCTRACKER;

		CleanUpResources();
		
	}

	void
	ProcM2Ims::CleanUpResources()
	{
		FUNCTRACKER;

		// close RtpWorker if not already stopped
		if ( _rtpWorkerShutdownEvt != NULL && 
			::WaitForSingleObject(_rtpWorkerShutdownEvt,0) != WAIT_OBJECT_0)
		{
			::SetEvent(_rtpWorkerShutdownEvt);
			::Sleep(100);
		}

		if (_rtpWorkerHandle != NULL) 
		{
			::CloseHandle(_rtpWorkerHandle);
		}

		if (_ticker != NULL) 
		{
			ms_ticker_destroy(_ticker);
			_ticker = NULL;
		}
		
		ms_exit();

		if (_avProfile != NULL) 
		{
			rtp_profile_destroy(_avProfile);
			_avProfile = NULL;
		}

		if (_rtp_q != NULL) 
		{
			ortp_ev_queue_destroy(_rtp_q);
		}

		ortp_exit();

		if (_rtpWorkerShutdownEvt != NULL) 
		{
			::CloseHandle(_rtpWorkerShutdownEvt);
			_rtpWorkerShutdownEvt  = NULL;
		}
			
	}

	void 
	ProcM2Ims::NegotiateCodec(IN const MediaFormatsList &offer, OUT PayloadType **t, OUT int *idx)
	{
		for (MediaFormatsList::const_iterator iter = offer.begin(); iter != offer.end(); ++iter)
		{
			for (int i=0; i<RTP_PROFILE_MAX_PAYLOADS; ++i)
			{
				PayloadType *curr = _avProfile->payload[i];
				if (curr == NULL)
					continue;

				char *x =curr->mime_type;

				if ((*iter).sdp_name_tos() == x)
				{
					*t = curr;
					*idx = i;
					return;
				} // if
			}// for
		}// for
	}// func 

	

	

	void
	ProcM2Ims::InitCodecs()
	{
		
		rtp_profile_set_payload(_avProfile,0,&payload_type_pcmu8000);
		rtp_profile_set_payload(_avProfile,1,&payload_type_lpc1016);
		rtp_profile_set_payload(_avProfile,3,&payload_type_gsm);
		rtp_profile_set_payload(_avProfile,7,&payload_type_lpc);
		rtp_profile_set_payload(_avProfile,4,&payload_type_g7231);
		rtp_profile_set_payload(_avProfile,8,&payload_type_pcma8000);
		rtp_profile_set_payload(_avProfile,10,&payload_type_l16_stereo);
		rtp_profile_set_payload(_avProfile,11,&payload_type_l16_mono);
		rtp_profile_set_payload(_avProfile,18,&payload_type_g729);
		rtp_profile_set_payload(_avProfile,31,&payload_type_h261);
		rtp_profile_set_payload(_avProfile,32,&payload_type_mpv);
		rtp_profile_set_payload(_avProfile,34,&payload_type_h263);

		DWORD time = ::GetTickCount();

		stringstream rtmmaps;
		stringstream codecss;
		

		// build sdp
		
		
		for (int i=0; i<RTP_PROFILE_MAX_PAYLOADS; ++i)
		{
			PayloadType *curr = _avProfile->payload[i];
			if (curr == NULL)
				continue;
					
			char *x =payload_type_get_rtpmap(curr);

			codecss << " " << i;
			rtmmaps << "a=rtpmap:" << i << " " << x << "\r\n"; 

			

			//ortp_free(x);
			
		}

		_rtpMapPostfix = rtmmaps.str();
		_codecsListPostfix = codecss.str();

		
			
	}


	void
	ProcM2Ims::real_run()
	{
		FUNCTRACKER;

		WSADATA dat;
		if (WSAStartup(MAKEWORD(2,2),&dat)!=0)
		{
			LogCrit("Error starting up WIN socket. err:" << ::WSAGetLastError());
			return;
		}

		g_iocpHandle= _iocpPtr->WinHandle();

		string m2_ip = _conf->GetString("m2ims/local_ip");
		const char * input_address_str = m2_ip.c_str();

		_localInAddr = convert_hname_to_addrin(input_address_str);
		
		LogDebug("ProcM2Ims::real_run -  bind is done 0.0.0.0, local_ip=" << m2_ip << ", wil be used in communication protocols.");

		//
		// initialize ortp
		//
		ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR|ORTP_FATAL);
		ortp_set_log_handler(iw_logger_func);

		ortp_init();
		ms_init();

		_ticker = ms_ticker_new();
		if (_ticker == NULL)
		{
			LogCrit("Cannot start ms ticker");
			throw;
		}


		_avProfile=rtp_profile_new("ivrworx profile");

		// 
		// set master RTP profile
		//
		InitCodecs();

		_rtp_q=ortp_ev_queue_new();	

		//
		// start rtp packets consuming thread
		//
		_rtpWorkerShutdownEvt = ::CreateEvent( 
			NULL,               // default security attributes
			TRUE,               // manual-reset event
			FALSE,              // initial state is nonsignaled
			NULL				// object name
			);

		if (_rtpWorkerShutdownEvt == NULL)
		{
			LogSysError("::CreateEvent");
			throw;
		}

		RtpWorkerParams *params = new RtpWorkerParams();
		params->q = _rtp_q;
		params->shutdown_event = _rtpWorkerShutdownEvt;

		DWORD dwThreadId = 0;
		_rtpWorkerHandle = ::CreateThread( 
			NULL,                   // default security attributes
			0,                      // use default stack size  
			RtpWorker,				 // thread function name
			params,			        // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);   // returns the thread identifier 

		if (_rtpWorkerHandle == NULL)
		{
			LogSysError("::CreateThread");
			throw;
		}


		START_FORKING_REGION;

		LogDebug("Ims process started successfully.");
		I_AM_READY;


		BOOL shutdown_flag = FALSE;
		while (shutdown_flag == FALSE)
		{

			DWORD number_of_bytes    = 0;
			ULONG_PTR completion_key = 0;
			LPOVERLAPPED lpOverlapped = NULL;

			BOOL res = ::GetQueuedCompletionStatus(
				_iocpPtr->WinHandle(),		// A handle to the completion port. To create a completion port, use the CreateIoCompletionPort function.
				&number_of_bytes,		// A pointer to a variable that receives the number of bytes transferred during an I/O operation that has completed.
				&completion_key,		// A pointer to a variable that receives the completion key value associated with the file handle whose I/O operation has completed. A completion key is a per-file key that is specified in a call to CreateIoCompletionPort.
				&lpOverlapped,			// A pointer to a variable that receives the address of the OVERLAPPED structure that was specified when the completed I/O operation was started. 
				IW_DEFAULT_IMS_TIMEOUT // The number of milliseconds that the caller is willing to wait for a completion packet to appear at the completion port. If a completion packet does not appear within the specified time, the function times out, returns FALSE, and sets *lpOverlapped to NULL.
				);

			//IX_PROFILE_CHECK_INTERVAL(10000);

			// error during overlapped I/O?
			int last_err = ::GetLastError();
			if (res == FALSE)
			{
				if (last_err != WAIT_TIMEOUT)
				{
					LogSysError("::GetQueuedCompletionStatus");
					throw;
				}
				else 
				{
					LogInfo("Ims keep alive.");
					continue;
				}
			}

			// oRTP event?
			switch (completion_key)
			{
			case IW_IMS_EOF_EVENT:
				{
					UponPlaybackStopped((ImsOverlapped*)lpOverlapped);
					continue;
				}
			default:
				{
					if (completion_key!=IOCP_UNIQUE_COMPLETION_KEY)
					{
						LogCrit("Unknown overlapped structure received");
						throw;
					}
				}
			}

			try
			{
				ApiErrorCode err_code = API_SUCCESS;
				IwMessagePtr ptr =  _inbound->Wait(Seconds(0), err_code);

				switch (ptr->message_id)
				{
				case MSG_STREAM_ALLOCATE_SESSION_REQ:
					{
						AllocatePlaybackSession(ptr);
						break;
					}
				case MSG_STREAM_MODIFY_REQ:
					{
						ModifySession(ptr);
						break;
					}
				case MSG_STREAM_PLAY_REQUEST:
					{
						StartPlayback(ptr);
						break;
					}
				case MSG_STREAM_STOP_PLAY_REQ:
					{
						StopPlayback(ptr);
						break;
					}

				case MSG_STREAM_TEARDOWN_REQ:
					{
						TearDown(ptr);
						break;
					}
				case MSG_PROC_SHUTDOWN_REQ:
					{
						shutdown_flag = TRUE;
						SendResponse(ptr,new MsgShutdownAck());
						break;
					}
				default:
					{
						BOOL oob_res = HandleOOBMessage(ptr);
						if (oob_res == FALSE)
						{
							LogWarn("Received unknown OOB msg:" << ptr->message_id);
						}// if
					}// default
				}// switch

			}
			catch (std::exception e)
			{
				LogWarn("ProcM2Ims::real_run - exception:" << e.what());
			}
			
		}// while

		TearDownAllSessions();

		::SetEvent(_rtpWorkerShutdownEvt);
		::Sleep(100);

		END_FORKING_REGION
		WSACleanup();
	}


	void 
	ProcM2Ims::AllocatePlaybackSession(IwMessagePtr msg)
	{
		FUNCTRACKER;

		PayloadType *pt = NULL;
		int idx			= -1;
		stringstream sdps;

		shared_ptr<MsgStreamAllocateSessionReq> req  =
			dynamic_pointer_cast<MsgStreamAllocateSessionReq> (msg);

		//
		// create new context
		//
		StreamingCtxPtr ctx(new StreamingCtx());
		long handle = GetNewImsHandle();
		ctx->streamer_handle = handle;
		ctx->rcv_device_type = req->rcv_device_type;

		//
		// create ortp stream and initialize it with available port
		//
		// local port will be allocated dynamically
		ctx->stream = audio_stream_new(0, false /*ms_is_ipv6(req->local_media_data.iptoa()*/);
		if (ctx->stream==NULL)
		{
			LogWarn("Failed to find available port");
			goto error;

		}
		

		/********************
		*
		*		Session 
		*
		*********************/
		// create local rtp profile and associate it with session
		RtpSession *rtps=ctx->stream->session;
		RtpProfile *profile=rtp_profile_clone_full(_avProfile);
		rtp_session_set_profile(rtps,profile);
		ctx->profile = profile;

		if (req->offer.body.size() != 0 && req->offer.type == "application/sdp")
		{
			SdpParser parser(req->offer.body);
			SdpParser::Medium medium = parser.first_audio_medium();

			if (!medium.connection.is_valid() || medium.list.empty())
			{
				LogDebug("ProcM2Ims::AllocatePlaybackSession - error parsing sdp, imsh:" <<  handle);
				goto error;
			}

			
			NegotiateCodec(medium.list,&pt,&idx);
			if (pt == NULL)
			{
				LogDebug("ProcM2Ims::AllocatePlaybackSession - error negotiating, imsh:" <<  handle);
				goto error;
			}
			else
			{
				ctx->pt = pt;
			}
				
			ApiErrorCode res = RecommutateSession(ctx,medium.connection,pt,idx);
			if (IW_FAILURE(res))
			{
				LogDebug("ProcM2Ims::AllocatePlaybackSession - error re-commutating, imsh:" <<  handle);
				goto error;
			}
		}
		else
		{
			LogDebug("ProcM2Ims::AllocatePlaybackSession - no valid info, not re-commutating, imsh:" <<  handle);
		}
		

		
		//
		// update map and send acknowledgment
		//
		
		_streamingObjectSet[handle] = ctx;
		ctx->session_handler	= req->session_handler;

		MsgStreamAllocateSessionAck *ack = 
			new MsgStreamAllocateSessionAck();

		DWORD time = ::GetTickCount();

		
		if (pt == NULL)
		{
			// prepare sdp with all supported codecs
			sdps << "v=0\r\n"			<<
				"o=m2streamer " << time << " " << time <<" IN IP4 " << ::inet_ntoa(_localInAddr) << "\r\n"
				"s=m2session\r\n"			<<
				"c=IN IP4 "	<< ::inet_ntoa(_localInAddr) << "\r\n" <<
				"t=0 0\r\n"	<<
				"m=audio "  << ctx->stream->session->rtp.loc_port << " RTP/AVP"   <<  _codecsListPostfix  << "\r\n"
				<< _rtpMapPostfix 
				<< "\r\n";
		}
		else
		{
			// prepare sdp with negotiated codec only
			sdps << "v=0\r\n"			<<
				"o=m2streamer " << time << " " << time <<" IN IP4 " << ::inet_ntoa(_localInAddr) << "\r\n"
				"s=m2session\r\n"			<<
				"c=IN IP4 "	<< ::inet_ntoa(_localInAddr) << "\r\n" <<
				"t=0 0\r\n"	<<
				"m=audio "  << ctx->stream->session->rtp.loc_port << " RTP/AVP "   << idx << "\r\n"
				<< "a=rtpmap:" << idx << " " << pt->mime_type << "/" <<  pt->clock_rate		<< "\r\n";

		}

	

		ack->streamer_handle  = handle;
		ack->offer.body = sdps.str();
		ack->offer.type = "application/sdp";


		ctx->state = IMS_ALLOCATED;
		SendResponse(req,ack);
		return;

error:
		SendResponse(req,new MsgStreamAllocateSessionNack());
		return;
	}



	ApiErrorCode 
	ProcM2Ims::StartTicking(StreamingCtxPtr ctx)
	{
		FUNCTRACKER;
		

		if (ctx->state == IMS_TICKING)
		{
			return API_SUCCESS;
		};

		if (ctx->stream->ticker == NULL)
		{
			ctx->stream->ticker = _ticker;
		};

		// final touch
		int res = ms_ticker_attach(ctx->stream->ticker,ctx->stream->soundread);
		if (res < 0)
		{
			return API_FAILURE;
		}

		res = ms_ticker_attach(ctx->stream->ticker,ctx->stream->rtprecv);
		if (res < 0)
		{
			return API_FAILURE;
		}

		ctx->state = IMS_TICKING;

		return API_SUCCESS;

	}

	ApiErrorCode 
	ProcM2Ims::RecommutateSession(
		IN StreamingCtxPtr ctx, 
		IN const CnxInfo &remoteInfo, 
		IN const PayloadType *mediaFormat,
		IN int idx)
	{
		
		

		StopTicking(ctx);

		RtpSession *rtps =  ctx->stream->session;

		// update remote end for session
		int remport  = remoteInfo.port_ho();
		char *remip  = (char *)remoteInfo.iptoa();

		int res = rtp_session_set_remote_addr(rtps,remip,remport);
		if (res < 0) 
		{
			LogWarn("error:rtp_session_set_remote_addr");
			goto error;
		}

		// update session payload type
		res = rtp_session_set_payload_type(rtps,idx);
		if (res < 0) 
		{
			LogWarn("error:rtp_session_set_payload_type " << idx);
			goto error;
		}

		// update session jitter compensation
		rtp_session_set_jitter_compensation(rtps,0/*jitt_comp*/);

		// attach queue to a session
		rtp_session_unregister_event_queue(rtps,_rtp_q);
		rtp_session_register_event_queue(rtps,_rtp_q);

		/********************
		*
		*		Receiver 
		*
		*********************/
		if (ctx->stream->rtprecv != NULL)
		{
			res = ms_filter_unlink(ctx->stream->rtprecv,0,ctx->stream->decoder,0);
			if (res < 0) 
			{
				LogWarn("error:ms_filter_unlink rtprecv->decoder");
				goto error;
			}
			ms_filter_destroy(ctx->stream->rtprecv);
			ctx->stream->rtprecv = NULL;
		}

		
		ctx->stream->rtprecv=ms_filter_new(MS_RTP_RECV_ID);
		if (ctx->stream->rtprecv == NULL)
		{
			LogWarn("error:ms_filter_new MS_RTP_RECV_ID");
			goto error;
		}
		res = ms_filter_call_method(ctx->stream->rtprecv,MS_RTP_RECV_SET_SESSION,rtps);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_call_method MS_RTP_RECV_SET_SESSION");
			goto error;
		}
		
			
			

		
		/********************
		*
		*		Decoder 
		*
		*********************/
		if (ctx->stream->decoder != NULL)
		{
			ms_filter_postprocess(ctx->stream->decoder);
			res = ms_filter_unlink(ctx->stream->decoder,0,ctx->stream->dtmfgen,0);
			if (res < 0) 
			{
				LogWarn("error:ms_filter_unlink rtprecv->dtmfgen");
				goto error;
			}
			ms_filter_destroy(ctx->stream->decoder);
			ctx->stream->decoder = NULL;
		}

		PayloadType *pt = rtp_profile_get_payload(_avProfile,idx);
		if (pt==NULL)
		{
			LogWarn("error:rtp_profile_get_payload " << idx);
			goto error;
		}

		ctx->stream->decoder=ms_filter_create_decoder(pt->mime_type);
		if (ctx->stream->decoder == NULL)
		{
			LogWarn("error:ms_filter_create_decoder " << pt->mime_type);
			goto error;
		}

		if (pt->send_fmtp!=NULL) 
		{
			res = ms_filter_call_method(ctx->stream->decoder,MS_FILTER_ADD_FMTP, (void*)pt->send_fmtp);
			if (res < 0) 
			{
				LogWarn("error:ms_filter_call_method decoder send_fmtp MS_FILTER_ADD_FMTP");
				goto error;
			}
		}

		if (pt->recv_fmtp!=NULL) 
		{
			res = ms_filter_call_method(ctx->stream->decoder,MS_FILTER_ADD_FMTP,(void*)pt->recv_fmtp);
			if (res < 0) 
			{
				LogWarn("error:ms_filter_call_method decoder recv_fmtp MS_FILTER_ADD_FMTP");
				goto error;
			}
		}


		/********************
		*
		*		Sender 
		*
		*********************/
		if (ctx->stream->rtpsend != NULL)
		{
			ms_filter_destroy(ctx->stream->rtpsend);
			ctx->stream->rtpsend = NULL;
		}

		ctx->stream->rtpsend =ms_filter_new(MS_RTP_SEND_ID);
		if (ctx->stream->rtpsend == NULL)
		{
			LogWarn("error:ms_filter_new MS_RTP_SEND_ID");
			goto error;
		}

		res = ms_filter_call_method(ctx->stream->rtpsend,MS_RTP_SEND_SET_SESSION,rtps);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_call_method MS_RTP_SEND_SET_SESSION");
			goto error;
		}

		/********************
		*
		*		Encoder 
		*
		*********************/
		if (ctx->stream->encoder != NULL)
		{
			res = ms_filter_unlink(ctx->stream->encoder,0,ctx->stream->rtpsend,0);
			if (res < 0) 
			{
				LogWarn("error:ms_filter_unlink encoder->rtpsend");
				goto error;
			}
			ms_filter_destroy(ctx->stream->encoder);
			ctx->stream->encoder = NULL;
		}

		
		ctx->stream->encoder = ms_filter_create_encoder(pt->mime_type);
		if (ctx->stream->encoder == NULL) 
		{
			LogWarn("error:ms_filter_create_encoder");
			goto error;
		}

		res = ms_filter_call_method(ctx->stream->encoder,MS_FILTER_SET_SAMPLE_RATE,&pt->clock_rate);
// 		if (res < 0) 
// 		{
// 			LogWarn("error:ms_filter_call_method(ctx->stream->encoder,MS_FILTER_SET_SAMPLE_RATE");
// 			goto error;
// 	
		if (pt->normal_bitrate>0){
			res = ms_filter_call_method(ctx->stream->encoder,MS_FILTER_SET_BITRATE,&pt->normal_bitrate);
// 			if (res < 0) 
// 			{
// 				LogWarn("error:ms_filter_call_method(ctx->stream->encoder,MS_FILTER_SET_BITRATE");
// 				goto error;
// 				
		}

		if (pt->send_fmtp!=NULL) 
		{
			res = ms_filter_call_method(ctx->stream->encoder,MS_FILTER_ADD_FMTP, (void*)pt->send_fmtp);
			if (res < 0) 
			{
				LogWarn("error:ms_filter_call_method(ctx->stream->encoder,MS_FILTER_ADD_FMTP");
				goto error;
			}
		}
			
			
		


		/********************
		*
		*		Reader 
		*
		*********************/
		if (ctx->stream->soundread != NULL)
		{
			ms_filter_destroy(ctx->stream->soundread);
			ctx->stream->soundread = NULL;
		}

		switch (ctx->snd_device_type)
		{

		case SND_DEVICE_TYPE_FILE:
			{
				ctx->stream->soundread = ms_filter_new(MS_FILE_PLAYER_ID);
				if (ctx->stream->soundread  == NULL) 
				{
					LogWarn("error:ms_filter_new(MS_FILE_PLAYER_ID)");
					goto error;
				};

				break;
			}
		case SND_DEVICE_TYPE_SND_CARD_MIC:
			{
				MSSndCard *sc = NULL;
				const char * card_id = NULL;
				
				sc = ms_snd_card_manager_get_card(ms_snd_card_manager_get(),card_id);
				if (sc == NULL) 
				{
					LogWarn("error:ms_snd_card_manager_get_card");
					goto error;
				}
				ctx->stream->soundread=ms_snd_card_create_reader(sc);
				if (sc == NULL) 
				{
					LogWarn("error:ms_snd_card_create_reader");
					goto error;
				}

				res = ms_filter_call_method(ctx->stream->soundread,MS_FILTER_SET_SAMPLE_RATE,&pt->clock_rate);
				if (res<0) 
				{
					LogWarn("error:ms_filter_call_method(MS_FILTER_SET_SAMPLE_RATE)(SND CARD) cr:" << pt->clock_rate);
					goto error;
				}

				int tmp = 1;
				ms_filter_call_method(ctx->stream->soundread,MS_FILTER_SET_NCHANNELS,&tmp);
				if (res<0) 
				{
					LogWarn("error:ms_filter_call_method(MS_FILTER_SET_NCHANNELS)(SND CARD) ch:" << tmp);
					goto error;
				}
				break;
			}
		default:
			{

			}
		}

		


		ctx->stream->soundread->notify = &on_file_filter_event;
		ctx->stream->soundread->notify_ud = (void*)ctx->streamer_handle;



		/********************
		*
		*		Sound Writer  
		*
		*********************/
		if (ctx->stream->soundwrite != NULL)
		{
			ms_filter_destroy(ctx->stream->soundwrite);
			ctx->stream->soundwrite = NULL;
		}

		switch (ctx->rcv_device_type)
		{
		
		case RCV_DEVICE_WINSND_WRITE:
			{
				MSSndCard *sc = NULL;
				const char * card_id = NULL;
				int tmp = -1;
				sc = ms_snd_card_manager_get_card(ms_snd_card_manager_get(),card_id);
				if (sc == NULL) 
				{
					LogWarn("error:ms_snd_card_manager_get_card");
					goto error;
				}
				ctx->stream->soundwrite=ms_snd_card_create_writer(sc);
				if (sc == NULL) 
				{
					LogWarn("error:ms_snd_card_create_writer");
					goto error;
				}

				ms_filter_call_method(ctx->stream->soundread,MS_FILTER_GET_SAMPLE_RATE,&tmp);
				ms_filter_call_method(ctx->stream->soundwrite,MS_FILTER_SET_SAMPLE_RATE,&tmp);
				ms_filter_call_method(ctx->stream->soundread,MS_FILTER_GET_NCHANNELS,&tmp);
				ms_filter_call_method(ctx->stream->soundwrite,MS_FILTER_SET_NCHANNELS,&tmp);
				break;

			}
		case RCV_DEVICE_FILE_REC_ID:
			{
				//we just do not open the file so it stays dummy
				ctx->stream->soundwrite=ms_filter_new(MS_FILE_REC_ID);
				if (ctx->stream->soundwrite == NULL) 
				{
					LogWarn("error:ms_filter_new(MS_FILE_REC_ID)");
					goto error;
				}

				ms_filter_call_method_noarg(ctx->stream->soundwrite,MS_FILE_REC_CLOSE);
				char buf[100];
				::itoa(ctx->streamer_handle,buf,10);
				string path = _conf->GetString("m2ims/record_dir") + "\\" + buf + ".wav";
				res = ms_filter_call_method(ctx->stream->soundwrite,MS_FILE_REC_OPEN,(void*)path.c_str());
				if (res<0) 
				{
					LogWarn("error:ms_filter_call_method(MS_FILE_REC_OPEN) name:" << path);
					goto error;
				}

				res =  ms_filter_call_method_noarg(ctx->stream->soundwrite,MS_FILE_REC_START);
				if (res<0) 
				{
					LogWarn("error:ms_filter_call_method(MS_FILE_REC_START) name:" << path);
					goto error;
				}
				break;

			}
		default:
			{
				//we just do not open the file so it stays dummy
				ctx->stream->soundwrite=ms_filter_new(MS_FILE_REC_ID);
				if (ctx->stream->soundwrite == NULL) 
				{
					LogWarn("error:ms_filter_new(MS_FILE_REC_ID)");
					goto error;
				}
			}
		}

	
		


		/*********************************************************************
		*
		*	file_reader -> encoder ->  +  -> rtp_sender 
		*							   |
		*           soundwrite	   <-  +  <- decoder <- rtp_receiver
		*
		***********************************************************************/

		res = ms_filter_link(ctx->stream->soundread,0,ctx->stream->encoder,0);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_link soundread->encoder");
			goto error;
		}

		res = ms_filter_link(ctx->stream->encoder,0,ctx->stream->rtpsend,0);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_link encoder->rtpsend");
			goto error;
		}

// 		res = ms_filter_link(ctx->stream->dtmfgen,0,ctx->stream->soundwrite,0);
// 		if (res < 0) 
// 		{
// 			LogWarn("error:ms_filter_link dtmfgen->soundwrite");
// 			goto error;
// 		}

		res = ms_filter_link(ctx->stream->rtprecv,0,ctx->stream->decoder,0);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_link rtprecv->decoder");
			goto error;
		}

// 		res = ms_filter_link(ctx->stream->decoder,0,ctx->stream->dtmfgen,0);
// 		if (res < 0) 
// 		{
// 			LogWarn("error:ms_filter_link decoder->dtmfgen");
// 			goto error;
// 		}

		res = ms_filter_link(ctx->stream->decoder,0,ctx->stream->soundwrite,0);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_link decoder->soundwrite");
			goto error;
		}

		return API_SUCCESS;

error:
		return API_FAILURE;


	}

	ApiErrorCode 
	ProcM2Ims::StopTicking(StreamingCtxPtr ctx)
	{
		FUNCTRACKER;
		

		if (ctx->state != IMS_TICKING)
		{
			return API_SUCCESS;
		};

		if (ctx->stream->ticker == NULL)
		{
			return API_SUCCESS;
		};

		AudioStream *stream = ctx->stream;

		if (stream->ticker)
		{
			int res = ms_ticker_detach(stream->ticker,stream->soundread);
			if (res < 0)
			{
				LogWarn("mserror: ms_ticker_detach soundread, res:" << res );
				return API_FAILURE;
			}

			 res = ms_ticker_detach(stream->ticker,stream->rtprecv);
			if (res < 0)
			{
				LogWarn("mserror: ms_ticker_detach rtprecv, res:" << res );
				return API_FAILURE;
			}


		}

		ctx->state = IMS_STOPPED;

		return API_SUCCESS;

	}

	void 
	ProcM2Ims::ModifySession(IwMessagePtr msg)
	{

		FUNCTRACKER;

		SdpParser::Medium medium;
		StreamingCtxPtr ctx;
		
		shared_ptr<MsgStreamModifyReq> req  =
			dynamic_pointer_cast<MsgStreamModifyReq> (msg);

		LogDebug("ProcM2Ims::ModifySession imsh:" << req->streamer_handle);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->streamer_handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("ProcM2Ims::ModifySession invalid imsh:" << req->streamer_handle);
			goto error;
		}

		ctx	= iter->second;
		if (ctx->pt != NULL)
		{
			LogWarn("ProcM2Ims::ModifySession sessions parameters were set already imsh:" << req->streamer_handle);
			goto error;
		}

		if (ctx->pt == NULL && req->offer.body.empty())
		{
			LogWarn("ProcM2Ims::ModifySession sdp is invalid");
			goto error;
		}

		{
			SdpParser parser(req->offer.body);
			medium = parser.first_audio_medium();

			if (!medium.connection.is_valid() || medium.list.empty())
			{
				LogWarn("erro parsing sdp imsh:" << ctx->streamer_handle);
				goto error;
			}

			LogDebug("ProcM2Ims::ModifySession medium connection:" << medium.connection.ipporttos());
		}

		// update remote end for session
		int remport  = medium.connection.port_ho();
		char *remip  = (char *)medium.connection.iptoa();

		int idx = -1;
		PayloadType *pt = NULL;
		NegotiateCodec(medium.list,&pt,&idx);
		if (pt == NULL)
		{
			LogWarn("Error negotiating codecs");
			goto error;
		}
		ctx->pt = pt;

		// to prevent race conditions?
		ApiErrorCode err = StopTicking(ctx);
		if (IW_FAILURE(err))
		{
			LogWarn("Error detaching from ticker imsh:" << ctx->streamer_handle);
			goto error;
		}

		RtpSession *rtps = ctx->stream->session;

		int res = rtp_session_set_remote_addr(rtps,remip,remport);
		if (res < 0) 
		{
			LogWarn("error:rtp_session_set_remote_addr");
			goto error;
		}

		err = RecommutateSession(
			ctx,
			CnxInfo(remip,remport),
			pt,
			idx);

		if (IW_FAILURE(err))
		{
			LogWarn("Error recommutating imsh:" << ctx->streamer_handle);
			goto error;
		}

		err = StartTicking(ctx);
		if (IW_FAILURE(err))
		{
			LogWarn("Error attaching to ticker imsh:" << ctx->streamer_handle);
			goto error;
		}

		SendResponse(req, new MsgStreamModifyAck());

		return;

error:
		SendResponse(req, new MsgStreamModifyNack());

	}



	void 
	ProcM2Ims::StartPlayback(IwMessagePtr msg)
	{

		FUNCTRACKER;
		

		shared_ptr<MsgStreamPlayReq> req  =
			dynamic_pointer_cast<MsgStreamPlayReq> (msg);

		StreamingCtxsMap::iterator iter = _streamingObjectSet.end();

		iter = _streamingObjectSet.find(req->streamer_handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("StartPlayback:: Invalid imsh:" << req->streamer_handle);
			SendResponse(msg, new MsgStreamPlayNack());
			return;
		}

		StreamingCtxPtr ctx		= iter->second;

		StopTicking(ctx);

		// close it anyway
		int res = -1;

		// the session wasn't really created
		if (ctx->stream == NULL || ctx->stream->soundread == NULL)
		{
			LogWarn("session wasn't created imsh:" << req->streamer_handle);
			SendResponse(msg, new MsgStreamPlayNack());
			return;
		}

		switch (ctx->snd_device_type)
		{
		case SND_DEVICE_TYPE_FILE:
			{
				string filename = req->file_name;

				//
				// Check if file exists
				//
				WIN32_FIND_DATAA FindFileData;
				HANDLE hFind = NULL; 
				hFind = ::FindFirstFileA(filename.c_str(), &FindFileData);
				if (hFind == INVALID_HANDLE_VALUE) 
				{
					LogDebug("file:" << filename << " not found. Trying relative path...");

					// relative path?
					filename = _conf->GetString("m2ims/sounds_dir")+ "\\" + req->file_name;
					hFind = ::FindFirstFileA(filename.c_str(), &FindFileData);
					if (hFind == INVALID_HANDLE_VALUE) 
					{
						LogWarn("file:" << filename << " not found.");
						SendResponse(msg, new MsgStreamPlayNack());
						return;
					}
					else
					{
						BOOL res = FALSE;
						res = ::FindClose(hFind);
						if (res == FALSE)
						{
							LogCrit("::CloseHandle");
							throw;
						}
					}
				} 
				else
				{
					BOOL res = FALSE;
					res = ::FindClose(hFind);
					if (res == FALSE)
					{
						LogCrit("::CloseHandle");
						throw;
					}
				}



				char buffer[1024];
				buffer[0] = '\0';
				DWORD res_len = 0;
				res_len=::GetFullPathNameA(filename.c_str(),1024,buffer,NULL);
				if (res_len <= 0)
				{
					LogSysError("::GetFullPathNameA");
					SendResponse(msg, new MsgStreamPlayNack());
					return;
				}

				
				filename = buffer;
				LogDebug("StartPlayback:: Play file name:" << filename << ", loop:" << req->loop << ", imsh:" << req->streamer_handle 
					<< ", dst:" << ::inet_ntoa(ctx->stream->session->rtp.rem_addr.sin_addr)<< ":" << ::ntohs(ctx->stream->session->rtp.rem_addr.sin_port));

				

				res = ms_filter_call_method_noarg(ctx->stream->soundread,MS_FILE_PLAYER_CLOSE);
				if (res < 0)
				{
					LogWarn("mserror:ms_filter_call_method_noarg MS_FILE_PLAYER_CLOSE imsh:" << req->streamer_handle);
					SendResponse(msg, new MsgStreamPlayNack());
					return;
				}

				res = ms_filter_call_method(ctx->stream->soundread,MS_FILE_PLAYER_OPEN,(void*)filename.c_str());
				if (res < 0)
				{
					LogWarn("mserror:ms_filter_call_method MS_FILE_PLAYER_OPEN imsh:" << req->streamer_handle);
					SendResponse(msg, new MsgStreamPlayNack());
					return;
				}

				res = ms_filter_call_method_noarg(ctx->stream->soundread,MS_FILE_PLAYER_START);
				if (res < 0)
				{
					LogWarn("mserror:ms_filter_call_method_noarg MS_FILE_PLAYER_START imsh:" << req->streamer_handle);
					SendResponse(msg, new MsgStreamPlayNack());
					return;
				}

				int tmp = 0;
				res = ms_filter_call_method(ctx->stream->soundread,MS_FILTER_GET_SAMPLE_RATE, &tmp);
				if (res < 0)
				{
					LogWarn("mserror:ms_filter_call_method MS_FILTER_GET_SAMPLE_RATE imsh:" << req->streamer_handle);
					SendResponse(msg, new MsgStreamPlayNack());
					return;
				};

				if (tmp != ctx->pt->clock_rate)
				{
					LogWarn("clock rates of payload and wav file are different pt:" << ctx->pt->clock_rate << " file:" << tmp << " imsh:" << req->streamer_handle);
				}

				res = ms_filter_call_method(ctx->stream->soundwrite,MS_FILTER_SET_SAMPLE_RATE,&tmp);
				if (res < 0)
				{
					LogWarn("mserror:ms_filter_call_method MS_FILTER_SET_SAMPLE_RATE imsh:" << req->streamer_handle);
					SendResponse(msg, new MsgStreamPlayNack());
					return;
				};



				int loop_param = req->loop ? 0 : -2;
				res = ms_filter_call_method(ctx->stream->soundread,MS_FILE_PLAYER_LOOP, &loop_param);
				if (res < 0)
				{
					LogWarn("mserror:ms_filter_call_method MS_FILE_PLAYER_LOOP, imsh:" << req->streamer_handle);
					SendResponse(msg, new MsgStreamPlayNack());
					return;
				}

				ctx->loop = req->loop;
				break;
			}
		case SND_DEVICE_TYPE_SND_CARD_LINE:
			{

			}
		default:
			{

			}
		}

		
		
		ApiErrorCode iw_res = API_FAILURE;
		iw_res = StartTicking(ctx);
		if (IW_FAILURE(iw_res))
		{
			LogWarn("Unable to srart ticking on iwh:" << ctx->streamer_handle);
			SendResponse(msg, new MsgStreamPlayNack());
			return;
		}

		
		MsgStreamPlayAck *rsp = new MsgStreamPlayAck();
		rsp->correlation_id = ++_correlationCounter;
		ctx->correlation_id = _correlationCounter;

		SendResponse(msg, rsp);
		
		ctx->last_user_request = req;
	}

	void 
	ProcM2Ims::UponPlaybackStopped(ImsOverlapped *ovlp)
	{
		FUNCTRACKER;
		

		auto_ptr<ImsOverlapped> args = auto_ptr<ImsOverlapped>(ovlp);
		
		
		StreamingCtxsMap::iterator iter = _streamingObjectSet.find(ovlp->streamer_handle_id);
		if (iter == _streamingObjectSet.end())
		{
			LogWarn("UponPlaybackStopped:: Invalid imsh:" << ovlp->streamer_handle_id);
			return;
		}

		StreamingCtxPtr ctx = iter->second;

		// ignore eof event if playback is looped
		if (ctx->loop)
		{
			return;
		}

		MsgStreamPlayStopped *stopped_msg = new MsgStreamPlayStopped();
		stopped_msg->streamer_handle = ovlp->streamer_handle_id; 
		stopped_msg->correlation_id = ctx->correlation_id;

		this->SendMessage(ctx->session_handler.inbound, IwMessagePtr(stopped_msg));
	}

	void
	ProcM2Ims::TearDown(IwMessagePtr msg)
	{
		FUNCTRACKER;
		

		shared_ptr<MsgStreamTearDownReq> req  =
			dynamic_pointer_cast<MsgStreamTearDownReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->streamer_handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("TearDown:: Invalid imsh:" << req->streamer_handle);
			return;
		}

		LogDebug("TearDown:: imsh:" << req->streamer_handle);

		StreamingCtxPtr ctx = iter->second;
		
		TearDown(ctx);
		
		
		// ctx dtor should release all associated resources
		_streamingObjectSet.erase(iter);
		
	}

	void 
	ProcM2Ims::TearDown(StreamingCtxPtr ctx)
	{
		FUNCTRACKER;
		

		AudioStream *stream = ctx->stream;

		if (stream->ticker)
		{
			if (stream->soundread)	ms_ticker_detach(stream->ticker,stream->soundread);
			if (stream->rtprecv) ms_ticker_detach(stream->ticker,stream->rtprecv);
				
			
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

			if (stream->rtpsend) ms_filter_unlink(stream->encoder,0,stream->rtpsend,0);
			if (stream->decoder) ms_filter_unlink(stream->rtprecv,0,stream->decoder,0);
			if (stream->dtmfgen) ms_filter_unlink(stream->decoder,0,stream->dtmfgen,0);
		}

		if (stream->soundread)
		{
			switch (ctx->snd_device_type)
			{
			case SND_DEVICE_TYPE_FILE:
				{
					ms_filter_call_method_noarg(stream->soundread,MS_FILE_PLAYER_STOP);
					ms_filter_call_method_noarg(stream->soundread,MS_FILE_PLAYER_CLOSE);
					break;
				}
			case SND_DEVICE_TYPE_SND_CARD_MIC:
				{
					break;
				}
			default:;

			};
			
		};
		
	}


	void 
	ProcM2Ims::StopPlayback(IwMessagePtr msg)
	{
		FUNCTRACKER;
		
		shared_ptr<MsgStreamStopPlayReq> req  =
		 	dynamic_pointer_cast<MsgStreamStopPlayReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->streamer_handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("StopPlayback:: Invalid imsh:" << req->streamer_handle);
			return;
		}

		LogDebug("StopPlayback:: imsh:" << req->streamer_handle);

		StreamingCtxPtr ctx = iter->second;

		// session was never commutated
		if (ctx->stream->soundread == NULL)
		{
			return;
		}

		int res = ms_filter_call_method_noarg(ctx->stream->soundread,MS_FILE_PLAYER_STOP);
		if (res == -1)
		{
			LogWarn("mserror:ms_filter_call_method_noarg  MS_FILE_PLAYER_STOP, handle:" << req->streamer_handle);
			return;
		}
		
		
	}

	void 
	ProcM2Ims::TearDownAllSessions()
	{
		FUNCTRACKER;

		for(StreamingCtxsMap::iterator iter = _streamingObjectSet.begin();
			iter != _streamingObjectSet.end();
			iter++)
		{
			TearDown(iter->second);
		}

		_streamingObjectSet.clear();
	}
}

