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

#include "ImsSession.h"
#include "ImsFactory.h"

namespace ivrworx
{

#pragma region Playback_Events

	enum ImsEvents
	{
		MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST = MSG_USER_DEFINED,
		MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK,
		MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK,
		MSG_START_PLAYBACK_REQUEST,
		MSG_START_PLAY_REQ_ACK,
		MSG_IMS_START_PLAY_REQ_NACK,
		MSG_STOP_PLAYBACK_REQUEST,
		MSG_STOP_PLAYBACK_REQUEST_ACK,
		MSG_STOP_PLAYBACK_REQUEST_NACK,
		MSG_IMS_PLAY_STOPPED,
		MSG_IMS_TEARDOWN_REQ,
		MSG_IMS_RFC2833DTMF_EVT,
		MSG_IMS_SEND_RFC2833DTMF_REQ,
		MSG_IMS_START_RECORD_REQ,
		MSG_IMS_START_RECORD_ACK,
		MSG_IMS_STOP_RECORD_REQ
	};


	class MsgAllocateImsSessionReq:
		public MsgRequest
	{
	public:
		MsgAllocateImsSessionReq():
		  MsgRequest(MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST, 
			  NAME(MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST)){};

		  CnxInfo remote_media_data;

		  MediaFormat codec;

		  LpHandlePair session_handler;

	};

	class MsgAllocateImsSessionAck:
		public IwMessage
	{
	public:
		MsgAllocateImsSessionAck():
		  IwMessage(MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK, 
			  NAME(MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK)),
			  playback_handle(IW_UNDEFINED){};

		  int playback_handle;

		  CnxInfo ims_media_data;

	};

	class MsgAllocateImsSessionNack:
		public IwMessage
	{
	public:
		MsgAllocateImsSessionNack():
		  IwMessage(MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK, 
			  NAME(MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK)){};

	};

	class MsgStartPlayReq:
		public MsgRequest
	{
	public:
		MsgStartPlayReq():
		  MsgRequest(MSG_START_PLAYBACK_REQUEST, NAME(MSG_START_PLAYBACK_REQUEST)),
			  playback_handle(IW_UNDEFINED),loop(false),send_provisional(false) {};

		  int playback_handle;

		  string file_name;

		  BOOL loop;

		  BOOL send_provisional;

	};

	class MsgStartPlayReqAck:
		public IwMessage
	{
	public:
		MsgStartPlayReqAck():
		  IwMessage(MSG_START_PLAY_REQ_ACK, NAME(MSG_START_PLAY_REQ_ACK)){};

	};

	class MsgStartPlayReqNack:
		public IwMessage
	{
	public:
		MsgStartPlayReqNack():
		  IwMessage(MSG_IMS_START_PLAY_REQ_NACK, NAME(MSG_IMS_START_PLAY_REQ_NACK)),
			  playback_handle(IW_UNDEFINED){};

		  int playback_handle;
	};

	class MsgImsPlayStopped:
		public IwMessage
	{
	public:
		MsgImsPlayStopped():
		  IwMessage(MSG_IMS_PLAY_STOPPED, NAME(MSG_IMS_PLAY_STOPPED)),
			  playback_handle(IW_UNDEFINED),
			  error(API_FAILURE){};

		  int playback_handle;

		  ApiErrorCode error;
	};

	class MsgImsTearDownReq:
		public MsgRequest
	{
	public:
		MsgImsTearDownReq():
		  MsgRequest(MSG_IMS_TEARDOWN_REQ, 
			  NAME(MSG_IMS_TEARDOWN_REQ)){};

		  ImsHandleId handle;

	};

	class MsgStopPlaybackReq:
		public MsgRequest
	{

	public:
		MsgStopPlaybackReq():
		  MsgRequest(MSG_STOP_PLAYBACK_REQUEST, 
			  NAME(MSG_STOP_PLAYBACK_REQUEST)){};

		  ImsHandleId handle;

	};


	class MsgStopPlaybackAck:
		public MsgRequest
	{
	public:
		MsgStopPlaybackAck():
		  MsgRequest(MSG_STOP_PLAYBACK_REQUEST_ACK, 
			  NAME(MSG_STOP_PLAYBACK_REQUEST_ACK)){};

	};


	class MsgStopPlaybackNack:
		public MsgRequest
	{
	public:
		MsgStopPlaybackNack():
		  MsgRequest(MSG_STOP_PLAYBACK_REQUEST_NACK, 
			  NAME(MSG_STOP_PLAYBACK_REQUEST_NACK)){};

	};


	class MsgImsRfc2833DtmfEvt:
		public IwMessage
	{
	public:
		MsgImsRfc2833DtmfEvt():IwMessage(MSG_IMS_RFC2833DTMF_EVT, 
			NAME(MSG_IMS_RFC2833DTMF_EVT)){}

		int dtmf_digit;

	};

	class MsgImsSendRfc2833DtmfReq:
		public IwMessage
	{
	public:
		MsgImsSendRfc2833DtmfReq():IwMessage(MSG_IMS_SEND_RFC2833DTMF_REQ, 
			NAME(MSG_IMS_SEND_RFC2833DTMF_REQ)){}

		int dtmf_digit;

		ImsHandleId handle;

	};

}

#pragma endregion Playback_Events




