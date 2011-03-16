#include "StdAfx.h"
#include "ProcUniMrcp.h"
#include "Logger.h"
#include "MrcpUtils.h"


// 5 secs
#define IW_MRCP_CLIENT_TIMEOUT		5000

// 1 min
#define	IW_DEFAULT_MRCP_TIMEOUT		60000 


#define IW_MRCP_CHANNEL_ADD_EVT			5000
#define IW_MRCP_TERMINATE_RSP			5001
#define IW_MRCP_MESSAGE_RECEIVE_EVT		5002
#define IW_MRCP_TERMINATE_EVT			5003

namespace ivrworx
{
	
	static HANDLE g_iocpHandle = NULL;

	static int GetNewMrcpHandle()
	{
		static volatile int handle_counter = 8000000;
		return handle_counter++;
	}

	MrcpSessionCtx::MrcpSessionCtx()
		:state(MRCP_INITIAL),
		mrcp_handle(IW_UNDEFINED),
		session(NULL),
		synthesizer_channel(NULL),
		recognizer_channel(NULL),
		last_message(NULL),
		rtp_desc(NULL)
	{

	}

	MrcpSessionCtx::~MrcpSessionCtx()
	{
		

	}

	void send_overlapped_message(MrcpOverlapped *olap, int event_id)
	{
		FUNCTRACKER;

		BOOL res = ::PostQueuedCompletionStatus(
			g_iocpHandle,				//A handle to an I/O completion port to which the I/O completion packet is to be posted.
			0,							//The value to be returned through the lpNumberOfBytesTransferred parameter of the GetQueuedCompletionStatus function.
			event_id,					//The value to be returned through the lpCompletionKey parameter of the GetQueuedCompletionStatus function.
			olap						//The value to be returned through the lpOverlapped parameter of the GetQueuedCompletionStatus function.
			);

		if (res == FALSE)
		{
			LogSysError("::PostQueuedCompletionStatus");
			throw std::exception("System error in iw_application_on_channel_add");
		}

	}

	apt_bool_t iw_on_terminate_event(
		mrcp_application_t *application, 
		mrcp_session_t *session,
		mrcp_session_descriptor_t * desc,
		mrcp_sig_status_code_e status)
	{
		FUNCTRACKER;

		MrcpOverlapped *olap = new MrcpOverlapped();
		olap->mrcp_handle_id = (int)((mrcp_client_session_t*)session)->app_obj;
		olap->session = session;
		//olap->channel = channel;

		send_overlapped_message(olap,IW_MRCP_TERMINATE_EVT);

		return TRUE;
	}


	apt_bool_t iw_application_on_session_terminate(
		mrcp_application_t *application, 
		mrcp_session_t *session, 
		mrcp_sig_status_code_e status)
	{
		FUNCTRACKER;

		MrcpOverlapped *olap = new MrcpOverlapped();
		olap->mrcp_handle_id = (int)((mrcp_client_session_t*)session)->app_obj;
		olap->status = status;
		olap->session = session;

		send_overlapped_message(olap,IW_MRCP_TERMINATE_RSP);

		return TRUE;
	}

	apt_bool_t iw_application_on_channel_add(
		mrcp_application_t *application, 
		mrcp_session_t *session, 
		mrcp_channel_t *channel, 
		mrcp_sig_status_code_e status)
	{
		FUNCTRACKER;
		
		MrcpOverlapped *olap = new MrcpOverlapped();
		olap->mrcp_handle_id = (int)((mrcp_client_session_t*)session)->app_obj;
		olap->status = status;
		olap->channel = channel;
		olap->session = session;

		send_overlapped_message(olap,IW_MRCP_CHANNEL_ADD_EVT);
		
		return TRUE;
	}

	apt_bool_t iw_application_on_message_receive(
		mrcp_application_t *application, 
		mrcp_session_t *session, 
		mrcp_channel_t *channel, 
		mrcp_message_t *message)
	{

		MrcpOverlapped *olap = new MrcpOverlapped();
		olap->mrcp_handle_id = (int)((mrcp_client_session_t*)session)->app_obj;
		olap->message = message;
		olap->channel = channel;
		olap->session = session;

		send_overlapped_message(olap,IW_MRCP_MESSAGE_RECEIVE_EVT);


		return TRUE;
	}

	static const mrcp_app_message_dispatcher_t iw_application_dispatcher = {
		NULL,									/** Response to mrcp_application_session_update()request */
		iw_application_on_session_terminate,	/** Response to mrcp_application_session_terminate()request */
		iw_application_on_channel_add,			/** Response to mrcp_application_channel_add() request */
		NULL,									/** Response to mrcp_application_channel_remove() request */
		iw_application_on_message_receive,		/** Response (event) to mrcp_application_message_send() request */
		NULL,									/** Event indicating client stack is started and ready to process requests from the application */
		iw_on_terminate_event,					/** Event indicating unexpected session/synthesizer_channel termination */
	};

