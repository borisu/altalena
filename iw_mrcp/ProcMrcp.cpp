#include "StdAfx.h"
#include "ProcMrcp.h"
#include "Logger.h"


#define IW_MRCP_CLIENT_TIMEOUT 5000
#define	IW_DEFAULT_MRCP_TIMEOUT		60000 // 1 min


#define IW_MRCP_CHANNEL_STATUS_EVT 5000
#define IW_MRCP_MESSAGE_RECEIVE_EVT 5001

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
		channel(NULL)

	{

	}

	MrcpSessionCtx::~MrcpSessionCtx()
	{
		

	}

	/** Event indicating client stack is started and ready to process requests from the application */
	static HANDLE dupReadyEvent = NULL;
	apt_bool_t iw_application_on_message_ready(
		mrcp_application_t *application, 
		mrcp_sig_status_code_e status)
	{
		if (dupReadyEvent)
		{
			::SetEvent(dupReadyEvent);
			::CloseHandle(dupReadyEvent);
		}

		return TRUE;
	}

	apt_bool_t iw_application_on_channel_add(
		mrcp_application_t *application, 
		mrcp_session_t *session, 
		mrcp_channel_t *channel, 
		mrcp_sig_status_code_e status)
	{

		MrcpOverlapped *olap = new MrcpOverlapped();
		SecureZeroMemory(olap, sizeof(MrcpOverlapped));
		olap->mrcp_handle_id = (int)((mrcp_client_session_t*)session)->app_obj;
		olap->status = status;
		olap->channel = channel;
		olap->session = session;

		BOOL res = ::PostQueuedCompletionStatus(
					g_iocpHandle,				//A handle to an I/O completion port to which the I/O completion packet is to be posted.
					0,							//The value to be returned through the lpNumberOfBytesTransferred parameter of the GetQueuedCompletionStatus function.
					IW_MRCP_CHANNEL_STATUS_EVT,	//The value to be returned through the lpCompletionKey parameter of the GetQueuedCompletionStatus function.
					olap						//The value to be returned through the lpOverlapped parameter of the GetQueuedCompletionStatus function.
					);

		if (res == FALSE)
		{
			LogSysError("::PostQueuedCompletionStatus");
			throw std::exception("System error in iw_application_on_channel_add");
		}

		
		return TRUE;
	}

	apt_bool_t iw_application_on_message_receive(
		mrcp_application_t *application, 
		mrcp_session_t *session, 
		mrcp_channel_t *channel, 
		mrcp_message_t *message)
	{

		MrcpOverlapped *olap = new MrcpOverlapped();
		SecureZeroMemory(olap, sizeof(MrcpOverlapped));
		olap->mrcp_handle_id = (int)((mrcp_client_session_t*)session)->app_obj;
		olap->message = message;
		olap->channel = channel;
		olap->session = session;

		BOOL res = ::PostQueuedCompletionStatus(
			g_iocpHandle,				 //A handle to an I/O completion port to which the I/O completion packet is to be posted.
			0,							 //The value to be returned through the lpNumberOfBytesTransferred parameter of the GetQueuedCompletionStatus function.
			IW_MRCP_MESSAGE_RECEIVE_EVT, //The value to be returned through the lpCompletionKey parameter of the GetQueuedCompletionStatus function.
			olap						 //The value to be returned through the lpOverlapped parameter of the GetQueuedCompletionStatus function.
			);

		if (res == FALSE)
		{
			LogSysError("::PostQueuedCompletionStatus");
			throw std::exception("System error in iw_application_on_channel_add");
		}


		return TRUE;
	}

	static const mrcp_app_message_dispatcher_t iw_application_dispatcher = {
		NULL, //iw_application_on_session_update,
		NULL, //iw_application_on_session_terminate,
		iw_application_on_channel_add, 
		NULL, //iw_application_on_channel_remove,
		iw_application_on_message_receive, //iw_application_on_message_receive
		iw_application_on_message_ready,
		NULL,
		NULL
	};

	apt_bool_t app_message_handler(const mrcp_app_message_t *app_message)
	{
		/* app_message should be dispatched now,
		*  the default dispatcher is used in demo. */
		mrcp_application_message_dispatch(&iw_application_dispatcher,app_message);

		return TRUE;
	}

	
	ProcMrcp::ProcMrcp(LpHandlePair pair, Configuration &conf):
	LightweightProcess(pair,MRCP_Q,"ProcMrcpClient"),
	_conf(conf),
	_application(NULL),
	_pool(NULL),
	_logInititiated(FALSE)
	{
		FUNCTRACKER;

		_iocpPtr = IocpInterruptorPtr(new IocpInterruptor());
		_inbound->HandleInterruptor(_iocpPtr);

		g_iocpHandle = _iocpPtr->WinHandle();
	}

	void 
	ProcMrcp::real_run()
	{

		if (IW_FAILURE(Init()))
		{
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
					throw;
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
			case IW_MRCP_CHANNEL_STATUS_EVT:
				{
					UponChnannelConnected((MrcpOverlapped*)lpOverlapped);
					continue;
				}
			case IW_MRCP_MESSAGE_RECEIVE_EVT:
				{
					UponMessageReceived((MrcpOverlapped*)lpOverlapped);
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
			case MSG_MRCP_ALLOCATE_SESSION_REQ:
				{
					UponAllocateSessionReq(ptr);
					break;
				}
			case MSG_MRCP_SPEAK_REQ:
				{
					UponSpeakReq(ptr);
					break;
				}
			case MSG_MRCP_TEARDOWN_REQ:
				{
					UponTearDownReq(ptr);
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
		}// while

	}

	/** Create demo RTP termination descriptor */
	mpf_rtp_termination_descriptor_t* rtp_descriptor_create(apr_pool_t *pool, const CnxInfo &info,const MediaFormat &media_format)
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
		apt_string_assign(&media->base.ip,info.iptoa(),pool);
		media->base.port = info.port_ho();
		media->base.state = MPF_MEDIA_ENABLED;
		media->mode = STREAM_MODE_RECEIVE;

		
		/* initialize codec list */
		mpf_codec_list_init(&media->codec_list,1,pool);

		

		if (media_format.get_media_type() == MediaFormat::MediaType_SPEECH)
		{
			/* set another codec descriptor */
			codec_descriptor = mpf_codec_list_add(&media->codec_list);
			if(codec_descriptor) {
				codec_descriptor->payload_type = media_format.sdp_mapping();
				apt_string_set(&codec_descriptor->name,media_format.sdp_name_tos().c_str());
				codec_descriptor->sampling_rate = media_format.sampling_rate();
				codec_descriptor->channel_count = 1;
			}
		}
		

		rtp_descriptor->audio.local = media;
		return rtp_descriptor;
	}

	void
	ProcMrcp::UponTearDownReq(IwMessagePtr msg)
	{
		FUNCTRACKER;


		shared_ptr<MsgMrcpTearDownReq> req  =
			dynamic_pointer_cast<MsgMrcpTearDownReq> (msg);

		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(req->mrcp_handle);
		if (iter == _mrcpCtxMap.end())
		{

			LogWarn("ProcMrcp::UponTearDownReq - non existent ctx.");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		MrcpSessionCtxPtr ctx = (*iter).second;

		if (ctx->state != MRCP_ALLOCATED)
		{
			
			return;
		}

		mrcp_application_session_terminate(ctx->session);
		ctx->session = NULL;


	}

	void
	ProcMrcp::UponSpeakReq(IwMessagePtr msg)
	{
		FUNCTRACKER;
		

		shared_ptr<MsgMrcpSpeakReq> req  =
			dynamic_pointer_cast<MsgMrcpSpeakReq> (msg);

		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(req->mrcp_handle);
		if (iter == _mrcpCtxMap.end())
		{
			
			LogWarn("ProcMrcp::UponSpeakReq - non existent mrcph:" << req->mrcp_handle);
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		MrcpSessionCtxPtr ctx = (*iter).second;

		if (ctx->state != MRCP_ALLOCATED)
		{
			LogWarn("Channel status event on non existent ctx.");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		


		//
		// create MRCP message 
		//
		mrcp_message_t *mrcp_message = 
			mrcp_application_message_create(
			ctx->session,
			ctx->channel,
			SYNTHESIZER_SPEAK);

		if(!mrcp_message) 
		{
			LogWarn("error(speak):mrcp_application_message_create");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return ;
		}

		//
		// get/allocate generic header 
		//
		mrcp_generic_header_t *generic_header = (mrcp_generic_header_t *)mrcp_generic_header_prepare(mrcp_message);
		if(!generic_header) 
		{
			LogWarn("error(speak):mrcp_generic_header_prepare");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		//
		// set generic header fields 
		//
		apt_string_assign(&generic_header->content_type,"application/synthesis+ssml",mrcp_message->pool);
		mrcp_generic_header_property_add(mrcp_message,GENERIC_HEADER_CONTENT_TYPE);


		//
		// get/allocate synthesizer header 
		//
		mrcp_synth_header_t *synth_header = (mrcp_synth_header_t *)mrcp_resource_header_prepare(mrcp_message);
		if(!synth_header) 
		{
			LogWarn("error(speak):mrcp_resource_header_prepare");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		
		//
		//
		//
		synth_header->voice_param.age = 28;
		mrcp_resource_header_property_add(mrcp_message,SYNTHESIZER_HEADER_VOICE_AGE);

		apt_string_assign_n(
			&mrcp_message->body,
			req->mrcp_xml.c_str(),
			req->mrcp_xml.size(),
			mrcp_message->pool);

	
		/* send MRCP request (non-blocking, asynchronous processing) */
		apt_bool_t res = mrcp_application_message_send(ctx->session,ctx->channel,mrcp_message);
		if (!res)
		{
			LogWarn("error(speak):mrcp_application_message_send");
			SendResponse(req, new MsgMrcpSpeakReqNack());
			return;
		}

		ctx->last_user_request = req;

		if (req->send_provisional)
		{
			SendResponse(req, new MsgMrcpSpeakReqAck());
		}

		

	}

	void
	ProcMrcp::UponAllocateSessionReq(IwMessagePtr msg)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		shared_ptr<MsgMrcpAllocateSessionReq> req  =
			dynamic_pointer_cast<MsgMrcpAllocateSessionReq> (msg);

		//
		// create new context
		//
		MrcpSessionCtxPtr ctx(new MrcpSessionCtx());
		long handle = GetNewMrcpHandle();
		ctx->mrcp_handle = handle;
		ctx->state = MRCP_CONNECTING;
		ctx->last_user_request = req;


		/* create session */
		mrcp_session_t *session = 
			mrcp_application_session_create(_application,"MRCPv2-Default", (void *)handle);

		if (!session)
		{
			LogWarn("error:mrcp_application_session_create");
			SendResponse(req,new MsgMrcpAllocateSessionNack());
		}

		ctx->session = session;

		mpf_rtp_termination_descriptor_t *rtp_descriptor = 
			rtp_descriptor_create(session->pool,req->remote_media_data, req->codec);

		mrcp_channel_t *channel;
		/* create synthesizer channel */
		channel = mrcp_application_channel_create(
			session,                     /* session, channel belongs to */
			MRCP_SYNTHESIZER_RESOURCE,   /* MRCP resource identifier */
			NULL /*termination*/,        /* media termination, used to terminate audio stream */
			rtp_descriptor,              /* RTP descriptor, used to create RTP termination (NULL by default) */
			NULL);                       /* object to associate */

		/* add channel to session (non-blocking asynchronous processing) */
		apt_bool_t res = mrcp_application_channel_add(session,channel);
		if (!res)
		{
			LogWarn("error:mrcp_application_channel_add");
			SendResponse(req,new MsgMrcpAllocateSessionNack());
		}

		ctx->channel = channel;

		_mrcpCtxMap[ctx->mrcp_handle] = ctx;

	}

	void
	ProcMrcp::UponMessageReceived(MrcpOverlapped *olap)
	{
		FUNCTRACKER;

		if (olap == NULL)
		{
			LogWarn("olap NULL!");
			return;
		}

		shared_ptr<MrcpOverlapped> guard(olap);

		MrcpHandle handle = olap->mrcp_handle_id;
		LogDebug("ProcMrcp::UponMessageReceived handle:" <<  olap->mrcp_handle_id << ", status:" << olap->status);

		

		MrcpCtxMap::iterator iter =  _mrcpCtxMap.find(handle);
		if (iter == _mrcpCtxMap.end())
		{
			mrcp_application_session_destroy(olap->session);
			LogWarn("ProcMrcp::UponMessageReceived mrcph:" << handle << " not found.");
			return;
		}

		MrcpSessionCtxPtr ctx = (*iter).second;

		string method_name (olap->message->start_line.method_name.buf);

		if (method_name == "SPEAK-COMPLETE")
		{
			MsgMrcpSpeakStoppedEvt *stopped_msg = new MsgMrcpSpeakStoppedEvt();
			SendResponse(ctx->last_user_request, stopped_msg);
		}

	}

	void
	ProcMrcp::UponChnannelConnected(MrcpOverlapped *olap)
	{
		FUNCTRACKER;

		if (olap == NULL)
		{
			LogWarn("olap NULL!");
			return;
		}

		shared_ptr<MrcpOverlapped> ptr (olap);

	
		MrcpHandle handle = olap->mrcp_handle_id;
		mrcp_sig_status_code_e status = olap->status;

		LogDebug("UponChnannelConnected handle:" <<  olap->mrcp_handle_id << ", status:" << olap->status);

		

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
				rsp->mrcp_handle = handle;
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
		
				_mrcpCtxMap.erase(iter);
				if (ctx->session_handler.inbound)
				{
					MsgMrcpTearDownEvt * rsp = new MsgMrcpTearDownEvt();
					rsp->mrcp_handle = handle;
					SendMessage(ctx->session_handler.inbound,IwMessagePtr(rsp));
				}
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
	ProcMrcp::TearDownSession(MrcpSessionCtxPtr ctx)
	{
		
		FUNCTRACKER;
		_mrcpCtxMap.erase(ctx->mrcp_handle);
		
	}

	ApiErrorCode
	ProcMrcp::Init()
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
			apt_default_dir_layout_create("c:\\mrcp",_pool);
		if(!dir_layout) {
			LogWarn("error:apt_default_dir_layout_create");
			goto error;
		}

		/* create singleton logger */
		apt_bool_t res = apt_log_instance_create(APT_LOG_OUTPUT_FILE,APT_PRIO_DEBUG,_pool);
		if(!res) {
			LogWarn("error:apt_log_instance_create");
			goto error;
		}

		res = apt_log_file_open(
			dir_layout->log_dir_path,
			"unimrcpclient",
			MAX_LOG_FILE_SIZE,
			MAX_LOG_FILE_COUNT,
			_pool);
		if(!res) {
			LogWarn("error:apt_log_file_open");
			goto error;
		}

		_logInititiated = TRUE;

		/* create client stack */
		_client = 
			unimrcp_client_create(dir_layout);
		if(!_client) {
			LogWarn("error:unimrcp_client_create");
			goto error;
		}

		_application = 
			mrcp_application_create(app_message_handler,NULL,_pool);
		if(!_application) {
			LogWarn("error:mrcp_application_create");
			goto error;
		}

		res = mrcp_client_application_register(_client,_application,"ivrworx_mrcp_app");
		if(!res) {
			LogWarn("error:mrcp_client_application_register");
			goto error;
		}

		HANDLE ready_event = ::CreateEvent(NULL,TRUE,FALSE,NULL);
		if (!ready_event)
		{
			LogSysError("::CreateEvent");
			goto error;
		}

		 BOOL osRes = ::DuplicateHandle(
			::GetCurrentProcess(), 
			ready_event, 
			::GetCurrentProcess(),
			&dupReadyEvent, 
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS);
		 if (!osRes)
		 {
			 ::CloseHandle(ready_event);
			 ::CloseHandle(dupReadyEvent);
			 LogSysError("::DuplicateHandle");
			 goto error;
		 }
		
		
		/* asynchronous start of client stack processing */
		res = mrcp_client_start(_client);
		if(!res) {
			LogWarn("error:mrcp_client_start");
			::CloseHandle(ready_event);
			::CloseHandle(dupReadyEvent);
			goto error;
		}


		DWORD waitRes = ::WaitForSingleObject(ready_event,IW_MRCP_CLIENT_TIMEOUT);
		switch (waitRes)
		{
		case WAIT_OBJECT_0:
			{
				::CloseHandle(ready_event);
				break;
			}
		default:
			{
				LogWarn("error:mrcp_client_start");
				::CloseHandle(ready_event);
				::CloseHandle(dupReadyEvent);
				goto error;
			}

		}

		return API_SUCCESS;

error:
		Destroy();
		return API_FAILURE;
	}




	
	void
	ProcMrcp::Destroy(void)
	{
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

	
	ProcMrcp::~ProcMrcp(void)
	{
		Destroy();
	}

}

