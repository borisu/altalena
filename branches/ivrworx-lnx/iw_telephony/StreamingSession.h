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

#include "Telephony.h"

using namespace boost;


namespace ivrworx
{

	enum ImsEvents
	{
		MSG_STREAM_ALLOCATE_SESSION_REQ = STREAM_MSG_BASE,
		MSG_STREAM_ALLOCATE_SESSION_ACK,
		MSG_STREAM_ALLOCATE_SESSION_NACK,
		MSG_STREAM_PLAY_REQUEST,
		MSG_STREAM_PLAY_ACK,
		MSG_STREAM_PLAY_NACK,
		MSG_STREAM_STOP_PLAY_REQ,
		MSG_STREAM_STOP_PLAY_ACK,
		MSG_STREAM_STOP_PLAY_NACK,
		MSG_STREAM_PLAY_STOPPED_EVT,
		MSG_STREAM_TEARDOWN_REQ,
		MSG_STREAM_MODIFY_REQ,
		MSG_STREAM_MODIFY_ACK,
		MSG_STREAM_MODIFY_NACK

	};

	typedef int 
	StreamerHandle;

	enum SndDeviceType
	{
		SND_DEVICE_TYPE_FILE,
		SND_DEVICE_TYPE_SND_CARD_MIC,
		SND_DEVICE_TYPE_SND_CARD_LINE
	};

	enum RcvDeviceType
	{
		RCV_DEVICE_NONE,
		RCV_DEVICE_FILE_REC_ID,
		RCV_DEVICE_WINSND_WRITE
	};

	class IW_TELEPHONY_API StreamMixin
	{
	public:

		StreamMixin():
		  streamer_handle(IW_UNDEFINED),
			  correlation_id(IW_UNDEFINED){}

		  AbstractOffer offer;

		  LpHandlePair session_handler;

		  StreamerHandle streamer_handle;

		  int correlation_id;

		  SndDeviceType snd_device_type;

		  RcvDeviceType rcv_device_type;

	};


	class IW_TELEPHONY_API MsgStreamAllocateSessionReq:
		public MsgRequest,
		public StreamMixin
	{
	public:
		MsgStreamAllocateSessionReq():
		  MsgRequest(MSG_STREAM_ALLOCATE_SESSION_REQ, 
			  NAME(MSG_STREAM_ALLOCATE_SESSION_REQ)){};


	};



	class IW_TELEPHONY_API MsgStreamAllocateSessionAck:
		public IwMessage,
		public StreamMixin
	{
	public:
		MsgStreamAllocateSessionAck():
		  IwMessage(MSG_STREAM_ALLOCATE_SESSION_ACK, 
			  NAME(MSG_STREAM_ALLOCATE_SESSION_ACK)){};

	};

	class IW_TELEPHONY_API MsgStreamAllocateSessionNack:
		public IwMessage,
		public StreamMixin
	{
	public:
		MsgStreamAllocateSessionNack():
		  IwMessage(MSG_STREAM_ALLOCATE_SESSION_NACK, 
			  NAME(MSG_STREAM_ALLOCATE_SESSION_NACK)){};

	};

	class IW_TELEPHONY_API MsgStreamModifyReq:
		public MsgRequest,
		public StreamMixin
	{
	public:
		MsgStreamModifyReq():
		  MsgRequest(MSG_STREAM_MODIFY_REQ, 
			  NAME(MSG_STREAM_MODIFY_REQ)){};


	};


	class IW_TELEPHONY_API MsgStreamModifyAck:
		public MsgRequest,
		public StreamMixin
	{
	public:
		MsgStreamModifyAck():
		  MsgRequest(MSG_STREAM_MODIFY_ACK, 
			  NAME(MSG_STREAM_MODIFY_ACK)){};

	};


	class IW_TELEPHONY_API MsgStreamModifyNack:
		public MsgRequest,
		public StreamMixin
	{
	public:
		MsgStreamModifyNack():
		  MsgRequest(MSG_STREAM_MODIFY_NACK, 
			  NAME(MSG_STREAM_MODIFY_NACK)){};

	};

	class IW_TELEPHONY_API MsgStreamPlayReq:
		public MsgRequest,
		public StreamMixin
	{
	public:
		MsgStreamPlayReq():
		  MsgRequest(MSG_STREAM_PLAY_REQUEST, NAME(MSG_STREAM_PLAY_REQUEST)),
			  loop(false),
			  snd_device_type(SND_DEVICE_TYPE_FILE),
			  rcv_device_type(RCV_DEVICE_FILE_REC_ID){};

		  string file_name;

		  BOOL loop;

		  SndDeviceType snd_device_type;

		  RcvDeviceType rcv_device_type;

	};