	apt_bool_t app_message_handler(const mrcp_app_message_t *app_message)
	{
		/* app_message should be dispatched now,
		*  the default dispatcher is used in demo. */
		return mrcp_application_message_dispatch(&iw_application_dispatcher,app_message);
		
	}

	
	ProcUniMrcp::ProcUniMrcp(LpHandlePair pair, ConfigurationPtr conf):
	LightweightProcess(pair,"ProcUniMrcp"),
	_conf(conf),
	_application(NULL),
	_pool(NULL),
	_logInititiated(FALSE),
	_mrcpClient(NULL)
	{
		FUNCTRACKER;

		ServiceId(_conf->GetString("unimrcp/uri"));

		_iocpPtr = IocpInterruptorPtr(new IocpInterruptor());
		_inbound->HandleInterruptor(_iocpPtr);

		g_iocpHandle = _iocpPtr->WinHandle();
	}

	void 
	ProcUniMrcp::real_run()
	{

		if (IW_FAILURE(Init()))
		{
			LogWarn("Error initializing UniMRCP infrastructure");
			return;
		}

		LogInfo("Mrcp process started successfully.");
		I_AM_READY;

		BOOL shutdown_flag = FALSE;
		while (shutdown_flag == FALSE)
		{

			DWORD number_of_bytes    = 0;
			ULONG_PTR completion_key = 0;
			LPOVERLAPPED lpOverlapped = NULL;

			BOOL res = ::GetQueuedCompletionStatus(
				_iocpPtr->WinHandle(),		// A handle to the completion port. To create a completion port, use the CreateIoCompletionPort function.
				&number_of_bytes,			// A pointer to a variable that receives the number of bytes transferred during an I/O operation that has completed.
				&completion_key,			// A pointer to a variable that receives the completion key value associated with the file handle whose I/O operation has completed. A completion key is a per-file key that is specified in a call to CreateIoCompletionPort.
				&lpOverlapped,				// A pointer to a variable that receives the address of the OVERLAPPED structure that was specified when the completed I/O operation was started. 
				IW_DEFAULT_MRCP_TIMEOUT		// The number of milliseconds that the caller is willing to wait for a completion packet to appear at the completion port. If a completion packet does not appear within the specified time, the function times out, returns FALSE, and sets *lpOverlapped to NULL.
				);

			//IX_PROFILE_CHECK_INTERVAL(10000);

			// error during overlapped I/O?
			int last_err = ::GetLastError();
			if (res == FALSE)
			{
				if (last_err != WAIT_TIMEOUT)
				{
					LogSysError("::GetQueuedCompletionStatus");
					Destroy();
					return;
				}
				else 
				{
					LogInfo("Mrcp keep alive.");
					continue;
				}
			}

			// oRTP event?
			switch (completion_key)
			{
			case IW_MRCP_CHANNEL_ADD_EVT:
				{
					onMrcpChanndelAddEvt((MrcpOverlapped*)lpOverlapped);
					continue;
				}
			case IW_MRCP_MESSAGE_RECEIVE_EVT:
				{
					onMrcpMessageReceived((MrcpOverlapped*)lpOverlapped);
					continue;
				}
			case IW_MRCP_TERMINATE_RSP:
				{
					onMrcpSessionTerminatedRsp((MrcpOverlapped*)lpOverlapped);
					continue;
				}
			case IW_MRCP_TERMINATE_EVT:
				{
					onMrcpSessionTerminatedEvt((MrcpOverlapped*)lpOverlapped);
					continue;
				}
			case IOCP_UNIQUE_COMPLETION_KEY:
				{
					ApiErrorCode err_code = API_SUCCESS;
					IwMessagePtr ptr = _inbound->Wait(Seconds(0), err_code);

					switch (ptr->message_id)
					{
					case MSG_MRCP_ALLOCATE_SESSION_REQ:
						{
							UponMrcpAllocateSessionReq(ptr);
							break;
						}
					case MSG_MRCP_SPEAK_REQ:
						{
							UponSpeakReq(ptr);
							break;
						}
					case MSG_MRCP_STOP_SPEAK_REQ:
						{
							UponStopSpeakReq(ptr);
							break;
						}
					case MSG_MRCP_TEARDOWN_REQ:
						{
							UponTearDownReq(ptr);
							break;
						}
					case MSG_MRCP_RECOGNIZE_REQ:
						{
							UponRecognizeReq(ptr);
							break;
						}
					case MSG_PROC_SHUTDOWN_REQ:
						{
							shutdown_flag = TRUE;
							SendResponse(ptr, new MsgShutdownAck());
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
					break;
				}// case
			default:
				{
					
					shutdown_flag = TRUE;
					LogCrit("Unknown overlapped structure received");
				} // default
			}// case
		}// while

		Destroy();

	}

	/** Create demo RTP termination descriptor */
	mpf_rtp_termination_descriptor_t* rtp_descriptor_create(apr_pool_t *pool, const CnxInfo &local_info,const MediaFormat &media_format, mpf_stream_direction_e direction)
	{
		mpf_codec_descriptor_t *codec_descriptor;
		mpf_rtp_media_descriptor_t *media;
		/* create rtp descriptor */
		mpf_rtp_termination_descriptor_t *rtp_descriptor = 
			(mpf_rtp_termination_descriptor_t *)apr_palloc(pool,sizeof(mpf_rtp_termination_descriptor_t));
		mpf_rtp_termination_descriptor_init(rtp_descriptor);
		/* create rtp local media */
		media = (mpf_rtp_media_descriptor_t *)apr_palloc(pool,sizeof(mpf_rtp_media_descriptor_t));
		mpf_rtp_media_descriptor_init(media);
		apt_string_assign(&media->ip,local_info.iptoa(),pool);
		media->port = local_info.port_ho();
		media->state = MPF_MEDIA_ENABLED;
		media->direction = direction;

		
		/* initialize codec list */
		mpf_codec_list_init(&media->codec_list,1,pool);

		

		if (media_format.get_media_type() == MediaFormat::MediaType_SPEECH)
		{
			/* set another codec descriptor */
			codec_descriptor = mpf_codec_list_add(&media->codec_list);
			if(codec_descriptor) {
				codec_descriptor->payload_type = media_format.sdp_mapping();
				apt_string_assign(&codec_descriptor->name,media_format.sdp_name_tos().c_str(),pool);
				codec_descriptor->sampling_rate = media_format.sampling_rate();
				codec_descriptor->channel_count = 1;
			}
		}
		

		rtp_descriptor->audio.local = media;
		return rtp_descriptor;
	}

	void
	ProcUniMrcp::UponTearDownReq(IwMessagePtr msg)
	{
		FUNCTRACKER;


		shared_ptr<MsgMrcpTearDownReq> req  =
			dynamic_pointer_cast<MsgMrcpTearDownReq> (msg);

		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(req->mrcp_handle);
		if (iter == _mrcpCtxMap.end())
		{

			LogWarn("ProcUniMrcp::UponTearDownReq - non existent ctx.");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		MrcpSessionCtxPtr ctx = (*iter).second;

		if (ctx->session)
		{
			mrcp_application_session_terminate(ctx->session);
		}

		FinalizeSessionContext(ctx);

	}

	void
	ProcUniMrcp::UponStopSpeakReq(IwMessagePtr msg)
	{
		FUNCTRACKER;


		shared_ptr<MsgMrcpStopSpeakReq> req  =
			dynamic_pointer_cast<MsgMrcpStopSpeakReq> (msg);

		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(req->mrcp_handle);
		if (iter == _mrcpCtxMap.end())
		{

			LogWarn("ProcUniMrcp::UponSpeakReq - non existent mrcph:" << req->mrcp_handle);
			SendResponse(req, new MsgMrcpStopSpeakNack());
			return;
		}

		MrcpSessionCtxPtr ctx = (*iter).second;
		ctx->last_user_request = req;

		if (ctx->state != MRCP_ALLOCATED)
		{
			LogWarn("ProcUniMrcp::UponSpeakReq - mrcph:" << ctx->mrcp_handle << " is not in allocated state.");
			SendResponse(req, new MsgMrcpStopSpeakNack());
			return;
		}

		//
		// create MRCP message 
		//
		mrcp_message_t *mrcp_message = 
			mrcp_application_message_create(
			ctx->session,
			ctx->synthesizer_channel,
			SYNTHESIZER_STOP);

		if(!mrcp_message) 
		{
			LogWarn("ProcUniMrcp::UponSpeakReq - error:mrcp_application_message_create");
			SendResponse(req, new MsgMrcpStopSpeakNack());
			return ;
		}

		//
		// get/allocate generic header 
		//
		mrcp_generic_header_t *generic_header = (mrcp_generic_header_t *)mrcp_generic_header_prepare(mrcp_message);
		if(!generic_header) 
		{
			LogWarn("ProcUniMrcp::UponSpeakReq - error:mrcp_generic_header_prepare");
			SendResponse(req, new MsgMrcpStopSpeakNack());
			return;
		}

		//
		// send MRCP request (non-blocking, asynchronous processing) 
		//
		ctx->last_message = mrcp_message;
		apt_bool_t res = mrcp_application_message_send(ctx->session,ctx->synthesizer_channel,mrcp_message);


		if (!res)
		{
			LogWarn("ProcUniMrcp::UponSpeakReq - error(speak):mrcp_application_message_send");
			SendResponse(req, new MsgMrcpStopSpeakNack());
			return;
		}

		// ack will be sent when the request is confirmed from UniMRCP side	
	
	}

	void 
	ProcUniMrcp::UponRecognizeReq(IN IwMessagePtr msg)
	{
		shared_ptr<MsgMrcpRecognizeReq> req  =
			dynamic_pointer_cast<MsgMrcpRecognizeReq> (msg);

		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(req->mrcp_handle);
		if (iter == _mrcpCtxMap.end())
		{

			LogWarn("ProcUniMrcp::UponRecognizeReq - non existent mrcph:" << req->mrcp_handle);
			SendResponse(req, new MsgMrcpRecognizeNack());
			return;
		}

		MrcpSessionCtxPtr ctx = (*iter).second;
		ctx->last_user_request = req;

		if (ctx->state != MRCP_ALLOCATED || ctx->recognizer_channel == NULL)
		{
			LogWarn("ProcUniMrcp::UponRecognizeReq - mrcph:" << ctx->mrcp_handle << " is not in allocated state.");
			SendResponse(req, new MsgMrcpRecognizeNack());
			return;
		};

		//
		// create MRCP message 
		//
		mrcp_message_t *mrcp_message = 
			mrcp_application_message_create(
			ctx->session,
			ctx->recognizer_channel,
			RECOGNIZER_RECOGNIZE);

		if(!mrcp_message) 
		{
			LogWarn("ProcUniMrcp::UponRecognizeReq - error(recognize):mrcp_application_message_create");
			SendResponse(req, new MsgMrcpRecognizeNack());
			return ;
		}

		//
		// get/allocate generic header 
		//
		mrcp_generic_header_t *generic_header = (mrcp_generic_header_t *)mrcp_generic_header_prepare(mrcp_message);
		if(!generic_header) 
		{
			LogWarn("ProcUniMrcp::UponRecognizeReq - error(speak):mrcp_generic_header_prepare");
			SendResponse(req, new MsgMrcpRecognizeNack());
			return;
		}

		

		//
		// get/allocate recognizer header 
		//
		mrcp_recog_header_t *recog_header = (mrcp_recog_header_t *)mrcp_resource_header_prepare(mrcp_message);
		if(!recog_header) 
		{
			LogWarn("ProcUniMrcp::UponRecognizeReq - error(speak):mrcp_resource_header_prepare");
			SendResponse(req, new MsgMrcpRecognizeNack());
			return;
		}

		try 
		{
			translate_generic_params_into_message(mrcp_message,generic_header,&req->params);
			translate_recog_params_into_message(mrcp_message,recog_header,&req->params);
		} catch (std::exception &e)
		{
			LogWarn("ProcUniMrcp::UponRecognizeReq - bad param exception" << e.what());
			SendResponse(req, new MsgMrcpRecognizeNack());
			return;
		}

	
		apt_string_assign_n(
			&mrcp_message->body,
			req->body.c_str(),
			req->body.size(),
			mrcp_message->pool);


		//
		// send MRCP request (non-blocking, asynchronous processing) 
		//
		ctx->last_message = mrcp_message;
		apt_bool_t res = mrcp_application_message_send(ctx->session,ctx->recognizer_channel,mrcp_message);

		if (!res)
		{
			LogWarn("ProcUniMrcp::UponSpeakReq - error(speak):mrcp_application_message_send");
			SendResponse(req, new MsgMrcpRecognizeNack());
			return;
		}



	}

	void
	ProcUniMrcp::UponSpeakReq(IwMessagePtr msg)
	{
		FUNCTRACKER;
		

		shared_ptr<MsgMrcpSpeakReq> req  =
			dynamic_pointer_cast<MsgMrcpSpeakReq> (msg);

		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(req->mrcp_handle);
		if (iter == _mrcpCtxMap.end())
		{
			
			LogWarn("ProcUniMrcp::UponSpeakReq - non existent mrcph:" << req->mrcp_handle);
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		MrcpSessionCtxPtr ctx = (*iter).second;
		ctx->last_user_request = req;

		if (ctx->state != MRCP_ALLOCATED)
		{
			LogWarn("ProcUniMrcp::UponSpeakReq - mrcph:" << ctx->mrcp_handle << " is not in allocated state.");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		//
		// create MRCP message 
		//
		mrcp_message_t *mrcp_message = 
			mrcp_application_message_create(
			ctx->session,
			ctx->synthesizer_channel,
			SYNTHESIZER_SPEAK);

		if(!mrcp_message) 
		{
			LogWarn("ProcUniMrcp::UponSpeakReq - error(speak):mrcp_application_message_create");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return ;
		}

		//
		// get/allocate generic header 
		//
		mrcp_generic_header_t *generic_header = (mrcp_generic_header_t *)mrcp_generic_header_prepare(mrcp_message);
		if(!generic_header) 
		{
			LogWarn("ProcUniMrcp::UponSpeakReq - error(speak):mrcp_generic_header_prepare");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		

		//
		// get/allocate synthesizer header 
		//
		mrcp_synth_header_t *synth_header = (mrcp_synth_header_t *)mrcp_resource_header_prepare(mrcp_message);
		if(!synth_header) 
		{
			LogWarn("ProcUniMrcp::UponSpeakReq - error(speak):mrcp_resource_header_prepare");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		translate_generic_params_into_message(mrcp_message,generic_header,&req->params);
		translate_synth_params_into_message(mrcp_message,synth_header,&req->params);

		apt_string_assign_n(
			&mrcp_message->body,
			req->body.c_str(),
			req->body.size(),
			mrcp_message->pool);

	
		//
		// send MRCP request (non-blocking, asynchronous processing) 
		//
		ctx->last_message = mrcp_message;
		apt_bool_t res = mrcp_application_message_send(ctx->session,ctx->synthesizer_channel,mrcp_message);
		
		if (!res)
		{
			LogWarn("ProcUniMrcp::UponSpeakReq - error(speak):mrcp_application_message_send");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		//SendResponse(req, new MsgMrcpSpeakAck());

	}

	

	void
	ProcUniMrcp::UponMrcpAllocateSessionReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

		MediaFormat media_format;
		MediaFormatsList::iterator mfiter;
		

		shared_ptr<MsgMrcpAllocateSessionReq> req  =
			dynamic_pointer_cast<MsgMrcpAllocateSessionReq> (msg);



		LogDebug("ProcUniMrcp::UponMrcpAllocateSessionReq " <<req->offer.body)

		long handle = IW_UNDEFINED;

		MrcpSessionCtxPtr ctx;

		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(req->mrcp_handle);
		if (iter != _mrcpCtxMap.end())
		{
			handle = req->mrcp_handle;
			ctx = iter->second;
			switch (req->resource)
			{
			case RECOGNIZER:
				{
					if (ctx->recognizer_channel != NULL)
					{
						LogWarn("ProcUniMrcp::UponMrcpAllocateSessionReq - recognizer_channel already exists");
						SendResponse(req,new MsgMrcpAllocateSessionNack());
						return;
					}
					break;
				}
			case SYNTHESIZER:
				{
					if (ctx->synthesizer_channel != NULL)
					{
						LogWarn("ProcUniMrcp::UponMrcpAllocateSessionReq - synthesizer_channel already exists");
						SendResponse(req,new MsgMrcpAllocateSessionNack());
						return;
					}
					break;
				}
			default:
				{
					LogWarn("ProcUniMrcp::UponMrcpAllocateSessionReq - unknown resource" <<req->resource);
					SendResponse(req,new MsgMrcpAllocateSessionNack());
					return;
				}
			}
		} 
		else
		{
			ctx = MrcpSessionCtxPtr(new MrcpSessionCtx());
			handle = GetNewMrcpHandle();
			ctx->mrcp_handle = handle;
			ctx->session_handler = req->session_handler;
		}

		ctx->state = MRCP_CONNECTING;
		ctx->last_user_request = req;

		if (!req->offer.body.empty() && ctx->session != NULL)
		{
			LogWarn("ProcUniMrcp::UponMrcpAllocateSessionReq - invalid local connection info");
			SendResponse(req,new MsgMrcpAllocateSessionNack());
		}

	
		/* create session */
		mrcp_session_t *session = ctx->session ? ctx->session :
			mrcp_application_session_create(_application,_conf->GetString("unimrcp/unimrcp_client_profile").c_str(), (void *)handle);

		if (!session)
		{
			LogWarn("error:mrcp_application_session_create");
			SendResponse(req,new MsgMrcpAllocateSessionNack());
			return;
		}

		ctx->session = session;

		mpf_stream_direction_e direction;

		if (req->resource == RECOGNIZER)
			direction = STREAM_DIRECTION_SEND;
		else	
			direction = STREAM_DIRECTION_RECEIVE;

		SdpParser p(req->offer.body);
		SdpParser::Medium  m = p.first_audio_medium();
		if (!m.connection.is_valid())
		{
			LogWarn("ProcUniMrcp::UponMrcpAllocateSessionReq - Wrong sdp");
			SendResponse(req,new MsgMrcpAllocateSessionNack());
			goto allocate_error;
		}


		// find first supported codec

		mfiter = m.list.begin();
		for (; mfiter != m.list.end(); mfiter++)
		{
			MediaFormat &curr_format = *mfiter; 
			if (curr_format.get_media_type() != MediaFormat::MediaType_DTMF)
			{
				media_format = curr_format;
				break;
			}
		}

		
		if (mfiter == m.list.end())
		{
			LogWarn("ProcUniMrcp::UponMrcpAllocateSessionReq - Wrong sdp - media format");
			SendResponse(req,new MsgMrcpAllocateSessionNack());
			goto allocate_error;
		}



		if (m.list.size() > 1)
		{
			LogWarn("Supporting only one codec in body, codec chosen:" << media_format.get_sdp_a());
		}

		ctx->media_format = media_format;

		mpf_rtp_termination_descriptor_t *rtp_descriptor = 	req->offer.body.empty() ? 
									NULL:
									rtp_descriptor_create(session->pool,m.connection,m.list.front(),direction);

		mrcp_channel_t *channel = NULL;
		
		switch (req->resource)
		{
		case RECOGNIZER:
			{
				/* create synthesizer channel */
				channel = mrcp_application_channel_create(
					session,                     /* session, channel belongs to */
					MRCP_RECOGNIZER_RESOURCE,    /* MRCP resource identifier */
					NULL /*termination*/,        /* media termination, used to terminate audio stream */
					rtp_descriptor,              /* RTP descriptor, used to create RTP termination (NULL by default) */
					NULL);                       /* object to associate */

				ctx->recognizer_channel = channel;
				break;

			}
		case SYNTHESIZER:
			{
				/* create synthesizer channel */
				channel = mrcp_application_channel_create(
					session,                     /* session, channel belongs to */
					MRCP_SYNTHESIZER_RESOURCE,   /* MRCP resource identifier */
					NULL /*termination*/,        /* media termination, used to terminate audio stream */
					rtp_descriptor,              /* RTP descriptor, used to create RTP termination (NULL by default) */
					NULL);                       /* object to associate */

				ctx->synthesizer_channel = channel;
				break;

			}
		default:
			{
				LogWarn("ProcUniMrcp::UponMrcpAllocateSessionReq - unknown resource" <<req->resource);
				SendResponse(req,new MsgMrcpAllocateSessionNack());
				goto allocate_error;
			}
		}

		
		/* add channel to session (non-blocking asynchronous processing) */
		apt_bool_t res = mrcp_application_channel_add(session,channel);
		if (!res)
		{
			LogWarn("error:mrcp_application_channel_add");
			SendResponse(req,new MsgMrcpAllocateSessionNack());
			goto allocate_error;
		}

		_mrcpCtxMap[ctx->mrcp_handle] = ctx;
		return;

allocate_error:
		if (channel)
			mrcp_application_session_destroy(session);

	}

	void
	ProcUniMrcp::onMrcpMessageReceived(MrcpOverlapped *olap)
	{
		FUNCTRACKER;

		shared_ptr<MrcpOverlapped> guard(olap);

		MrcpHandle handle = olap->mrcp_handle_id;
		LogDebug("ProcUniMrcp::UponMessageReceived mrcph:" <<  olap->mrcp_handle_id << ", status:" << olap->status << ", request-line:" << olap->message->start_line.method_name.buf);


		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(handle);
		if (iter == _mrcpCtxMap.end())
		{
			//mrcp_application_session_destroy(olap->session);
			LogWarn("ProcUniMrcp::UponMessageReceived mrcph:" << handle << " not found.");
			return;
		}

		MrcpSessionCtxPtr ctx = (*iter).second;

		LogDebug("ProcUniMrcp::Received MRCP message mrcph:" << olap->mrcp_handle_id << 
			" -> net reqid:" << olap->message->start_line.request_id << 
			"(" << olap->message->start_line.method_name.buf << ")" << 
			", curr reqid:" << ctx->last_message->start_line.request_id << 
			"(" << ctx->last_message->start_line.method_name.buf << ")" );

		
		if (ctx->last_message->start_line.request_id != 
			olap->message->start_line.request_id)
		{
			LogWarn("ProcUniMrcp::Received oudated mrcp reponse mrcph:" << olap->mrcp_handle_id );
			return;
		}

		string method_name (olap->message->start_line.method_name.buf);

		if (method_name == "SPEAK-COMPLETE")
		{
			MsgMrcpSpeakStoppedEvt *stopped_msg = new MsgMrcpSpeakStoppedEvt();
			stopped_msg->correlation_id = olap->message->start_line.request_id;
			SendMessage(ctx->session_handler.inbound, IwMessagePtr(stopped_msg));
		} 
		else if (method_name == "SPEAK")
		{
			MsgMrcpSpeakAck *speak_ack= new MsgMrcpSpeakAck();
			speak_ack->correlation_id = olap->message->start_line.request_id;
			SendResponse(ctx->last_user_request, speak_ack);
		}
		else if (method_name == "STOP")
		{
			MsgMrcpStopSpeakAck *stopped_msg = new MsgMrcpStopSpeakAck();
			SendResponse(ctx->last_user_request, stopped_msg);
		}
		// RECOGNIZE-IN-PROGRESS
		else if (method_name == "RECOGNIZE")
		{
			IwMessage *response = NULL;
			if (olap->message->start_line.status_code <= 401) 
			{
				MsgMrcpRecognizeAck *ack = new MsgMrcpRecognizeAck(); 
				ack->response_error_code = olap->message->start_line.status_code;
				response = ack;
			}
			else
			{
				MsgMrcpRecognizeNack *nack = new MsgMrcpRecognizeNack(); 
				nack->response_error_code = olap->message->start_line.status_code;
				response = nack;
			};

			
			SendResponse(ctx->last_user_request, response);
		}
		else if (method_name == "RECOGNITION-COMPLETE")
		{
			MsgMrcpRecognitionCompleteEvt* stopped_msg = new MsgMrcpRecognitionCompleteEvt();
			stopped_msg->correlation_id = olap->message->start_line.request_id;
			stopped_msg->body = (olap->message->body.buf != NULL && olap->message->body.length > 0) ? 
				string(olap->message->body.buf, olap->message->body.length):"";

			SendMessage(ctx->session_handler.inbound, IwMessagePtr(stopped_msg));
		}
		else
		{
			LogWarn("Received unknown response:" << method_name <<", mrcph:" <<olap->mrcp_handle_id )
		}

		
	}

	
	void
	ProcUniMrcp::onMrcpSessionTerminatedEvt(MrcpOverlapped *olap)
	{
		FUNCTRACKER;


		shared_ptr<MrcpOverlapped> ptr (olap);


		MrcpHandle handle = olap->mrcp_handle_id;
		mrcp_sig_status_code_e status = olap->status;

		LogDebug("ProcUniMrcp::onMrcpTerminatedEvt mrcph:" <<  olap->mrcp_handle_id << ", status:" << olap->status);



		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(handle);
		if (iter == _mrcpCtxMap.end())
		{
			LogWarn("session terminated event on non existent ctx.");
		} 
		else 
		{
			FinalizeSessionContext((*iter).second);
		}

	}


	void
	ProcUniMrcp::onMrcpSessionTerminatedRsp(MrcpOverlapped *olap)
	{
		FUNCTRACKER;


		shared_ptr<MrcpOverlapped> ptr (olap);


		MrcpHandle handle = olap->mrcp_handle_id;
		mrcp_sig_status_code_e status = olap->status;

		LogDebug("ProcUniMrcp::onMrcpSessionTerminatedEvt mrcph:" <<  olap->mrcp_handle_id << ", status:" << olap->status);


		// if we got here ctx does not exist already
		mrcp_application_session_destroy(olap->session);
		
		
	}

	void
	ProcUniMrcp::onMrcpChanndelAddEvt(MrcpOverlapped *olap)
	{
		FUNCTRACKER;

		shared_ptr<MrcpOverlapped> ptr (olap);

	
		MrcpHandle handle = olap->mrcp_handle_id;
		mrcp_sig_status_code_e status = olap->status;

		LogDebug("ProcUniMrcp::onMrcpChanndelAddEvtd handle:" <<  olap->mrcp_handle_id << ", status:" << olap->status);

		

		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(handle);
		if (iter == _mrcpCtxMap.end())
		{
			mrcp_application_session_destroy(olap->session);
			LogWarn("Channel status event on non existent ctx.");
			return;
		}

		MrcpSessionCtxPtr ctx = (*iter).second;
		
		switch (status)
		{
		case MRCP_SIG_STATUS_CODE_SUCCESS:
			{
				ctx->state = MRCP_ALLOCATED;

				
				MsgMrcpAllocateSessionAck * rsp = new MsgMrcpAllocateSessionAck();

				

				mrcp_channel_t *channel = NULL;
				switch (olap->channel->resource->id)
				{
				case MRCP_RECOGNIZER_RESOURCE:
					{
						
						channel = ctx->recognizer_channel;
						
						
						break;
					}
				case MRCP_SYNTHESIZER_RESOURCE:
					{
						channel = ctx->synthesizer_channel;
						break;
					}
				default:{}
				};

				mpf_rtp_media_descriptor_t *remote_desc =  
					channel->rtp_termination_slot->descriptor->audio.remote;

				CnxInfo info(
					remote_desc->ip.buf,
					remote_desc->port);

				 DWORD time = ::GetTickCount(); 

				mpf_codec_descriptor_t *dtmf_descriptor = NULL;
				string dtmf_str = "telephone-event";

				BOOL found = FALSE;
				for(int i=0; i<remote_desc->codec_list.descriptor_arr->nelts; i++) {
					mpf_codec_descriptor_t *descriptor1 = &APR_ARRAY_IDX(remote_desc->codec_list.descriptor_arr,i,mpf_codec_descriptor_t);
					if(descriptor1->enabled == FALSE) {
						/* this descriptor has been already disabled, process only enabled ones */
						continue;
					}

					if (!dtmf_descriptor)
					{
						apt_str_t dtmf_name;
						dtmf_name.buf = (char*)dtmf_str.c_str();
						dtmf_name.length = dtmf_str.length();

						// found DTMF string ?
						if (apt_string_compare(&descriptor1->name,&dtmf_name))
						{
							dtmf_descriptor = descriptor1;
						}
					}

					if (!found)
					{
						apt_str_t name;
						name.buf = (char*)ctx->media_format.sdp_name_tos().c_str();
						name.length = ctx->media_format.sdp_name_tos().length();

						found = apt_string_compare(&descriptor1->name,&name);
					}
					
					if (found && dtmf_descriptor)
						break;
				}

				if (!found)
				{
					LogWarn("ProcUniMrcp::onMrcpChanndelAddEvt - suggested codec was not found in remote answer:" <<ctx->media_format.sdp_name_tos());
					MsgMrcpAllocateSessionNack * rsp = new MsgMrcpAllocateSessionNack();
					SendResponse(ctx->last_user_request,rsp);
					break;
				}

				// we assume that server agrred for the sinlge codec suggested
				stringstream sdps;
				sdps << "v=0\r\n"
					<< "o=mrcp " << time << " " << time <<" IN IP4 " << info.iptoa() << "\r\n"
					<< "s=mrcp\r\n"	
					<< "c=IN IP4 "	<< info.iptoa() << "\r\n" 
					<< "t=0 0\r\n"	
					<< "m=audio "  << info.port_ho() << " RTP/AVP " << ctx->media_format.sdp_mapping();
					if (dtmf_descriptor)
						sdps << " " << (int)dtmf_descriptor->payload_type;
				sdps <<  "\r\n";
				sdps << ctx->media_format.get_sdp_a() << "\r\n";
				if (dtmf_descriptor)
				{
					sdps << "a=rtpmap:" << (int)dtmf_descriptor->payload_type<< " "<< dtmf_str << "/" << dtmf_descriptor->sampling_rate << "\r\n";
					sdps << "a=fmtp:"   << (int)dtmf_descriptor->payload_type<< " 0-15" << "\r\n";
				}
					
		
				rsp->mrcp_handle = handle;
				rsp->offer.body = sdps.str();
				
				SendResponse(ctx->last_user_request,rsp);
				break;
			}
		case MRCP_SIG_STATUS_CODE_FAILURE:
		case MRCP_SIG_STATUS_CODE_CANCEL:
			{
				MsgMrcpAllocateSessionNack * rsp = new MsgMrcpAllocateSessionNack();
				SendResponse(ctx->last_user_request,rsp);
				break;
			}
		case MRCP_SIG_STATUS_CODE_TERMINATE:
			{
		
				FinalizeSessionContext(ctx);
				mrcp_application_session_destroy(ctx->session);
				break;
				
		
			}
		default:
			{
				LogWarn("Channel status event on non existent ctx.");
			}
		}

	}

	void
	ProcUniMrcp::FinalizeSessionContext(MrcpSessionCtxPtr ctx)
	{
		
		FUNCTRACKER;
		if (ctx->session_handler.inbound)
		{
			ctx->session_handler.inbound->Send(new MsgMrcpTearDownEvt());
		}
		
		_mrcpCtxMap.erase(ctx->mrcp_handle);
		
	}

	ApiErrorCode
	ProcUniMrcp::Init()
	{
		
		/* APR global initialization */
		if(apr_initialize() != APR_SUCCESS) {
			LogWarn("error:apr_initialize");
			goto error;
		}

		/* create APR pool */
		_pool = apt_pool_create();
		if(!_pool) {
			LogWarn("error:apt_pool_create");
			goto error;
		}

		/* create default directory layout relative to root directory path */
		apt_dir_layout_t *dir_layout = 
			apt_default_dir_layout_create(_conf->GetString("unimrcp/unimrcp_conf_dir").c_str(),_pool);
		if(!dir_layout) {
			LogWarn("error:apt_default_dir_layout_create");
			goto error;
		}

		/* create singleton logger */
		apt_bool_t res = apt_log_instance_create(APT_LOG_OUTPUT_FILE,APT_PRIO_DEBUG,_pool);
		if(!res) {
			LogWarn("error:apt_log_instance_create, res:" << res);
			goto error;
		}

		res = apt_log_file_open(
			dir_layout->log_dir_path,
			_conf->GetString("unimrcp/unimrcp_log_file").c_str(),
			MAX_LOG_FILE_SIZE,
			MAX_LOG_FILE_COUNT,
			_pool);
		if(!res) {
			LogWarn("error:apt_log_file_open, res:" << res);
			goto error;
		}

		_logInititiated = TRUE;

		/* create MRCP client stack */
		_mrcpClient = 
			unimrcp_client_create(dir_layout);
		if(!_mrcpClient) {
			LogWarn("error:unimrcp_client_create");
			goto error;
		}

		_application = 
			mrcp_application_create(app_message_handler,NULL,_pool);
		if(!_application) {
			LogWarn("error:mrcp_application_create");
			goto error;
		}

		res = mrcp_client_application_register(_mrcpClient,_application,"ivrworx_mrcp_app");
		if(!res) {
			LogWarn("error:mrcp_client_application_register");
			goto error;
		}

		
		/* synchronous start of client stack processing */
		res = mrcp_client_start(_mrcpClient);
		if(!res) {
			LogWarn("error:mrcp_client_start");
			goto error;
		}


		return API_SUCCESS;

error:
		Destroy();
		return API_FAILURE;
	}




	
	void
	ProcUniMrcp::Destroy(void)
	{
		
		_mrcpCtxMap.clear();

		if (_mrcpClient) 
		{
			mrcp_client_shutdown(_mrcpClient);
			_mrcpClient = NULL;
		}
		
		/* destroy singleton logger */
		if (_logInititiated == TRUE) 
		{
			apt_log_instance_destroy();
			_logInititiated = FALSE;
		}

		/* destroy APR pool */
		if (_pool != NULL ) 
		{
			apr_pool_destroy(_pool);
			_pool = NULL;
		}
		/* APR global termination */
		apr_terminate();
	}

	
	ProcUniMrcp::~ProcUniMrcp(void)
	{
		FUNCTRACKER;
	}

}

