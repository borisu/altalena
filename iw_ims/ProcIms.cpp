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
#include "Call.h"

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
			{
				LogInfo(buf);
				break;
			}
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
		state(IMS_INITIAL)
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
#define IW_IMS_DTMF_EVENT 2

	static void 
	on_dtmf_received(RtpSession *s, int dtmf, void * user_data)
	{

		MSFilter *filter = (MSFilter *) user_data;
		ImsHandleId handle = (ImsHandleId) filter->notify_ud;

		ImsOverlapped *olap = new ImsOverlapped();
		SecureZeroMemory(olap, sizeof(ImsOverlapped));
		olap->ims_handle_id = handle;
		olap->dtmf = dtmf;

		BOOL res = ::PostQueuedCompletionStatus(
			g_iocpHandle,				//A handle to an I/O completion port to which the I/O completion packet is to be posted.
			0,							//The value to be returned through the lpNumberOfBytesTransferred parameter of the GetQueuedCompletionStatus function.
			IW_IMS_DTMF_EVENT,			//The value to be returned through the lpCompletionKey parameter of the GetQueuedCompletionStatus function.
			olap						//The value to be returned through the lpOverlapped parameter of the GetQueuedCompletionStatus function.
			);

		if (res == FALSE)
		{
			LogSysError("::PostQueuedCompletionStatus");
			throw;
		}
	}

	static void 
	on_file_filter_event(void *userdata , unsigned int id, void *arg)
	{

		ImsHandleId handle = (ImsHandleId) userdata;

		switch (id)
		{
		case MS_FILE_PLAYER_EOF:
			{
			
				ImsOverlapped *olap = new ImsOverlapped();
				SecureZeroMemory(olap, sizeof(ImsOverlapped));
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

			}
		default:
			{
				LogWarn("Unknown rtp event:" << id);
			}
		}
	}




	ProcIms::ProcIms(IN LpHandlePair pair, IN Configuration &conf)
		:LightweightProcess(pair, IMS_Q, "Ims"),
		_conf(conf),
		_localMedia(conf.ImsCnxInfo()),
		_rtp_q(NULL),
		_portManager(conf.ImsTopPort(),conf.ImsBottomPort()),
		_rtpWorkerShutdownEvt(NULL),
		_avProfile(NULL),
		_ticker(NULL)
	{
		FUNCTRACKER;

		_iocpPtr = IocpInterruptorPtr(new IocpInterruptor());
		_inbound->HandleInterruptor(_iocpPtr);


		_payloadTypeMap["PCMA"] = &payload_type_pcma8000;
		_payloadTypeMap["PCMU"] = &payload_type_pcmu8000;
		_payloadTypeMap["telephone-event"]  = &payload_type_telephone_event;

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
				LogWarn("Ims encountered configured media format that is not supported " << *media_format);
				continue;
			}

			rtp_profile_set_payload(
				_avProfile,
				media_format->sdp_mapping(),
				(*ms_map_iter).second
				);

			LogDebug("Ims added media format " << *media_format);
		}

	}

	void
	ProcIms::real_run()
	{
		FUNCTRACKER;

		WSADATA dat;
		if (WSAStartup(MAKEWORD(2,2),&dat)!=0)
		{
			LogCrit("Error starting up WIN socket");
			throw;
		}

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
				_iocpPtr->Handle(),		// A handle to the completion port. To create a completion port, use the CreateIoCompletionPort function.
				&number_of_bytes,		// A pointer to a variable that receives the number of bytes transferred during an I/O operation that has completed.
				&completion_key,		// A pointer to a variable that receives the completion key value associated with the file handle whose I/O operation has completed. A completion key is a per-file key that is specified in a call to CreateIoCompletionPort.
				&lpOverlapped,			// A pointer to a variable that receives the address of the OVERLAPPED structure that was specified when the completed I/O operation was started. 
				IW_DEFAULT_IMS_TIMEOUT // The number of milliseconds that the caller is willing to wait for a completion packet to appear at the completion port. If a completion packet does not appear within the specified time, the function times out, returns FALSE, and sets *lpOverlapped to NULL.
				);

			IX_PROFILE_CHECK_INTERVAL(10000);

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
					LogDebug("Ims keep alive.");
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
			case IW_IMS_DTMF_EVENT:
				{
					UponDtmfEvent((ImsOverlapped*)lpOverlapped);
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
			case MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST:
				{
					AllocatePlaybackSession(ptr);
					break;
				}
			case MSG_START_PLAYBACK_REQUEST:
				{
					StartPlayback(ptr);
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
			case MSG_IMS_SEND_RFC2833DTMF_REQ:
				{
					SendDtmf(ptr);
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

		shared_ptr<MsgAllocateImsSessionReq> req  =
			dynamic_pointer_cast<MsgAllocateImsSessionReq> (msg);

		//
		// create new context
		//
		StreamingCtxPtr ctx(new StreamingCtx());
		long handle = GetNewImsHandle();

		//
		// create ortp stream and initialize it with available port
		//
		int local_port = 0;
		do 
		{
			local_port = _portManager.GetNextPortFromPool();
			if (local_port == IW_UNDEFINED)
			{
				break;
			};

			// creates stream and session
			ctx->stream = audio_stream_new(local_port, ms_is_ipv6(_localMedia.iptoa()));
			if (ctx->stream!=NULL)
			{
				ctx->port = local_port;
				break;
			}

			_portManager.ReturnPortToPool(local_port);

		} while (ctx->stream!=NULL);

		// failed to find available port
		if (ctx->stream == NULL)
		{
			LogWarn("Failed to find available port");
			goto error;
		}

		LogDebug("Allocated local ims address:" << _localMedia.ipporttos() );

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

		// update remote end for session
		int remport  = req->remote_media_data.port_ho();
		char *remip  = (char *)req->remote_media_data.iptoa();
		int res = rtp_session_set_remote_addr(rtps,remip,remport);
		if (res < 0) 
		{
			LogWarn("error:rtp_session_set_remote_addr");
			goto error;
		}

		// update session payload type
		res = rtp_session_set_payload_type(rtps,req->codec.sdp_mapping());
		if (res < 0) 
		{
			LogWarn("error:rtp_session_set_payload_type " << req->codec.sdp_mapping_tos());
			goto error;
		}

		// update session jitter compensation
		rtp_session_set_jitter_compensation(rtps,0/*jitt_comp*/);

		// attach queue to a session
		rtp_session_register_event_queue(rtps,_rtp_q);

		/********************
		*
		*		Receiver 
		*
		*********************/
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
		PayloadType *pt = rtp_profile_get_payload(_avProfile,req->codec.sdp_mapping());
		if (pt==NULL)
		{
			LogWarn("error:rtp_profile_get_payload " << req->codec.sdp_mapping_tos());
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
		*		DTMF 
		*
		*********************/
		ctx->stream->dtmfgen=ms_filter_new(MS_DTMF_GEN_ID);
		if (ctx->stream->dtmfgen == NULL)
		{
			LogWarn("error:ms_filter_new MS_DTMF_GEN_ID");
			goto error;
		}

		
		ctx->stream->dtmfgen->notify_ud = (void*)handle;

		
		res = rtp_session_signal_connect(rtps,"telephone-event",(RtpCallback)on_dtmf_received,(unsigned long)ctx->stream->dtmfgen);
		if (res < 0) 
		{
			LogWarn("error:rtp_session_signal_connect");
			goto error;
		}

		/********************
		*
		*		Sender 
		*
		*********************/
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

		ctx->stream->soundwrite=ms_filter_new(MS_FILE_REC_ID);
		if (ctx->stream->soundwrite == NULL) 
		{
			LogWarn("error:ms_filter_new(MS_FILE_REC_ID)");
			goto error;
		}

		g_iocpHandle= _iocpPtr->Handle();
		
		
		ctx->stream->soundread->notify_ud = (void*)handle;
		

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

		res = ms_filter_link(ctx->stream->dtmfgen,0,ctx->stream->soundwrite,0);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_link dtmfgen->soundwrite");
			goto error;
		}

		res = ms_filter_link(ctx->stream->rtprecv,0,ctx->stream->decoder,0);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_link rtprecv->decoder");
			goto error;
		}

		res = ms_filter_link(ctx->stream->decoder,0,ctx->stream->dtmfgen,0);
		if (res < 0) 
		{
			LogWarn("error:ms_filter_link decoder->dtmfgen");
			goto error;
		}


		//
		// update map and send acknowledgment
		//
	
		_streamingObjectSet[handle] = ctx;
		ctx->session_handler	= req->session_handler;

		MsgAllocateImsSessionAck *ack = 
			new MsgAllocateImsSessionAck();

		ack->playback_handle = handle;
		ack->ims_media_data = CnxInfo(_localMedia.iptoa(),local_port);

		ctx->state = IMS_ALLOCATED;
		SendResponse(req,ack);
		return;

error:
		SendResponse(req,new MsgAllocateImsSessionNack());
		return;
	}

	ApiErrorCode 
	ProcIms::StartTicking(StreamingCtxPtr ctx)
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
	ProcIms::StopTicking(StreamingCtxPtr ctx)
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
	ProcIms::StartPlayback(IwMessagePtr msg)
	{

		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		shared_ptr<MsgStartPlayReq> req  =
			dynamic_pointer_cast<MsgStartPlayReq> (msg);

		LogDebug("StartPlayback:: Play file name:" << req->file_name << ", loop:" << req->loop << ", imsh:" << req->playback_handle);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->playback_handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("StartPlayback:: Invalid imsh:" << req->playback_handle);
			SendResponse(msg, new MsgStartPlayReqNack());
			return;
		}

		StreamingCtxPtr ctx		= iter->second;

		StopTicking(ctx);

		// close it anyway
		int res = ms_filter_call_method_noarg(ctx->stream->soundread,MS_FILE_PLAYER_CLOSE);
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method_noarg MS_FILE_PLAYER_CLOSE imsh:" << req->playback_handle);
			SendResponse(msg, new MsgStartPlayReqNack());
			return;
		}

		res = ms_filter_call_method(ctx->stream->soundread,MS_FILE_PLAYER_OPEN,(void*)req->file_name.c_str());
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method MS_FILE_PLAYER_OPEN imsh:" << req->playback_handle);
			SendResponse(msg, new MsgStartPlayReqNack());
			return;
		}

		res = ms_filter_call_method_noarg(ctx->stream->soundread,MS_FILE_PLAYER_START);
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method_noarg MS_FILE_PLAYER_START imsh:" << req->playback_handle);
			SendResponse(msg, new MsgStartPlayReqNack());
			return;
		}

		int tmp = 0;
		res = ms_filter_call_method(ctx->stream->soundread,MS_FILTER_GET_SAMPLE_RATE, &tmp);
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method MS_FILTER_GET_SAMPLE_RATE imsh:" << req->playback_handle);
			SendResponse(msg, new MsgStartPlayReqNack());
			return;
		};

		res = ms_filter_call_method(ctx->stream->soundwrite,MS_FILTER_SET_SAMPLE_RATE,&tmp);
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method MS_FILTER_SET_SAMPLE_RATE imsh:" << req->playback_handle);
			SendResponse(msg, new MsgStartPlayReqNack());
			return;
		};

		

		int loop_param = req->loop ? 0 : -2;
		res = ms_filter_call_method(ctx->stream->soundread,MS_FILE_PLAYER_LOOP, &loop_param);
		if (res < 0)
		{
			LogWarn("mserror:ms_filter_call_method MS_FILE_PLAYER_LOOP, imsh:" << req->playback_handle);
			SendResponse(msg, new MsgStartPlayReqNack());
			return;
		}

		ctx->loop = req->loop;
		
		ApiErrorCode iw_res = StartTicking(ctx);
		if (IW_FAILURE(iw_res))
		{
			SendResponse(msg, new MsgStartPlayReqNack());
			return;
		}

		if (req->send_provisional)
		{
			SendResponse(msg, new MsgStartPlayReqAck());
		}

		ctx->last_user_request = req;
	}

	void
	ProcIms::SendDtmf(IwMessagePtr msg)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		shared_ptr<MsgImsSendRfc2833DtmfReq> req  =
			dynamic_pointer_cast<MsgImsSendRfc2833DtmfReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("Invalid ims:" << req->handle);
			return;
		}

		StreamingCtxPtr ctx = iter->second;

		ApiErrorCode iw_res = StartTicking(ctx);
		if (IW_FAILURE(iw_res))
		{
			return;
		}

		LogDebug("Send dtmf:" << (char)req->dtmf_digit << ", ims:" << req->handle);

		if (ctx->stream->rtpsend)
		{
			int res = ms_filter_call_method(ctx->stream->rtpsend,MS_RTP_SEND_SEND_DTMF,&req->dtmf_digit);
			if (res == -1)
			{
				LogWarn("mserror:ms_filter_call_method  MS_RTP_SEND_SEND_DTMF dtmf:" << req->dtmf_digit << ", handle:" << req->handle);
				return;
			}
		} 
		else
		{
			LogWarn("mserror:ms_filter_call_method  MS_RTP_SEND_SEND_DTMF rtpsend is NULL.");
			return;
		}


		if (ctx->stream->dtmfgen)
		{
			int res = ms_filter_call_method(ctx->stream->dtmfgen,MS_DTMF_GEN_PUT,&req->dtmf_digit);
			if (res == -1)
			{
				LogWarn("mserror:ms_filter_call_method  MS_DTMF_GEN_PUT dtmf:" << req->dtmf_digit << ", handle:" << req->handle);
				return;
			}
		}
		else
		{
			LogWarn("mserror:ms_filter_call_method  MS_DTMF_GEN_PUT dtmfgen is NULL.");
			return;
		}
			

	}


	void
	ProcIms::UponDtmfEvent(ImsOverlapped* ovlp)
	{
		FUNCTRACKER;
		auto_ptr<ImsOverlapped> args = auto_ptr<ImsOverlapped>(ovlp);
		StreamingCtxsMap::iterator iter = _streamingObjectSet.find(ovlp->ims_handle_id);
		if (iter == _streamingObjectSet.end())
		{
			LogWarn("UponDtmfEvent:: Invalid imsh:" << ovlp->ims_handle_id);
			return;
		}

		StreamingCtxPtr ctx = iter->second;

		MsgImsRfc2833DtmfEvt *evt = new MsgImsRfc2833DtmfEvt();
		evt->dtmf_digit = ovlp->dtmf;

		this->SendMessage(ctx->session_handler.inbound, IwMessagePtr(evt));
		

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
		stopped_msg->playback_handle = ovlp->ims_handle_id; 

		this->SendResponse(ctx->last_user_request, stopped_msg);
	}

	void
	ProcIms::TearDown(IwMessagePtr msg)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		shared_ptr<MsgImsTearDownReq> req  =
			dynamic_pointer_cast<MsgImsTearDownReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("TearDown:: Invalid imsh:" << req->handle);
			return;
		}

		LogDebug("TearDown:: imsh:" << req->handle);

		StreamingCtxPtr ctx = iter->second;
		
		TearDown(ctx);
		_portManager.ReturnPortToPool(ctx->port);
		
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
			ms_ticker_detach(stream->ticker,stream->soundread);
			ms_ticker_detach(stream->ticker,stream->rtprecv);

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
			ms_filter_unlink(stream->rtprecv,0,stream->decoder,0);
			ms_filter_unlink(stream->decoder,0,stream->dtmfgen,0);
		}

		ms_filter_call_method_noarg(stream->soundread,MS_FILE_PLAYER_STOP);
		ms_filter_call_method_noarg(stream->soundread,MS_FILE_PLAYER_CLOSE);


	}


	void 
	ProcIms::StopPlayback(IwMessagePtr msg)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		shared_ptr<MsgStopPlaybackReq> req  =
		 	dynamic_pointer_cast<MsgStopPlaybackReq> (msg);

		StreamingCtxsMap::iterator iter = 
			_streamingObjectSet.find(req->handle);

		if (iter == _streamingObjectSet.end())
		{
			LogWarn("StopPlayback:: Invalid imsh:" << req->handle);
			return;
		}

		LogDebug("StopPlayback::, imsh:" << req->handle);

		StreamingCtxPtr ctx = iter->second;
		
		ApiErrorCode iw_res = StopTicking(ctx);
		if (IW_FAILURE(iw_res))
		{
			SendResponse(req,new MsgStopPlaybackNack());
			return;
		}

		SendResponse(req,new MsgStopPlaybackAck());
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

