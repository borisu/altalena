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

#pragma once
using namespace boost;

#define  MRCP_Q 5

namespace ivrworx
{
#pragma region Mrcp_Events

	enum MrcpEvents
	{
		MSG_MRCP_ALLOCATE_SESSION_REQ = MSG_USER_DEFINED,
		MSG_MRCP_ALLOCATE_SESSION_ACK,
		MSG_MRCP_ALLOCATE_SESSION_NACK,
		MSG_MRCP_SPEAK_REQ,
		MSG_MRCP_SPEAK_ACK,
		MSG_MRCP_SPEAK_NACK,
		MSG_MRCP_STOP_SPEAK_REQ,
		MSG_MRCP_STOP_SPEAK_ACK,
		MSG_MRCP_STOP_SPEAK_NACK,
		MSG_MRCP_SPEAK_STOPPED_EVT,
		MSG_MRCP_TEARDOWN_REQ,
		MSG_MRCP_TEARDOWN_EVT,
		MSG_MRCP_MODIFY_REQ,
		MSG_MRCP_MODIFY_ACK,
		MSG_MRCP_MODIFY_NACK

	};

	typedef int 
	MrcpHandle;

	class MrcpMixin 
	{
	public :
		MrcpMixin():mrcp_handle(IW_UNDEFINED),correlation_id(IW_UNDEFINED){};
		MrcpHandle mrcp_handle;
		int correlation_id;
	};		


	class MsgMrcpAllocateSessionReq:
		public MsgRequest
	{
	public:
		MsgMrcpAllocateSessionReq():
		  MsgRequest(MSG_MRCP_ALLOCATE_SESSION_REQ, 
			  NAME(MSG_MRCP_ALLOCATE_SESSION_REQ)){};

		  CnxInfo remote_media_data;

		  MediaFormat codec;

		  LpHandlePair session_handler;

	};



	class MsgMrcpAllocateSessionAck:
		public IwMessage, public MrcpMixin
	{
	public:
		MsgMrcpAllocateSessionAck():
		  IwMessage(MSG_MRCP_ALLOCATE_SESSION_ACK, 
			  NAME(MSG_MRCP_ALLOCATE_SESSION_ACK))
			  {};

		  CnxInfo mrcp_media_data;

	};

	class MsgMrcpAllocateSessionNack:
		public IwMessage
	{
	public:
		MsgMrcpAllocateSessionNack():
		  IwMessage(MSG_MRCP_ALLOCATE_SESSION_NACK, 
			  NAME(MSG_MRCP_ALLOCATE_SESSION_NACK)){};

	};

	class MsgMrcpModifyReq:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpModifyReq():
		  MsgRequest(MSG_MRCP_MODIFY_REQ, 
			  NAME(MSG_MRCP_MODIFY_REQ)){};

		  CnxInfo remote_media_data;

		  MediaFormat codec;

		  int playback_handle;


	};


	class MsgMrcpModifyAck:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpModifyAck():
		  MsgRequest(MSG_MRCP_MODIFY_ACK, 
			  NAME(MSG_MRCP_MODIFY_ACK)){};

	};


	class MsgMrcpModifyNack:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpModifyNack():
		  MsgRequest(MSG_MRCP_MODIFY_NACK, 
			  NAME(MSG_MRCP_MODIFY_NACK)){};

	};

	class MsgMrcpSpeakReq:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpSpeakReq():
		  MsgRequest(MSG_MRCP_SPEAK_REQ, NAME(MSG_MRCP_SPEAK_REQ)){};

		  string mrcp_xml;

		

	};

	class MsgMrcpSpeakAck:
		public IwMessage, 
		public MrcpMixin
	{
	public:
		MsgMrcpSpeakAck():
		  IwMessage(MSG_MRCP_SPEAK_ACK, NAME(MSG_MRCP_SPEAK_ACK)){};

	};

	class MsgMrcpSpeakReqNack:
		public IwMessage, 
		public MrcpMixin
	{
	public:
		MsgMrcpSpeakReqNack():
		  IwMessage(MSG_MRCP_SPEAK_NACK, NAME(MSG_MRCP_SPEAK_NACK)) {};

	};

	class MsgMrcpSpeakStoppedEvt:
		public IwMessage, 
		public MrcpMixin
	{
	public:
		MsgMrcpSpeakStoppedEvt():
		  IwMessage(MSG_MRCP_SPEAK_STOPPED_EVT, NAME(MSG_MRCP_SPEAK_STOPPED_EVT)),
			  error(API_FAILURE){};

		  ApiErrorCode error;
	};

	class MsgMrcpTearDownReq:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpTearDownReq():
		  MsgRequest(MSG_MRCP_TEARDOWN_REQ, 
			  NAME(MSG_MRCP_TEARDOWN_REQ)){};

		 

	};

	class MsgMrcpTearDownEvt:
		public IwMessage, 
		public MrcpMixin
	{
	public:
		MsgMrcpTearDownEvt():
		  IwMessage(MSG_MRCP_TEARDOWN_EVT, 
			  NAME(MSG_MRCP_TEARDOWN_EVT)){};

		  

	};

	class MsgMrcpStopSpeakReq:
		public MsgRequest, 
		public MrcpMixin
	{

	public:
		MsgMrcpStopSpeakReq():
		  MsgRequest(MSG_MRCP_STOP_SPEAK_REQ, 
			  NAME(MSG_MRCP_STOP_SPEAK_REQ)){};

		 

	};


	class MsgMrcpStopSpeakAck:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpStopSpeakAck():
		  MsgRequest(MSG_MRCP_STOP_SPEAK_ACK, 
			  NAME(MSG_MRCP_STOP_SPEAK_ACK)){};

	};


	class MsgMrcpStopSpeakNack:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpStopSpeakNack():
		  MsgRequest(MSG_MRCP_STOP_SPEAK_NACK, 
			  NAME(MSG_MRCP_STOP_SPEAK_NACK)){};

	};


#pragma endregion Mrcp_Events

	typedef map<int,char> 
	DtmfMap;

	class MrcpSession :
		public ActiveObject
	{
	public:

		MrcpSession (IN ScopedForking &forking);

		virtual ~MrcpSession (void);

		virtual ApiErrorCode	Allocate();

		virtual ApiErrorCode	Allocate(
			IN CnxInfo remote_end, 
			IN MediaFormat codec);

		virtual ApiErrorCode	ModifySession(
			IN CnxInfo remote_end, 
			IN MediaFormat codec);

		virtual ApiErrorCode	StopSpeak();

		virtual ApiErrorCode Speak(
			IN const string &file_name,
			IN BOOL sync = TRUE);

		virtual void TearDown();

		virtual CnxInfo MrcpMediaData() const;

		virtual void MrcpMediaData(IN CnxInfo val);

		virtual void UponActiveObjectEvent(IwMessagePtr ptr);

		virtual void InterruptWithHangup();

		virtual MrcpHandle SessionHandle();

	private:

		ApiErrorCode	SyncStreamFile();

		MrcpHandle _mrcpSessionHandle;

		CnxInfo _mrcpMediaData;

		ScopedForking &_forking;

		LpHandlePtr _hangupHandle;

		LpHandlePtr _playStoppedHandle;

		LpHandlePair _mrcpSessionHandlerPair;

	};


	typedef shared_ptr<MrcpSession> MrcpSessionPtr;


}

