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

#include "IwUtils.h"
#include "IwBase.h"
#include "Message.h"
#include "UIDOwner.h"

using namespace csp;
using namespace std;
using namespace boost;

namespace ivrworx
{

	#define MAX_MESSAGES_IN_QUEUE 1000

	#define DECLARE_NAMED_HANDLE(P) LpHandlePtr P (new LpHandle())
	#define DECLARE_NAMED_HANDLE_PAIR(P) LpHandlePair P (LpHandlePtr(new LpHandle()), LpHandlePtr(new LpHandle()))
	#define HANDLE_PAIR LpHandlePair(LpHandlePtr(new LpHandle()), LpHandlePtr(new LpHandle()))


	// iw messages come with this completion keys
	#define IOCP_UNIQUE_COMPLETION_KEY 555



	//
	// Some classes are not csp-fibers and need to be
	// interrupted by setting some event or any other
	// OS related measure while they are waiting. This
	// class IW_CORE_API represents the interface which wraps the OS
	// related facility which used to interrupt such a
	// process and inform it that new message has arrived
	//
	class IW_CORE_API WaitInterruptor
	{
	public:

		virtual void SignalDataIn() = 0;

		virtual void SignalDataOut() = 0;
	};

	typedef
	shared_ptr<WaitInterruptor> InterruptorPtr;

	// use this interruptor if you want to receive messages
	// by waiting on specific HANDLE by means of WaitForMultipleObjects
	class IW_CORE_API SemaphoreInterruptor
	:public WaitInterruptor, noncopyable
	{
	public:

		SemaphoreInterruptor();

		virtual ~SemaphoreInterruptor();

		virtual void SignalDataIn();

		virtual void SignalDataOut();

		virtual HANDLE WinHnd();

	private:

		HANDLE _handle;

	};

	typedef
	shared_ptr<SemaphoreInterruptor> SemaphoreInterruptorPtr;


	// use this interruptor if you want to receive messages
	// by waiting on io completion port by  means of  GetQueuedCompletionStatus
	class IW_CORE_API IocpInterruptor
		:public WaitInterruptor, noncopyable
	{
	public:

		IocpInterruptor();

		virtual ~IocpInterruptor();

		virtual void SignalDataIn();

		virtual void SignalDataOut();

		HANDLE WinHandle();

	private:

		HANDLE _iocpHandle;

	};

	typedef
	shared_ptr<IocpInterruptor> IocpInterruptorPtr;

	typedef
	BufferedAny2OneChannel<IwMessagePtr> CommChannel;

	typedef
	shared_ptr<CommChannel> CommChannelPtr;

	typedef
	FIFOBuffer<IwMessagePtr> IwFifoBuffer;

	typedef
	SizedChannelBufferFactoryImpl<IwMessagePtr, IwFifoBuffer> IwBufferFactory;

	enum HandleDirection
	{
		MSG_DIRECTION_UNDEFINED,
		MSG_DIRECTION_INBOUND,
		MSG_DIRECTION_OUTBOUND
	};


	class IW_CORE_API LpHandle;

	typedef
	shared_ptr<LpHandle> LpHandlePtr;

	typedef
	vector<LpHandlePtr> HandlesVector;

	typedef
	shared_ptr<HandlesVector> HandlesVectorPtr;

	class IW_CORE_API LpHandle :
	public UIDOwner
	{

	public:

		LpHandle();

		virtual ~LpHandle(void);

		virtual void Direction(IN HandleDirection val);

		virtual HandleDirection Direction() const;

		virtual void HandleInterruptor(
			IN InterruptorPtr interruptor);

		virtual ApiErrorCode Send(
			IN IwMessagePtr message);

		virtual ApiErrorCode Send(
			IN IwMessage *message);

		virtual IwMessagePtr Wait(
			IN Time timeout,
			OUT ApiErrorCode &res);

		virtual bool InboundPending();

		virtual string HandleName() const;

		virtual void HandleName(
			IN const string &val);

		virtual void Poison();

		virtual BOOL PoisonedForWrite();

		virtual int  Size();

	private:

		void inline CheckReader();

		IwMessagePtr Read();

		DWORD _threadId;

		PVOID _fiberId;

		HandleDirection _direction;

		IwBufferFactory _bufferFactory;

		CommChannel _channel;

		InterruptorPtr _interruptor;

		string _name;

		int _size;

		friend ostream& operator << (ostream &ostream, const LpHandle *lpHandlePtr);

		IW_CORE_API friend ApiErrorCode SelectFromChannels(
			IN  HandlesVector &map,
			IN  Time timeout,
			OUT int &index,
			OUT IwMessagePtr &event);

	};



	IW_CORE_API ApiErrorCode SelectFromChannels(
		IN  HandlesVector &map,
		IN  Time timeout,
		OUT int &index,
		OUT IwMessagePtr &event);


	struct IW_CORE_API LpHandlePair
	{
		LpHandlePair();

		LpHandlePair(IN LpHandlePtr inbound, IN LpHandlePtr outbound);

		LpHandlePair(const LpHandlePair &other);

		LpHandlePtr inbound;

		LpHandlePtr outbound;

	};

}

#define IW_NULL_HANDLE LpHandlePtr((LpHandle*) NULL)

