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

#define IMS_Q 1

namespace ivrworx
{

#pragma region Playback_Events

	enum ImsEvents
	{
		MSG_IMS_ALLOCATE_SESSION_REQ = MSG_USER_DEFINED,
		MSG_IMS_ALLOCATE_SESSION_ACK,
		MSG_IMS_ALLOCATE_SESSION_NACK,
		MSG_IMS_PLAY_REQUEST,
		MSG_IMS_PLAY_ACK,
		MSG_IMS_PLAY_NACK,
		MSG_IMS_STOP_PLAY_REQ,
		MSG_IMS_STOP_PLAY_ACK,
		MSG_IMS_STOP_PLAY_NACK,
		MSG_IMS_PLAY_STOPPED_EVT,
		MSG_IMS_TEARDOWN_REQ,
		MSG_IMS_MODIFY_REQ,
		MSG_IMS_MODIFY_ACK,
		MSG_IMS_MODIFY_NACK

	};

	typedef int 
	ImsHandle;

	class ImsMixin
	{
	public:

		ImsMixin():
		  ims_handle(IW_UNDEFINED),
			  correlation_id(IW_UNDEFINED){}

		CnxInfo remote_media_data;

		MediaFormat codec;

		LpHandlePair session_handler;

		ImsHandle ims_handle;

		int correlation_id;



	};


	class MsgImsAllocateSessionReq:
		public MsgRequest,
		public ImsMixin
	{
	public:
		MsgImsAllocateSessionReq():
		  MsgRequest(MSG_IMS_ALLOCATE_SESSION_REQ, 
			  NAME(MSG_IMS_ALLOCATE_SESSION_REQ)){};

		
	};

	

	class MsgImsAllocateSessionAck:
		public IwMessage,
		public ImsMixin
	{
	public:
		MsgImsAllocateSessionAck():
		  IwMessage(MSG_IMS_ALLOCATE_SESSION_ACK, 
			  NAME(MSG_IMS_ALLOCATE_SESSION_ACK)){};

	};

	class MsgImsAllocateSessionNack:
		public IwMessage,
		public ImsMixin
	{
	public:
		MsgImsAllocateSessionNack():
		  IwMessage(MSG_IMS_ALLOCATE_SESSION_NACK, 
			  NAME(MSG_IMS_ALLOCATE_SESSION_NACK)){};

	};

	class MsgImsModifyReq:
		public MsgRequest,
		public ImsMixin
	{
	public:
		MsgImsModifyReq():
		  MsgRequest(MSG_IMS_MODIFY_REQ, 
			  NAME(MSG_IMS_MODIFY_REQ)){};

	
	};


	class MsgImsModifyAck:
		public MsgRequest,
		public ImsMixin
	{
	public:
		MsgImsModifyAck():
		  MsgRequest(MSG_IMS_MODIFY_ACK, 
			  NAME(MSG_IMS_MODIFY_ACK)){};
	  
	};


	class MsgImsModifyNack:
		public MsgRequest,
		public ImsMixin
	{
	public:
		MsgImsModifyNack():
		  MsgRequest(MSG_IMS_MODIFY_NACK, 
			  NAME(MSG_IMS_MODIFY_NACK)){};

	};

	class MsgImsPlayReq:
		public MsgRequest,
		public ImsMixin
	{
	public:
		MsgImsPlayReq():
		  MsgRequest(MSG_IMS_PLAY_REQUEST, NAME(MSG_IMS_PLAY_REQUEST)),
			  loop(false){};

		  string file_name;

		  BOOL loop;

	};

	class MsgImsPlayAck:
		public IwMessage,
		public ImsMixin
	{
	public:
		MsgImsPlayAck():
		  IwMessage(MSG_IMS_PLAY_ACK, NAME(MSG_IMS_PLAY_ACK)){};

	};

	class MsgImsPlayNack:
		public IwMessage,
		public ImsMixin
	{
	public:
		MsgImsPlayNack():
		  IwMessage(MSG_IMS_PLAY_NACK, NAME(MSG_IMS_PLAY_NACK)){};

	};

	class MsgImsPlayStopped:
		public IwMessage,
		public ImsMixin
	{
	public:
		MsgImsPlayStopped():
		  IwMessage(MSG_IMS_PLAY_STOPPED_EVT, NAME(MSG_IMS_PLAY_STOPPED_EVT)){};

	};

	class MsgImsTearDownReq:
		public MsgRequest,
		public ImsMixin
	{
	public:
		MsgImsTearDownReq():
		  MsgRequest(MSG_IMS_TEARDOWN_REQ, 
			  NAME(MSG_IMS_TEARDOWN_REQ)){};

	};

	class MsgImsStopPlayReq:
		public MsgRequest,
		public ImsMixin
	{

	public:
		MsgImsStopPlayReq():
		  MsgRequest(MSG_IMS_STOP_PLAY_REQ, 
			  NAME(MSG_IMS_STOP_PLAY_REQ)){};

	};


	class MsgImsStopPlayAck:
		public MsgRequest,
		public ImsMixin
	{
	public:
		MsgImsStopPlayAck():
		  MsgRequest(MSG_IMS_STOP_PLAY_ACK, 
			  NAME(MSG_IMS_STOP_PLAY_ACK)){};

	};


	class MsgImsStopPlayNack:
		public MsgRequest,
		public ImsMixin
	{
	public:
		MsgImsStopPlayNack():
		  MsgRequest(MSG_IMS_STOP_PLAY_NACK, 
			  NAME(MSG_IMS_STOP_PLAY_NACK)){};

	};


}

#pragma endregion Playback_Events




