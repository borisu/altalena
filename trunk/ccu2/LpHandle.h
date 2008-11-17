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

#include "CcuCommon.h"
#include "CcuMessage.h"
#include "UIDOwner.h"


#define SL(...) CCU_MSG_MARKER,__VA_ARGS__,CCU_MSG_MARKER

#define ARR(...) CCU_MSG_MARKER[]{__VA_ARGS__}

#define CCU_MAX_MESSAGES_IN_QUEUE 1000

#define DECLARE_NAMED_HANDLE_PAIR(P) LpHandlePair P (LpHandlePtr(new LpHandle()), LpHandlePtr(new LpHandle()))
#define HANDLE_PAIR LpHandlePair(LpHandlePtr(new LpHandle()), LpHandlePtr(new LpHandle()))


using namespace csp;
using namespace std;
using namespace boost;


class Interruptor
{
public:

	virtual void SignalDataIn() = 0;

	virtual void SignalDataOut() = 0;
};

typedef 
shared_ptr<Interruptor> InterruptorPtr;

class SemaphoreInterruptor
	:public Interruptor, boost::noncopyable
{
public:

	SemaphoreInterruptor();

	virtual ~SemaphoreInterruptor();

	virtual void SignalDataIn();

	virtual void SignalDataOut();

	virtual HANDLE Handle();

private:

	HANDLE _handle;

};

typedef 
shared_ptr<SemaphoreInterruptor> SemaphoreInterruptorPtr;

typedef 
BufferedAny2OneChannel<CcuMsgPtr> CcuChannel;

typedef 
shared_ptr<CcuChannel> CcuChannelPtr;

typedef 
FIFOBuffer<CcuMsgPtr> CCUFifoBuffer;

typedef 
SizedChannelBufferFactoryImpl<CcuMsgPtr, CCUFifoBuffer> CcuBufferFactory;

typedef
set<int> EventsSet;


class IFirstChanceOOBMsgHandler
{
public:
	virtual BOOL HandleOOBMessage(CcuMsgPtr ptr) = 0;
};

enum CcuHandleDirection
{
	CCU_MSG_DIRECTION_UNDEFINED,
	CCU_MSG_DIRECTION_INBOUND,
	CCU_MSG_DIRECTION_OUTBOUND
};


class LpHandle;

typedef 
shared_ptr<LpHandle> LpHandlePtr;

typedef 
vector<LpHandlePtr> HandlesList;

class LpHandle :
	public UIDOwner
{
	
public:

	LpHandle();

	virtual ~LpHandle(void);

	virtual void Direction(
		IN CcuHandleDirection val);

	virtual CcuHandleDirection Direction() const;

	virtual void HandleInterruptor(
		IN InterruptorPtr interruptor);

	virtual void FirstChanceOOBMsgHandler(
		IN IFirstChanceOOBMsgHandler *handler);

	virtual void Send(
		IN CcuMsgPtr message);

	virtual void Send(
		IN CcuMessage *message);
	
	virtual CcuMsgPtr Wait();

	virtual CcuMsgPtr Wait(
		IN Time timeout,
		OUT CcuApiErrorCode &res);

	virtual CcuMsgPtr WaitForMessages(
		IN Time timeout,
		OUT CcuApiErrorCode &res,
		IN CcuMessageId marker,
		IN ...);

	virtual CcuMsgPtr  WaitForMessages(
		IN const  Time timeout, 
		IN const EventsSet &msg_id_map, 
		OUT CcuApiErrorCode &res);

	virtual void OwnerProc(
		IN int parentProcId, 
		IN const wstring &procName);

	virtual bool InboundPending();

	virtual wstring OwnerProcName() const;

	virtual void OwnerProcName(
		IN const wstring &val);

private:

	CcuHandleDirection _direction;

	virtual CcuMsgPtr  WaitForMessages(
		IN const  Time &timeout, 
		IN const EventsSet &msg_id_map, 
		IN CcuChannel &channel,
		OUT CcuApiErrorCode &res);

	void SendToChannel(
		IN CcuChannel &channel, 
		IN CcuMsgPtr message, 
		IN bool interrupt);

	CcuBufferFactory _bufferFactory;

	CcuChannel _channel;

	InterruptorPtr _interruptor;

	int _ownerProcId ;

	wstring _ownerProcName;

	IFirstChanceOOBMsgHandler *_firstChanceHandler;
	
	friend wostream& operator << (wostream &ostream, const LpHandle *lpHandlePtr);
	
	friend CcuApiErrorCode SelectFromChannels(
		IN  HandlesList &map,
		IN  Time timeout, 
		OUT int &index, 
		OUT CcuMsgPtr &event,
		IN  bool peekOnly);

};



CcuApiErrorCode SelectFromChannels(IN  HandlesList &map,
								   IN  Time timeout, 
								   OUT int &index, 
								   OUT CcuMsgPtr &event,
								   IN  bool peekOnly = false);


struct LpHandlePair
{
	LpHandlePair();

	LpHandlePair(IN LpHandlePtr inbound, IN LpHandlePtr outbound);

	LpHandlePair(const LpHandlePair &other);
	
	LpHandlePtr inbound;

	LpHandlePtr outbound;

};

#define CCU_NULL_LP_HANDLE LpHandlePtr((LpHandle*) NULL)

