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
#include "ImsSession.h"
#include "Ims.h"

using namespace boost;

namespace ivrworx
{

ImsSession::ImsSession(LightweightProcess &facade):
_imsSessionHandle(IX_UNDEFINED),
_facade(facade)
{

}

ImsSession::~ImsSession(void)
{
	if (_imsSessionHandle != IX_UNDEFINED)
	{

	}
}

IxApiErrorCode
ImsSession::PlayFile( IN const wstring &file_name, IN BOOL sync)
{

	if (_imsSessionHandle == IX_UNDEFINED)
	{
		return CCU_API_FAILURE;
	}

	FUNCTRACKER;
	IxMsgPtr response = CCU_NULL_MSG;

	CcuMsgStartPlayReq *msg = new CcuMsgStartPlayReq();
	msg->playback_handle = _imsSessionHandle;
	msg->file_name = file_name;

	IxApiErrorCode res =_facade.DoRequestResponseTransaction(
		IMS_Q,
		IxMsgPtr(msg),
		response,
		Time(Seconds(60)),
		L"Start Streaming File TXN");

	if (CCU_FAILURE(res))
	{
		return res;
	}

	switch (msg->message_id)
	{
	case CCU_MSG_IMS_START_PLAY_REQ_ACK:
		{
			if (!sync)
			{
				return CCU_API_SUCCESS;
			};

			break;
		}
	default:
		return CCU_API_FAILURE;
	}

	throw "Not implemented";

}

IxApiErrorCode
ImsSession::AllocateIMSConnection(IN CnxInfo remote_end, 
								  IN IxCodec codec)
{
	FUNCTRACKER;

	if (_imsSessionHandle != IX_UNDEFINED)
	{
		return CCU_API_SUCCESS;
	}

	IxMsgPtr response = CCU_NULL_MSG;

	CcuMsgAllocateImsSessionReq *msg = new CcuMsgAllocateImsSessionReq();
	throw;
	msg->remote_media_data = remote_end;
	msg->codec = codec;

	IxApiErrorCode res = _facade.DoRequestResponseTransaction(
		IMS_Q,
		IxMsgPtr(msg),
		response,
		Time(MilliSeconds(_facade.TransactionTimeout())),
		L"Allocate IMS Connection TXN");

	if (res != CCU_API_SUCCESS)
	{
		return res;
	}

	switch (response->message_id)
	{
	case CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_ACK:
		{


			shared_ptr<CcuMsgAllocateImsSessionAck> ack = 
				shared_polymorphic_cast<CcuMsgAllocateImsSessionAck>(response);

			_imsSessionHandle = ack->playback_handle;
			throw;
			//_imsMediaData = ack->ims_media;

			break;

		}
	case CCU_MSG_ALLOCATE_PLAYBACK_SESSION_REQUEST_NACK:
		{
			res = CCU_API_SERVER_FAILURE;
			break;
		}
	default:
		{
			throw;
		}
	}
	return res;

}



CnxInfo ImsSession::ImsMediaData() const 
{ 
	return _imsMediaData; 
}

void ImsSession::ImsMediaData(IN CnxInfo val) 
{ 
	_imsMediaData = val; 
}
}