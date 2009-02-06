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

#include "Ccu.h"
#include "StreamingObject.h"
#include "LightweightProcess.h"

enum CcuImsEvents
{
	CCU_MSG_STREAMER_ADD_REQ = CCU_MSG_USER_DEFINED + 1,
	CCU_MSG_STREAMER_REMOVE_REQ,	
	CCU_MSG_STREAMER_REMOVE_ACK,
	CCU_MSG_STREAMING_STOPPED_EVT,
};

using namespace std;

typedef
set<StreamingObject*> StreamingObjectSet;

class CcuMsgAddStreamingObjectReq
	: public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(id);
	}
public:

	CcuMsgAddStreamingObjectReq():
	IxMessage(CCU_MSG_STREAMER_ADD_REQ, NAME(CCU_MSG_STREAMER_ADD_REQ)),
		obj(NULL){};

	StreamingObject *obj;

	CcuConnectionId id;

};
BOOST_CLASS_EXPORT(CcuMsgAddStreamingObjectReq)

class CcuMsgRemoveStreamingObjectReq
	: public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(handle);
	}
public:

	CcuMsgRemoveStreamingObjectReq():
	IxMessage(CCU_MSG_STREAMER_REMOVE_REQ, NAME(CCU_MSG_STREAMER_REMOVE_REQ)),
		handle(IX_UNDEFINED){};

	int handle;
};
BOOST_CLASS_EXPORT(CcuMsgRemoveStreamingObjectReq)


class CcuMsgRemoveStreamingObjectAck
	: public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
	}
public:

	CcuMsgRemoveStreamingObjectAck():
	  IxMessage(CCU_MSG_STREAMER_REMOVE_ACK, NAME(CCU_MSG_STREAMER_REMOVE_ACK)),
		  obj(NULL){};

	  StreamingObject *obj;
};
BOOST_CLASS_EXPORT(CcuMsgRemoveStreamingObjectAck)

class CcuMsgStreamingStopped
	: public IxMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(IxMessage);
		SERIALIZE_FIELD(error);
	}
public:

	CcuMsgStreamingStopped():
	IxMessage(CCU_MSG_STREAMING_STOPPED_EVT, NAME(CCU_MSG_STREAMING_STOPPED_EVT)),
		obj(NULL){};

	StreamingObject *obj;

	IxApiErrorCode error;
};
BOOST_CLASS_EXPORT(CcuMsgStreamingStopped)

class ProcStreamer :
	public LightweightProcess
{
public:
	ProcStreamer(LpHandlePair pair, LpHandlePtr ims_inbound);

	virtual ~ProcStreamer(void);

	virtual void real_run();

private:

	virtual void AddStreamingObject(IxMsgPtr req);

	virtual void RemoveStreamingObject(IxMsgPtr req);

	StreamingObjectSet _streamingObjectsSet;

	LpHandlePtr _imsInbound;

};
