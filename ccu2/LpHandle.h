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

using namespace csp;
using namespace std;
using namespace boost;

namespace ivrworx 
{

	#define CCU_MAX_MESSAGES_IN_QUEUE 1000

	#define DECLARE_NAMED_HANDLE(P) LpHandlePtr P (new LpHandle())
	#define DECLARE_NAMED_HANDLE_PAIR(P) LpHandlePair P (LpHandlePtr(new LpHandle()), LpHandlePtr(new LpHandle()))
	#define HANDLE_PAIR LpHandlePair(LpHandlePtr(new LpHandle()), LpHandlePtr(new LpHandle()))


	// ix messages come with this completion keys
	#define IOCP_UNIQUE_COMPLETION_KEY 555 



	//
	// Some classes are not csp-fibers and need to be 
	// interrupted by setting some event or any other
	// OS related measure while they are waiting. This 
	// class represents the interface which wraps the OS 
	// related facility which used to interrupt such a
	// process and inform it that new message has arrived
	//
	class IxInterruptor
	{
	public:

		virtual void SignalDataIn() = 0;

		virtual void SignalDataOut() = 0;
	};

	typedef 
		shared_ptr<IxInterruptor> InterruptorPtr;

	// use this interruptor if you want to receive messages 
	// by waiting on specific HANDLE by means of WaitForMultipleObjects 
	class SemaphoreInterruptor
		:public IxInterruptor, boost::noncopyable
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


	// use this interruptor if you want to receive messages 
	// by waiting on io completion port by  means of  GetQueuedCompletionStatus  
	class IocpInterruptor
		:public IxInterruptor, boost::noncopyable
	{
	public:

		IocpInterruptor();

		virtual ~IocpInterruptor();

		virtual void SignalDataIn();

		virtual void SignalDataOut();

		HANDLE Handle();
		
	private:

		HANDLE _iocpHandle;

	};

	typedef 
		shared_ptr<IocpInterruptor> IocpInterruptorPtr;

	typedef 
		BufferedAny2OneChannel<IxMsgPtr> CcuChannel;

	typedef 
		shared_ptr<CcuChannel> CcuChannelPtr;

	typedef 
		FIFOBuffer<IxMsgPtr> CCUFifoBuffer;

	typedef 
		SizedChannelBufferFactoryImpl<IxMsgPtr, CCUFifoBuffer> CcuBufferFactory;

	enum IxHandleDirection
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
			IN IxHandleDirection val);

		virtual IxHandleDirection Direction() const;

		virtual void HandleInterruptor(
			IN InterruptorPtr interruptor);

		virtual IxApiErrorCode Send(
			IN IxMsgPtr message);

		virtual IxApiErrorCode Send(
			IN IxMessage *message);

		virtual IxMsgPtr Wait(
			IN Time timeout,
			OUT IxApiErrorCode &res);

		virtual bool InboundPending();

		virtual wstring HandleName() const;

		virtual void HandleName(
			IN const wstring &val);

		virtual void Poison();

	private:

		IxMsgPtr Read();

		IxHandleDirection _direction;

		CcuBufferFactory _bufferFactory;

		CcuChannel _channel;

		InterruptorPtr _interruptor;

		wstring _name;

		friend wostream& operator << (wostream &ostream, const LpHandle *lpHandlePtr);

		friend IxApiErrorCode SelectFromChannels(
			IN  HandlesList &map,
			IN  Time timeout, 
			OUT int &index, 
			OUT IxMsgPtr &event);

	};



	IxApiErrorCode SelectFromChannels(IN  HandlesList &map,
		IN  Time timeout, 
		OUT int &index, 
		OUT IxMsgPtr &event);


	struct LpHandlePair
	{
		LpHandlePair();

		LpHandlePair(IN LpHandlePtr inbound, IN LpHandlePtr outbound);

		LpHandlePair(const LpHandlePair &other);

		LpHandlePtr inbound;

		LpHandlePtr outbound;

	};

}

#define CCU_NULL_LP_HANDLE LpHandlePtr((LpHandle*) NULL)

