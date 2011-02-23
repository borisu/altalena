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


	// this trick only to make queue member available
	class IwDialogUsageManager: public DialogUsageManager
	{
		IwDialogUsageManager(SipStack &sipStack):
	DialogUsageManager(sipStack){};
	public:
		friend class ResipInterruptor;
		friend class ProcResipStack;

		virtual ~IwDialogUsageManager(){};
	};

	typedef
	SharedPtr<IwDialogUsageManager> IwDialogUsageManagerPtr;


	class ResipInterruptor
		:public WaitInterruptor,
		 public noncopyable
	{

	public:

		ResipInterruptor(IwDialogUsageManager *dum);

		virtual void SignalDataIn();

		virtual void SignalDataOut();

		virtual void Destroy();

		virtual ~ResipInterruptor();

	private:

		IwDialogUsageManager* _dumPtr;

		mutex _mutex;

	};

	typedef
	shared_ptr<ResipInterruptor> ResipInterruptorPtr;

	class ProcResipStack : 
		public LightweightProcess, 
		public SipSessionHandlerAdapter
	{
	public:

		ProcResipStack(
			IN LpHandlePair pair, 
			IN ConfigurationPtr conf);

		virtual ~ProcResipStack(void);

		virtual void SetResipLogLevel();

		virtual void real_run();

		virtual ApiErrorCode Init();

		virtual void UponSubscribeToIncomingReq(IN IwMessagePtr req);

		virtual void UponInfoReq(IN IwMessagePtr req);

		virtual void UponBlindXferReq(IN IwMessagePtr req);

		virtual void UponMakeCallReq(IN IwMessagePtr req);

		virtual void UponHangupCallReq(IN IwMessagePtr req);

		virtual void UponShutDownReq(IN IwMessagePtr req);

		virtual void UponRegisterReq(IN IwMessagePtr req);

		virtual void UponUnRegisterReq(IN IwMessagePtr req);

		virtual void UponCallOfferedAck(IN IwMessagePtr req);

		virtual void UponCallOfferedNack(IN IwMessagePtr req);

		virtual void UponCallConnected(IN IwMessagePtr req);

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

		// generic offer/answer
		virtual void onOffer(
			InviteSessionHandle h, 
			const SipMessage& msg, 
			const Contents& body);

		// generic offer/answer 
		virtual void onAnswer(
			InviteSessionHandle h, 
			const SipMessage& msg, 
			const Contents& body);

		// generic offer/answer  
		virtual void onRemoteAnswerChanged(
			InviteSessionHandle h, 
			const SipMessage& msg, 
			const Contents& body);

		// generic offer/answer  
		virtual void onEarlyMedia(
			ClientInviteSessionHandle h, 
			const SipMessage& msg, 
			const Contents& body);

		virtual void onReceivedRequest(
			IN ServerOutOfDialogReqHandle ood, 
			IN const SipMessage& request);

		virtual void onTerminated(
			IN InviteSessionHandle, 
			IN InviteSessionHandler::TerminatedReason reason, 
			IN const SipMessage* msg);

		virtual void onInfo(
			IN InviteSessionHandle, 
			IN const SipMessage& msg);

		/// called when response to INFO message is received 
		virtual void onInfoSuccess(
			IN InviteSessionHandle, 
			IN const SipMessage& msg);

		virtual void onInfoFailure(
			IN InviteSessionHandle, 
			IN const SipMessage& msg);

		virtual void onSessionExpired(
			IN InviteSessionHandle is);
	
		virtual void ShutdownStack();

		virtual void FinalizeContext(SipDialogContextPtr ctx);

	protected:

		virtual bool ProcessApplicationMessages();


		//
		// ConfigurationPtr  logging
		//
		IwResipLogger _logger;

		ConfigurationPtr _conf;

		typedef map<string,Profile::SessionTimerMode> 
		ConfSessionTimerModeMap;

		ConfSessionTimerModeMap _confSessionTimerModeMap;

		//
		// Resiprocate objects
		//

		SelectInterruptor _si;

		SipStack _stack;

		InterruptableStackThread _stackThread;
		
		//
		// Application objects
		//
		bool _shutDownFlag;

		IwHandlesMap _iwHandlesMap;

		ResipDialogHandlesMap _resipHandlesMap;

		ResipInterruptorPtr _dumInt;

		IwDialogUsageManager _dumMngr;

		UASDialogUsageManager _dumUas;

		UACDialogUsageManager _dumUac;

		LpHandlePtr _listener;

	};

}










