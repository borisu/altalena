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
#include "CcuLogger.h"
#include "Ims.h"

using namespace boost;

#pragma TODO ("Unify facade initiation logic into base class")
#pragma TODO ("Make it more efficient and able to play different files on the same IMS session handle")

ImsSession::ImsSession(LightweightProcess &facade):
_imsSessionHandle(IX_UNDEFINED),
_facade(facade)
{

}

ImsSession::~ImsSession(void)
{
	if (_imsSessionHandle != IX_UNDEFINED)
	{
#pragma TODO ("Implement Close Stream Logic")		

	}
}



IxApiErrorCode
ImsSession::PlayFile(IN CnxInfo destination, 
					 IN const wstring &file_name)
{
	
	IxApiErrorCode res = CCU_API_SUCCESS;
	if (_imsSessionHandle == IX_UNDEFINED)
	{
		
		res = AllocateIMSConnection(destination, file_name);
		if (CCU_FAILURE(res))
		{
			return res;
		}
	}
	
	res = SyncStreamFile();

	return res;

}

IxApiErrorCode
ImsSession::AllocateIMSConnection(IN CnxInfo remote_end, 
								  IN const wstring &file_name)
{
	FUNCTRACKER;

	if (_imsSessionHandle != IX_UNDEFINED)
	{
		return CCU_API_SUCCESS;
	}

	IxMsgPtr response = CCU_NULL_MSG;

	CcuMsgAllocateImsSessionReq *msg = new CcuMsgAllocateImsSessionReq();
	msg->file_name = file_name;
	msg->remote_media_data = remote_end;

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
			_imsMediaData = ack->ims_media;

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


IxApiErrorCode
ImsSession::SyncStreamFile()
{
	FUNCTRACKER;
	IxMsgPtr response = CCU_NULL_MSG;

	CcuMsgStartPlayReq *msg = new CcuMsgStartPlayReq();
	msg->playback_handle = _imsSessionHandle;
	msg->send_provisional = false;

	IxApiErrorCode res =_facade.DoRequestResponseTransaction(
		IMS_Q,
		IxMsgPtr(msg),
		response,
		Time(Seconds(60)),
		L"Synchronous Streaming File TXN");

#pragma  TODO ("Make Ims session reusable")

	_imsSessionHandle = IX_UNDEFINED;

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
