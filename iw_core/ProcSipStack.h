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

#include "IwBase.h"
#include "LightweightProcess.h"
#include "UASDialogUsageManager.h"
#include "UACDialogUsageManager.h"
#include "Call.h"
#include "Logger.h"


using namespace resip;

namespace ivrworx
{

	class IwResipLogger :
		public ExternalLogger
	{
	public:
		virtual ~IwResipLogger();

		/** return true to also do default logging, false to supress default logging. */
		virtual bool operator()(Log::Level level,
			const Subsystem& subsystem, 
			const Data& appName,
			const char* file,
			int line,
			const Data& message,
			const Data& messageWithHeaders);

	};


	typedef
	SharedPtr<DialogUsageManager> DialogUsageManagerPtr;

	typedef
	shared_ptr<UASDialogUsageManager> UASDialogUsageManagerPtr;

	typedef
	shared_ptr<UACDialogUsageManager> UACDialogUsageManagerPtr;

	typedef
	shared_ptr<SelectInterruptor> SelectInterruptorPtr;

	typedef
	shared_ptr<SipStack> SipStackPtr;		


	class ResipInterruptor
		:public WaitInterruptor, 
		public SelectInterruptor, 
		public noncopyable
	{

	public:

		ResipInterruptor();

		virtual void SignalDataIn();

		virtual void SignalDataOut();

	};

	typedef
	shared_ptr<ResipInterruptor> ResipInterruptorPtr;




	class ProcSipStack : 
		public LightweightProcess, public SipSessionHandlerAdapter
	{
	public:

		ProcSipStack(
			IN LpHandlePair pair, 
			IN Configuration &conf);

		virtual ~ProcSipStack(void);

		virtual void SetResipLogLevel();

		virtual void real_run();

		virtual ApiErrorCode Init();

		virtual void UponBlindXferReq(IN IwMessagePtr req);

		virtual void UponMakeCallReq(IN IwMessagePtr req);

		virtual void UponMakeCallAckReq(IN IwMessagePtr req);

		virtual void UponHangupCallReq(IN IwMessagePtr req);

		virtual void ShutDown(IN IwMessagePtr req);

		virtual void UponCallOfferedAck(IN IwMessagePtr req);

		virtual void UponCallOfferedNack(IN IwMessagePtr req);

		virtual void onNewSession(
			IN ClientInviteSessionHandle s, 
			IN InviteSession::OfferAnswerType oat, 
			IN const SipMessage& msg);

		virtual void onConnected(
			IN ClientInviteSessionHandle is, 
			IN const SipMessage& msg);

		/// Received a failure response from UAS
		virtual void onFailure(
			IN ClientInviteSessionHandle, 
			IN const SipMessage& msg);

		virtual void onNewSession(
			IN ServerInviteSessionHandle sis, 
			IN InviteSession::OfferAnswerType oat, 
			IN const SipMessage& msg);

		virtual void onConnectedConfirmed(
			IN InviteSessionHandle, 
			IN const SipMessage &msg);

		virtual void onOffer(
			IN InviteSessionHandle is, 
			IN const SipMessage& msg, 
			IN const SdpContents& sdp);

		virtual void onReceivedRequest(
			IN ServerOutOfDialogReqHandle ood, 
			IN const SipMessage& request);

		virtual void onTerminated(
			IN InviteSessionHandle, 
			IN InviteSessionHandler::TerminatedReason reason, 
			IN const SipMessage* msg);

		virtual void ShutDown();

	protected:

		virtual bool ProcessIwMessages();

		bool _shutDownFlag;

		IwResipLogger _logger;

		ResipInterruptorPtr _handleInterruptor;

		Configuration &_conf;

		typedef map<string,Profile::SessionTimerMode> 
			ConfSessionTimerModeMap;

		ConfSessionTimerModeMap _confSessionTimerModeMap;


		SipStackPtr _stack;

		IwHandlesMap _iwHandlesMap;

		ResipDialogHandlesMap _resipHandlesMap;

		typedef
		shared_ptr<DialogUsageManager> DialogUsageManagerPtr;
		DialogUsageManagerPtr _dumMngr;

		UACDialogUsageManagerPtr _dumUac;

		UASDialogUsageManagerPtr _dumUas;

		

		

	};

}










