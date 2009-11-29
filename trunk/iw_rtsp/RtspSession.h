#pragma once

namespace ivrworx
{
	enum RtspEvents
	{
		MSG_RTSP_ALLOCATE_SESSION_REQ = MSG_USER_DEFINED,
		MSG_RTSP_ALLOCATE_SESSION_ACK,
		MSG_RTSP_ALLOCATE_SESSION_NACK
	};

	class MsgRtspAllocateSessionReq
		: public MsgRequest
	{
	public:
		MsgRtspAllocateSessionReq(): 
			MsgRequest(MSG_RTSP_ALLOCATE_SESSION_REQ,NAME(MSG_RTSP_ALLOCATE_SESSION_REQ)){}
	};

	class MsgRtspAllocateSessionAck
		: public MsgRequest
	{
	public:
		MsgRtspAllocateSessionAck(): 
		  MsgRequest(MSG_RTSP_ALLOCATE_SESSION_ACK,NAME(MSG_RTSP_ALLOCATE_SESSION_ACK)){}
	};


	class MsgRtspAllocateSessionNack
		: public MsgRequest
	{
	public:
		MsgRtspAllocateSessionNack(): 
		  MsgRequest(MSG_RTSP_ALLOCATE_SESSION_NACK,NAME(MSG_RTSP_ALLOCATE_SESSION_NACK)){}
	};

	class RtspSession
	{
	public:
		RtspSession(void);
		virtual ~RtspSession(void);
	};

}
