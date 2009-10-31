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
#include "ImsSession.h"


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
		ims_handle(IW_UNDEFINED),
		correlation_id(IW_UNDEFINED)
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

		ImsHandle handle = (ImsHandle) userdata;

		switch (id)
		{
		case MS_FILE_PLAYER_EOF:
			{
			
				ImsOverlapped *olap = new ImsOverlapped();
				olap->ims_handle_id = handle;

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

	ProcIms::ProcIms(IN LpHandlePair pair, IN Configuration &conf)
		:LightweightProcess(pair, IMS_Q, "Ims"),
		_conf(conf),
		_rtp_q(NULL),
		_rtpWorkerShutdownEvt(NULL),
		_avProfile(NULL),
		_ticker(NULL),
		_correlationCounter(0)
	{
		FUNCTRACKER;

		_iocpPtr = IocpInterruptorPtr(new IocpInterruptor());
		_inbound->HandleInterruptor(_iocpPtr);


		_payloadTypeMap["PCMA"]  = &payload_type_pcma8000;
		_payloadTypeMap["PCMU"]  = &payload_type_pcmu8000;
		_payloadTypeMap["SPEEX"] = &payload_type_speex_nb;
		
		

	}

	ProcIms::~ProcIms(void)
	{
		FUNCTRACKER;

		CleanUpResources();
		
	}

	void
	ProcIms::CleanUpResources()
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
	ProcIms::InitCodecs()
	{
		for (MediaFormatsPtrList::const_iterator conf_iter = _conf.MediaFormats().begin(); conf_iter != _conf.MediaFormats().end(); conf_iter++)
		{
			
			const MediaFormat *media_format = *(conf_iter);

			PayloadTypeMap::iterator ms_map_iter = _payloadTypeMap.find(media_format->sdp_name_tos());
			if (ms_map_iter == _payloadTypeMap.end())
			{
				LogWarn("ProcIms::InitCodecs media format not supported :" << *media_format);
				continue;
			}

			if (_payloadIndexMap.find(media_format->sdp_name_tos()) != _payloadIndexMap.end())
			{
				LogWarn("ProcIms::InitCodecs media format appeared twice (not registering):" << *media_format);
				continue;
			}

			rtp_profile_set_payload(
				_avProfile,
				media_format->sdp_mapping(),
				(*ms_map_iter).second
				);

			_payloadIndexMap[media_format->sdp_name_tos()] = media_format->sdp_mapping();

			LogDebug("Ims added media format :" << *media_format);
		}

	}

	void
	ProcIms::real_run()
	{
		FUNCTRACKER;

		WSADATA dat;
		if (WSAStartup(MAKEWORD(2,2),&dat)!=0)
		{
			LogCrit("Error starting up WIN socket.");
			return;
		}

		
		g_iocpHandle= _iocpPtr->WinHandle();

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

		LogInfo("Ims process started successfully.");
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


			ApiErrorCode err_code = API_SUCCESS;
			IwMessagePtr ptr =  _inbound->Wait(Seconds(0), err_code);

			switch (ptr->message_id)
			{
			case MSG_IMS_ALLOCATE_SESSION_REQ:
				{
					AllocatePlaybackSession(ptr);
					break;
				}
			case MSG_IMS_MODIFY_REQ:
				{
					ModifySession(ptr);
					break;
				}
			case MSG_IMS_PLAY_REQUEST:
				{
					StartPlayback(ptr);
					break;
				}
			case MSG_IMS_STOP_PLAY_REQ:
				{
					StopPlayback(ptr);
					break;
				}

			case MSG_IMS_TEARDOWN_REQ:
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
						LogCrit("Received unknown OOB msg:" << ptr->message_id);
						throw;
					}// if
				}// default
			}// switch
		}// while

		TearDownAllSessions();

		::SetEvent(_rtpWorkerShutdownEvt);
		::Sleep(100);

		END_FORKING_REGION
		WSACleanup();
	}


	void 
	ProcIms::AllocatePlaybackSession(IwMessagePtr msg)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		shared_ptr<MsgImsAllocateSessionReq> req  =
			dynamic_pointer_cast<MsgImsAllocateSessionReq> (msg);

		//
		// create new context
		//
		StreamingCtxPtr ctx(new StreamingCtx());
		long handle = GetNewImsHandle();
		ctx->ims_handle = handle;

		//
		// create ortp stream and initialize it with available port
		//
		int local_port = req->local_media_data.port_ho();
		if (local_port == IW_UNDEFINED)
		{
			LogWarn("Failed to find available port");
			goto error;
		};

		// creates stream and session
		ctx->stream = audio_stream_new(local_port, ms_is_ipv6(req->local_media_data.iptoa()));
		if (ctx->stream!=NULL)
		{
			ctx->port = local_port;
	
		} 
		else 
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

		ApiErrorCode res = API_FAILURE;
		if (req->remote_media_data.is_ip_valid() && 
			req->remote_media_data.is_port_valid() &&
			req->codec.get_media_type() != MediaFormat::MediaType_UNKNOWN)
		{
			res = RecommutateSession(ctx,req->remote_media_data, req->codec);
			if (IW_FAILURE(res))
			{
				LogDebug("ProcIms::AllocatePlaybackSession - error re-commutating, imsh:" <<  handle);
				goto error;
			}
		}
		else
		{
			LogDebug("ProcIms::AllocatePlaybackSession - no valid info, not re-commutating, imsh:" <<  handle);
		}

		//
		// update map and send acknowledgment
		//
	
		_streamingObjectSet[handle] = ctx;
		ctx->session_handler	= req->session_handler;

		MsgImsAllocateSessionAck *ack = 
			new MsgImsAllocateSessionAck();

		ack->ims_handle = handle;
		ctx->state = IMS_ALLOCATED;
		SendResponse(req,ack);
		return;

