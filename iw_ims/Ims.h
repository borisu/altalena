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
		CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST = MSG_USER_DEFINED,
		CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK,
		CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK,
		CCU_MSG_START_PLAYBACK_REQUEST,
		CCU_MSG_START_PLAY_REQ_ACK,
		CCU_MSG_IMS_START_PLAY_REQ_NACK,
		CCU_MSG_STOP_PLAYBACK_REQUEST,
		CCU_MSG_STOP_PLAYBACK_REQUEST_ACK,
		CCU_MSG_STOP_PLAYBACK_REQUEST_NACK,
		CCU_MSG_IMS_PLAY_STOPPED,
		CCU_MSG_IMS_TEARDOWN_REQ
	};


	class CcuMsgAllocateImsSessionReq:
		public MsgRequest
	{
	public:
		CcuMsgAllocateImsSessionReq():
		  MsgRequest(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST, 
			  NAME(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST)){};

		  CnxInfo remote_media_data;

		  MediaFormat codec;

		  LpHandlePair session_handler;

	};

	class CcuMsgAllocateImsSessionAck:
		public IwMessage
	{
	public:
		CcuMsgAllocateImsSessionAck():
		  IwMessage(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK, 
			  NAME(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK)),
			  playback_handle(IW_UNDEFINED){};

		  int playback_handle;

		  CnxInfo ims_media_data;

	};

	class CcuMsgAllocateImsSessionNack:
		public IwMessage
	{
	public:
		CcuMsgAllocateImsSessionNack():
		  IwMessage(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK, 
			  NAME(CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK)){};

	};

	class CcuMsgStartPlayReq:
		public MsgRequest
	{
	public:
		CcuMsgStartPlayReq():
		  MsgRequest(CCU_MSG_START_PLAYBACK_REQUEST, NAME(CCU_MSG_START_PLAYBACK_REQUEST)),
			  playback_handle(IW_UNDEFINED),loop(false),send_provisional(false) {};

		  int playback_handle;

		  string file_name;

		  BOOL loop;

		  BOOL send_provisional;

	};

	class CcuMsgStartPlayReqAck:
		public IwMessage
	{
	public:
		CcuMsgStartPlayReqAck():
		  IwMessage(CCU_MSG_START_PLAY_REQ_ACK, NAME(CCU_MSG_START_PLAY_REQ_ACK)){};

	};

	class CcuMsgStartPlayReqNack:
		public IwMessage
	{
	public:
		CcuMsgStartPlayReqNack():
		  IwMessage(CCU_MSG_IMS_START_PLAY_REQ_NACK, NAME(CCU_MSG_IMS_START_PLAY_REQ_NACK)),
			  playback_handle(IW_UNDEFINED){};

		  int playback_handle;
	};

	class CcuMsgImsPlayStopped:
		public IwMessage
	{
	public:
		CcuMsgImsPlayStopped():
		  IwMessage(CCU_MSG_IMS_PLAY_STOPPED, NAME(CCU_MSG_IMS_PLAY_STOPPED)),
			  playback_handle(IW_UNDEFINED),
			  error(API_FAILURE){};

		  int playback_handle;

		  ApiErrorCode error;
	};

	class CcuMsgImsTearDownReq:
		public MsgRequest
	{
	public:
		CcuMsgImsTearDownReq():
		  MsgRequest(CCU_MSG_IMS_TEARDOWN_REQ, 
			  NAME(CCU_MSG_IMS_TEARDOWN_REQ)){};

		  ImsHandleId handle;

	};

	class CcuMsgStopPlaybackReq:
		public MsgRequest
	{

	public:
		CcuMsgStopPlaybackReq():
		  MsgRequest(CCU_MSG_STOP_PLAYBACK_REQUEST, 
			  NAME(CCU_MSG_STOP_PLAYBACK_REQUEST)){};

		  ImsHandleId handle;

	};


	class CcuMsgStopPlaybackAck:
		public MsgRequest
	{
	public:
		CcuMsgStopPlaybackAck():
		  MsgRequest(CCU_MSG_STOP_PLAYBACK_REQUEST_ACK, 
			  NAME(CCU_MSG_STOP_PLAYBACK_REQUEST_ACK)){};

	};


	class CcuMsgStopPlaybackNack:
		public MsgRequest
	{
	public:
		CcuMsgStopPlaybackNack():
		  MsgRequest(CCU_MSG_STOP_PLAYBACK_REQUEST_NACK, 
			  NAME(CCU_MSG_STOP_PLAYBACK_REQUEST_NACK)){};

	};

}

#pragma endregion Playback_Events