	class IW_TELEPHONY_API MsgStreamPlayAck:
		public IwMessage,
		public StreamMixin
	{
	public:
		MsgStreamPlayAck():
		  IwMessage(MSG_STREAM_PLAY_ACK, NAME(MSG_STREAM_PLAY_ACK)){};

	};

	class IW_TELEPHONY_API MsgStreamPlayNack:
		public IwMessage,
		public StreamMixin
	{
	public:
		MsgStreamPlayNack():
		  IwMessage(MSG_STREAM_PLAY_NACK, NAME(MSG_STREAM_PLAY_NACK)){};

	};

	class IW_TELEPHONY_API MsgStreamPlayStopped:
		public IwMessage,
		public StreamMixin
	{
	public:
		MsgStreamPlayStopped():
		  IwMessage(MSG_STREAM_PLAY_STOPPED_EVT, NAME(MSG_STREAM_PLAY_STOPPED_EVT)){};

	};

	class IW_TELEPHONY_API MsgStreamTearDownReq:
		public MsgRequest,
		public StreamMixin
	{
	public:
		MsgStreamTearDownReq():
		  MsgRequest(MSG_STREAM_TEARDOWN_REQ, 
			  NAME(MSG_STREAM_TEARDOWN_REQ)){};

	};

	class IW_TELEPHONY_API MsgStreamStopPlayReq:
		public MsgRequest,
		public StreamMixin
	{

	public:
		MsgStreamStopPlayReq():
		  MsgRequest(MSG_STREAM_STOP_PLAY_REQ, 
			  NAME(MSG_STREAM_STOP_PLAY_REQ)){};

	};


	class IW_TELEPHONY_API MsgStreamStopPlayAck:
		public MsgRequest,
		public StreamMixin
	{
	public:
		MsgStreamStopPlayAck():
		  MsgRequest(MSG_STREAM_STOP_PLAY_ACK, 
			  NAME(MSG_STREAM_STOP_PLAY_ACK)){};

	};


	class IW_TELEPHONY_API MsgStreamStopPlayNack:
		public MsgRequest,
		public StreamMixin
	{
	public:
		MsgStreamStopPlayNack():
		  MsgRequest(MSG_STREAM_STOP_PLAY_NACK, 
			  NAME(MSG_STREAM_STOP_PLAY_NACK)){};

	};


	typedef map<int,char> 
	DtmfMap;

	class IW_TELEPHONY_API StreamingSession :
		public ActiveObject
	{
	public:

		// used for optimizations
		enum StreamingSessionState
		{
			STREAMER_SESSION_STATE_UNKNOWN,
			STREAMER_SESSION_STATE_NOT_PLAYING,
			STREAMER_SESSION_STATE_PLAYING
		};

		StreamingSession (IN ScopedForking &forking, IN HandleId streamerHandle);

		virtual ~StreamingSession (void);

		virtual ApiErrorCode	Allocate(
			IN const AbstractOffer &remoteOffer,
			IN RcvDeviceType rcvDeviceType,
			IN SndDeviceType sndDeviceType);

		virtual ApiErrorCode	ModifyConnection(
			IN const AbstractOffer &remoteOffer);

		virtual ApiErrorCode	StopPlay();

		virtual ApiErrorCode PlayFile(
			IN const string &fileName,
			IN BOOL sync = FALSE,
			IN BOOL loop = FALSE);

		virtual void TearDown();

		virtual void UponActiveObjectEvent(IwMessagePtr ptr);

		virtual void InterruptWithHangup();

		virtual StreamerHandle SessionHandle();

		virtual AbstractOffer LocalOffer();

		virtual AbstractOffer RemoteOffer();

	protected:

		HandleId _streamerHandleId;

	private:

		StreamerHandle _streamingSessionHandle;

		ScopedForking &_forking;

		LpHandlePtr _hangupHandle;

		LpHandlePtr _playStoppedHandle;

		StreamingSessionState _state;

		AbstractOffer _localOffer;

		AbstractOffer _remoteOffer;

		
	};


	typedef shared_ptr<StreamingSession> StreamingSessionPtr;


}

