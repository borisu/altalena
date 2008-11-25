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
	: public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(id);
	}
public:

	CcuMsgAddStreamingObjectReq():
	CcuMessage(CCU_MSG_STREAMER_ADD_REQ, NAME(CCU_MSG_STREAMER_ADD_REQ)),
		obj(NULL){};

	StreamingObject *obj;

	CcuConnectionId id;

};
BOOST_CLASS_EXPORT(CcuMsgAddStreamingObjectReq)

class CcuMsgRemoveStreamingObjectReq
	: public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(handle);
	}
public:

	CcuMsgRemoveStreamingObjectReq():
	CcuMessage(CCU_MSG_STREAMER_REMOVE_REQ, NAME(CCU_MSG_STREAMER_REMOVE_REQ)),
		handle(CCU_UNDEFINED){};

	int handle;
};
BOOST_CLASS_EXPORT(CcuMsgRemoveStreamingObjectReq)


class CcuMsgRemoveStreamingObjectAck
	: public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
	}
public:

	CcuMsgRemoveStreamingObjectAck():
	  CcuMessage(CCU_MSG_STREAMER_REMOVE_ACK, NAME(CCU_MSG_STREAMER_REMOVE_ACK)),
		  obj(NULL){};

	  StreamingObject *obj;
};
BOOST_CLASS_EXPORT(CcuMsgRemoveStreamingObjectAck)

class CcuMsgStreamingStopped
	: public CcuMessage
{
	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_BASE_CLASS(CcuMessage);
		SERIALIZE_FIELD(error);
	}
public:

	CcuMsgStreamingStopped():
	CcuMessage(CCU_MSG_STREAMING_STOPPED_EVT, NAME(CCU_MSG_STREAMING_STOPPED_EVT)),
		obj(NULL){};

	StreamingObject *obj;

	CcuApiErrorCode error;
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

	virtual void AddStreamingObject(CcuMsgPtr req);

	virtual void RemoveStreamingObject(CcuMsgPtr req);

	StreamingObjectSet _streamingObjectsSet;

	LpHandlePtr _imsInbound;

};
