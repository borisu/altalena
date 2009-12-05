#pragma once

#define RTSP_Q 23

namespace ivrworx
{
	enum RtspEvents
	{
		MSG_RTSP_SETUP_SESSION_REQ = MSG_USER_DEFINED,
		MSG_RTSP_SETUP_SESSION_ACK,
		MSG_RTSP_SETUP_SESSION_NACK,
		MSG_RTSP_PLAY_REQ ,
		MSG_RTSP_PLAY_ACK,
		MSG_RTSP_PLAY_NACK,
		MSG_RTSP_PAUSE_REQ,
		MSG_RTSP_PAUSE_ACK,
		MSG_RTSP_PAUSE_NACK,
		MSG_RTSP_TEARDOWN_REQ,
		MSG_RTSP_TEARDOWN_ACK,
		MSG_RTSP_TEARDOWN_NACK
	};

	typedef int
	RtspHandle; 

	class RtsMixin
	{
	public:

		RtsMixin():
		  rtsp_handle(IW_UNDEFINED),
		  start_time(0.0),
		  end_time(0.0),
		  scale(1.0)
		{

		}

		RtspHandle rtsp_handle;

		string request_url;

		MediaFormat media_format;

		double start_time;

		double end_time;

		float scale;

		CnxInfo local_cnx_info;

	};

//--- Setup session

	class MsgRtspSetupSessionReq
		: public RtsMixin,
		public MsgRequest
	{
	public:
		MsgRtspSetupSessionReq(): 
			MsgRequest(MSG_RTSP_SETUP_SESSION_REQ,NAME(MSG_RTSP_SETUP_SESSION_REQ)){}
	};

	class MsgRtspSetupSessionAck
		: public RtsMixin, public MsgRequest
	{
	public:
		MsgRtspSetupSessionAck(): 
		  MsgRequest(MSG_RTSP_SETUP_SESSION_ACK,NAME(MSG_RTSP_SETUP_SESSION_ACK)){}
	};


	class MsgRtspSetupSessionNack
		: public RtsMixin,public MsgRequest
	{
	public:
		MsgRtspSetupSessionNack(): 
		  MsgRequest(MSG_RTSP_SETUP_SESSION_NACK,NAME(MSG_RTSP_SETUP_SESSION_NACK)){}
	};

//--- Play

	class MsgRtspPlayReq
		: public RtsMixin,
		public MsgRequest
	{
	public:
		MsgRtspPlayReq(): 
		  MsgRequest(MSG_RTSP_PLAY_REQ,NAME(MSG_RTSP_PLAY_REQ)){}
	};

	class MsgRtspPlayAck
		: public RtsMixin, public MsgRequest
	{
	public:
		MsgRtspPlayAck(): 
		  MsgRequest(MSG_RTSP_PLAY_ACK,NAME(MSG_RTSP_PLAY_ACK)){}
	};


	class MsgRtspPlayNack
		: public RtsMixin,public MsgRequest
	{
	public:
		MsgRtspPlayNack(): 
		  MsgRequest(MSG_RTSP_PLAY_NACK,NAME(MSG_RTSP_PLAY_NACK)){}
	};

//--- Pause

	class MsgRtspPauseReq
		: public RtsMixin,
		public MsgRequest
	{
	public:
		MsgRtspPauseReq(): 
		  MsgRequest(MSG_RTSP_PAUSE_REQ,NAME(MSG_RTSP_PAUSE_REQ)){}
	};

	class MsgRtspPauseAck
		: public RtsMixin, public MsgRequest
	{
	public:
		MsgRtspPauseAck(): 
		  MsgRequest(MSG_RTSP_PAUSE_ACK,NAME(MSG_RTSP_PAUSE_ACK)){}
	};

	class MsgRtspPauseNack
		: public RtsMixin, public MsgRequest
	{
	public:
		MsgRtspPauseNack(): 
		  MsgRequest(MSG_RTSP_PAUSE_NACK,NAME(MSG_RTSP_PAUSE_NACK)){}
	};

// -- Tear Down

	class MsgRtspTearDownReq
		: public RtsMixin,public MsgRequest
	{
	public:
		MsgRtspTearDownReq(): 
		  MsgRequest(MSG_RTSP_TEARDOWN_REQ,NAME(MSG_RTSP_TEARDOWN_REQ)){}
	};

	class MsgRtspTearDownAck
		: public RtsMixin, public MsgRequest
	{
	public:
		MsgRtspTearDownAck(): 
		  MsgRequest(MSG_RTSP_TEARDOWN_ACK,NAME(MSG_RTSP_TEARDOWN_ACK)){}
	};


	class MsgRtspTearDownNack
		: public RtsMixin,public MsgRequest
	{
	public:
		MsgRtspTearDownNack(): 
		  MsgRequest(MSG_RTSP_TEARDOWN_NACK,NAME(MSG_RTSP_TEARDOWN_NACK)){}
	};


	class RtspSession
	{
	public:

		RtspSession(ScopedForking &forking, Configuration &conf);

		virtual ~RtspSession(void);

		virtual ApiErrorCode Init();

		virtual ApiErrorCode Setup(IN const string &rtsp_url, IN const MediaFormat &format);

		virtual ApiErrorCode Play(double start_time = 0.0, double end_time = 0.0 , double scale = 1.0);

		virtual ApiErrorCode Pause();

		virtual ApiErrorCode TearDown();

		virtual RtspHandle SessionHandle();

	private:

		RtspHandle _rtspHandle;

		LpHandlePair _rtspProcPair;

		ScopedForking &_forking;

		Configuration &_conf;

	};

}
