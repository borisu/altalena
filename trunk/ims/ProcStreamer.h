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

#include "CcuMessage.h"
#include "Ccu.h"
#include "StreamingObject.h"
#include "LightweightProcess.h"

enum CcuImsEvents
{
	CCU_MSG_STREAMER_ADD = CCU_MSG_USER_DEFINED + 1,
	CCU_MSG_STREAMER_REMOVE,	
	CCU_MSG_STREAMER_REMOVE_ACK,
	CCU_MSG_STREAMING_STOPPED,
};

using namespace std;

typedef
set<StreamingObject*> StreamingObjectSet;

class CcuMsgAddStreamingObject
	: public CcuMessage
{
public:

	CcuMsgAddStreamingObject():
	CcuMessage(CCU_MSG_STREAMER_ADD, NAME(CCU_MSG_STREAMER_ADD)),
		obj(NULL){};

	StreamingObject *obj;

	CcuConnectionId id;

};

class CcuMsgRemoveStreamingObject
	: public CcuMessage
{
public:

	CcuMsgRemoveStreamingObject():
	CcuMessage(CCU_MSG_STREAMER_REMOVE, NAME(CCU_MSG_STREAMER_REMOVE)),
		handle(CCU_UNDEFINED){};

	int handle;
};


class CcuMsgRemoveStreamingObjectAck
	: public CcuMessage
{
public:

	CcuMsgRemoveStreamingObjectAck():
	  CcuMessage(CCU_MSG_STREAMER_REMOVE_ACK, NAME(CCU_MSG_STREAMER_REMOVE_ACK)),
		  obj(NULL){};

	  StreamingObject *obj;
};

class CcuMsgStreamingStopped
	: public CcuMessage
{
public:

	CcuMsgStreamingStopped():
	CcuMessage(CCU_MSG_STREAMING_STOPPED, NAME(CCU_MSG_STREAMING_STOPPED)),
		obj(NULL){};

	StreamingObject *obj;

	CcuApiErrorCode error;
};

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