error:
		SendResponse(req,new MsgImsAllocateSessionNack());
		return;
	}



	ApiErrorCode 
	ProcIms::StartTicking(StreamingCtxPtr ctx)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

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
	ProcIms::RecommutateSession(
		IN StreamingCtxPtr ctx, 
		IN const CnxInfo &remoteInfo, 
		IN const MediaFormat &mediaFormat)
	{
		
		IX_PROFILE_FUNCTION();

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
		res = rtp_session_set_payload_type(rtps,mediaFormat.sdp_mapping());
		if (res < 0) 
		{
			LogWarn("error:rtp_session_set_payload_type " << mediaFormat.sdp_mapping_tos());
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

		PayloadType *pt = rtp_profile_get_payload(_avProfile,mediaFormat.sdp_mapping());
		if (pt==NULL)
		{
			LogWarn("error:rtp_profile_get_payload " << mediaFormat.sdp_mapping_tos());
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
			res = ms_filter_call_method(ctx->stream->encoder,MS_FILTER_ADD_FMTP, (void*)pt->send_fmtp);
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

		ctx->stream->soundread = ms_filter_new(MS_FILE_PLAYER_ID);
		if (ctx->stream->soundread  == NULL) 
		{
			LogWarn("error:ms_filter_new(MS_FILE_PLAYER_ID)");
			goto error;
		}


		ctx->stream->soundread->notify = &on_file_filter_event;



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

		ctx->stream->soundwrite=ms_filter_new(MS_FILE_REC_ID);
		if (ctx->stream->soundwrite == NULL) 
		{
			LogWarn("error:ms_filter_new(MS_FILE_REC_ID)");
			goto error;
		}

		ctx->stream->soundread->notify_ud = (void*)ctx->ims_handle;


		/*********************************************************************
		*
		*	file_reader -> encoder ->  +  -> rtp_sender 
		*                 dtmf_gen <-> |
		*           file_recorder  <-  +  <- decoder <- rtp_receiver
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

		return API_SUCCESS;

error:
		return API_FAILURE;


	}

	ApiErrorCode 
	ProcIms::StopTicking(StreamingCtxPtr ctx)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

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
	ProcIms::ModifySession(IwMessagePtr msg)
	{

		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		shared_ptr<MsgImsModifyReq> req  =
			dynamic_pointer_cast<MsgImsModifyReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->ims_handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("ProcIms::ModifySession invalid imsh:" << req->ims_handle);
			SendResponse(msg, new MsgImsModifyNack());
			return;
		}

		StreamingCtxPtr ctx		= iter->second;


		// to ensure that we using the same mapping take from the table
		PayloadIndexMap::iterator payload_iter =  _payloadIndexMap.find(req->codec.sdp_name_tos());
		if (payload_iter == _payloadIndexMap.end())
		{
			LogWarn("ProcIms::ModifySession unsupported media:" << req->codec);
			SendResponse(msg, new MsgImsModifyNack());
			return;
		}

		
		// to prevent race conditions?
		ApiErrorCode err = StopTicking(ctx);
		if (IW_FAILURE(err))
		{
			LogWarn("Error detaching from ticker imsh:" << ctx->ims_handle);
			goto error;
		}

		// update remote end for session
		int remport  = req->remote_media_data.port_ho();
		char *remip  = (char *)req->remote_media_data.iptoa();
		
		RtpSession *rtps = ctx->stream->session;

		int res = rtp_session_set_remote_addr(rtps,remip,remport);
		if (res < 0) 
		{
			LogWarn("error:rtp_session_set_remote_addr");
			goto error;
		}

		LogDebug("ProcIms::ModifySession imsh:" << req->ims_handle << 
			", remip:" << remip  << 
			", port:"  << remport <<
			", codec(map):" << req->codec.sdp_mapping() << 
			", codec(name):" << req->codec.sdp_name_tos() );

		err = RecommutateSession(
			ctx,
			req->remote_media_data,
			req->codec);

		err = StartTicking(ctx);
		if (IW_FAILURE(err))
		{
			LogWarn("Error attaching to ticker imsh:" << ctx->ims_handle);
			goto error;
		}

		SendResponse(req, new MsgImsModifyAck());

		return;


error:

		SendResponse(req, new MsgImsModifyNack());

	}



	void 
	ProcIms::StartPlayback(IwMessagePtr msg)
	{

		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		shared_ptr<MsgImsPlayReq> req  =
			dynamic_pointer_cast<MsgImsPlayReq> (msg);

		string filename = req->file_name;

		//
		// Check if file exists
		//
		WIN32_FIND_DATAA FindFileData;
		HANDLE hFind = NULL; 
		hFind = ::FindFirstFileA(filename.c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) 
		{
			// relative path?
			filename = _conf.SoundsPath()+ "\\" + req->file_name;
			hFind = ::FindFirstFileA(filename.c_str(), &FindFileData);
			if (hFind == INVALID_HANDLE_VALUE) 
			{
				LogWarn("file:" << filename << " not found.");
				SendResponse(msg, new MsgImsPlayNack());
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
			SendResponse(msg, new MsgImsPlayNack());
			return;
		}

		filename = buffer;
		LogDebug("StartPlayback:: Play file name:" << filename << ", loop:" << req->loop << ", imsh:" << req->ims_handle);

		StreamingCtxsMap::iterator iter = _streamingObjectSet.end();

		iter = _streamingObjectSet.find(req->ims_handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("StartPlayback:: Invalid imsh:" << req->ims_handle);
			SendResponse(msg, new MsgImsPlayNack());
			return;
		}

		StreamingCtxPtr ctx		= iter->second;

		StopTicking(ctx);

		// close it anyway
		int res = -1;

		// the session wasn't really created
		if (ctx->stream == NULL || ctx->stream->soundread == NULL)
		{
			LogWarn("session wasn't created imsh:" << req->ims_handle);
			SendResponse(msg, new MsgImsPlayNack());
			return;
		}

		res = ms_filter_call_method_noarg(ctx->stream->soundread,MS_FILE_PLAYER_CLOSE);
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method_noarg MS_FILE_PLAYER_CLOSE imsh:" << req->ims_handle);
			SendResponse(msg, new MsgImsPlayNack());
			return;
		}

		res = ms_filter_call_method(ctx->stream->soundread,MS_FILE_PLAYER_OPEN,(void*)filename.c_str());
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method MS_FILE_PLAYER_OPEN imsh:" << req->ims_handle);
			SendResponse(msg, new MsgImsPlayNack());
			return;
		}

		res = ms_filter_call_method_noarg(ctx->stream->soundread,MS_FILE_PLAYER_START);
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method_noarg MS_FILE_PLAYER_START imsh:" << req->ims_handle);
			SendResponse(msg, new MsgImsPlayNack());
			return;
		}

		int tmp = 0;
		res = ms_filter_call_method(ctx->stream->soundread,MS_FILTER_GET_SAMPLE_RATE, &tmp);
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method MS_FILTER_GET_SAMPLE_RATE imsh:" << req->ims_handle);
			SendResponse(msg, new MsgImsPlayNack());
			return;
		};

		res = ms_filter_call_method(ctx->stream->soundwrite,MS_FILTER_SET_SAMPLE_RATE,&tmp);
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method MS_FILTER_SET_SAMPLE_RATE imsh:" << req->ims_handle);
			SendResponse(msg, new MsgImsPlayNack());
			return;
		};

		

		int loop_param = req->loop ? 0 : -2;
		res = ms_filter_call_method(ctx->stream->soundread,MS_FILE_PLAYER_LOOP, &loop_param);
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method MS_FILE_PLAYER_LOOP, imsh:" << req->ims_handle);
			SendResponse(msg, new MsgImsPlayNack());
			return;
		}

		ctx->loop = req->loop;
		
		ApiErrorCode iw_res = API_FAILURE;
		iw_res = StartTicking(ctx);
		if (IW_FAILURE(iw_res))
		{
			LogWarn("Unable to srart ticking on iwh:" << ctx->ims_handle);
			SendResponse(msg, new MsgImsPlayNack());
			return;
		}

		
		MsgImsPlayAck *rsp = new MsgImsPlayAck();
		rsp->correlation_id = ++_correlationCounter;
		ctx->correlation_id = _correlationCounter;

		SendResponse(msg, rsp);
		
		ctx->last_user_request = req;
	}

	void 
	ProcIms::UponPlaybackStopped(ImsOverlapped *ovlp)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		auto_ptr<ImsOverlapped> args = auto_ptr<ImsOverlapped>(ovlp);
		
		
		StreamingCtxsMap::iterator iter = _streamingObjectSet.find(ovlp->ims_handle_id);
		if (iter == _streamingObjectSet.end())
		{
			LogWarn("UponPlaybackStopped:: Invalid imsh:" << ovlp->ims_handle_id);
			return;
		}

		StreamingCtxPtr ctx = iter->second;

		// ignore eof event if playback is looped
		if (ctx->loop)
		{
			return;
		}

		MsgImsPlayStopped *stopped_msg = new MsgImsPlayStopped();
		stopped_msg->ims_handle = ovlp->ims_handle_id; 
		stopped_msg->correlation_id = ctx->correlation_id;

		this->SendMessage(ctx->session_handler.inbound, IwMessagePtr(stopped_msg));
	}

	void
	ProcIms::TearDown(IwMessagePtr msg)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		shared_ptr<MsgImsTearDownReq> req  =
			dynamic_pointer_cast<MsgImsTearDownReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->ims_handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("TearDown:: Invalid imsh:" << req->ims_handle);
			return;
		}

		LogDebug("TearDown:: imsh:" << req->ims_handle);

		StreamingCtxPtr ctx = iter->second;
		
		TearDown(ctx);
		
		
		// ctx dtor should release all associated resources
		_streamingObjectSet.erase(iter);
		
	}

	void 
	ProcIms::TearDown(StreamingCtxPtr ctx)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

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
			ms_filter_call_method_noarg(stream->soundread,MS_FILE_PLAYER_STOP);
			ms_filter_call_method_noarg(stream->soundread,MS_FILE_PLAYER_CLOSE);
		}
		
	}


	void 
	ProcIms::StopPlayback(IwMessagePtr msg)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		shared_ptr<MsgImsStopPlayReq> req  =
		 	dynamic_pointer_cast<MsgImsStopPlayReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->ims_handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("StopPlayback:: Invalid imsh:" << req->ims_handle);
			return;
		}

		LogDebug("StopPlayback:: imsh:" << req->ims_handle);

		StreamingCtxPtr ctx = iter->second;

		// session was never commutated
		if (ctx->stream->soundread == NULL)
		{
			return;
		}

		int res = ms_filter_call_method_noarg(ctx->stream->soundread,MS_FILE_PLAYER_STOP);
		if (res == -1)
		{
			LogWarn("mserror:ms_filter_call_method_noarg  MS_FILE_PLAYER_STOP, handle:" << req->ims_handle);
			return;
		}
		
		
	}

	void 
	ProcIms::TearDownAllSessions()
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

