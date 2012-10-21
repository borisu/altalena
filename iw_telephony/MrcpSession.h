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
using namespace boost;
#include "Telephony.h"

namespace ivrworx
{
#pragma region Mrcp_Events

	enum MrcpResource
	{
		UKNOWN_MRCP_RESOURCE,
		SYNTHESIZER,
		RECOGNIZER
	};

	enum MrcpEvents
	{
		MSG_MRCP_ALLOCATE_SESSION_REQ = MSG_USER_DEFINED,
		MSG_MRCP_ALLOCATE_SESSION_ACK,
		MSG_MRCP_ALLOCATE_SESSION_NACK,
		MSG_MRCP_SPEAK_REQ,
		MSG_MRCP_SPEAK_ACK,
		MSG_MRCP_SPEAK_NACK,
		MSG_MRCP_RECOGNIZE_REQ,
		MSG_MRCP_RECOGNIZE_ACK,
		MSG_MRCP_DEFINE_GRAMMAR_REQ,
		MSG_MRCP_DEFINE_GRAMMAR_ACK,
		MSG_MRCP_DEFINE_GRAMMAR_NACK,
		MSG_MRCP_RECOGNITION_COMPLETE_EVT,
		MSG_MRCP_RECOGNIZE_NACK,
		MSG_MRCP_STOP_SPEAK_REQ,
		MSG_MRCP_STOP_SPEAK_ACK,
		MSG_MRCP_STOP_SPEAK_NACK,
		MSG_MRCP_SPEAK_STOPPED_EVT,
		MSG_MRCP_TEARDOWN_REQ,
		MSG_MRCP_TEARDOWN_EVT,
		MSG_MRCP_MODIFY_REQ,
		MSG_MRCP_MODIFY_ACK,
		MSG_MRCP_MODIFY_NACK

	};

	typedef int 
	MrcpHandle;

	typedef map<string,string>
	MrcpParams;

	class IW_TELEPHONY_API MrcpMixin 
	{
	public :
		MrcpMixin():
		   mrcp_handle(IW_UNDEFINED),
		   correlation_id(IW_UNDEFINED), 
		  response_error_code(IW_UNDEFINED){};

		MrcpHandle mrcp_handle;

		int correlation_id;

		MrcpParams params;

		AbstractOffer offer;

		string body;

		int response_error_code;
	};		


	class IW_TELEPHONY_API MsgMrcpAllocateSessionReq:
		public MrcpMixin,
		public MsgRequest
	{
	public:
		MsgMrcpAllocateSessionReq():
		  MsgRequest(MSG_MRCP_ALLOCATE_SESSION_REQ, 
			  NAME(MSG_MRCP_ALLOCATE_SESSION_REQ)){};

		  LpHandlePair session_handler;

		  MrcpResource resource;

	};



	class IW_TELEPHONY_API MsgMrcpAllocateSessionAck:
		public IwMessage, public MrcpMixin
	{
	public:
		MsgMrcpAllocateSessionAck():
		  IwMessage(MSG_MRCP_ALLOCATE_SESSION_ACK, 
			  NAME(MSG_MRCP_ALLOCATE_SESSION_ACK))
			  {};

	};

	class IW_TELEPHONY_API MsgMrcpAllocateSessionNack:
		public IwMessage
	{
	public:
		MsgMrcpAllocateSessionNack():
		  IwMessage(MSG_MRCP_ALLOCATE_SESSION_NACK, 
			  NAME(MSG_MRCP_ALLOCATE_SESSION_NACK)){};

	};

	class IW_TELEPHONY_API MsgMrcpModifyReq:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpModifyReq():
		  MsgRequest(MSG_MRCP_MODIFY_REQ, 
			  NAME(MSG_MRCP_MODIFY_REQ)){};

	};


	class IW_TELEPHONY_API MsgMrcpModifyAck:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpModifyAck():
		  MsgRequest(MSG_MRCP_MODIFY_ACK, 
			  NAME(MSG_MRCP_MODIFY_ACK)){};

	};


	class IW_TELEPHONY_API MsgMrcpModifyNack:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpModifyNack():
		  MsgRequest(MSG_MRCP_MODIFY_NACK, 
			  NAME(MSG_MRCP_MODIFY_NACK)){};

	};

	class IW_TELEPHONY_API MsgMrcpSpeakReq:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpSpeakReq():
		  MsgRequest(MSG_MRCP_SPEAK_REQ, NAME(MSG_MRCP_SPEAK_REQ)){};


	};

	class IW_TELEPHONY_API MsgMrcpSpeakAck:
		public IwMessage, 
		public MrcpMixin
	{
	public:
		MsgMrcpSpeakAck():
		  IwMessage(MSG_MRCP_SPEAK_ACK, NAME(MSG_MRCP_SPEAK_ACK)){};

	};

	class IW_TELEPHONY_API MsgMrcpSpeakReqNack:
		public IwMessage, 
		public MrcpMixin
	{
	public:
		MsgMrcpSpeakReqNack():
		  IwMessage(MSG_MRCP_SPEAK_NACK, NAME(MSG_MRCP_SPEAK_NACK)) {};

	};

	class IW_TELEPHONY_API MsgMrcpSpeakStoppedEvt:
		public IwMessage, 
		public MrcpMixin
	{
	public:
		MsgMrcpSpeakStoppedEvt():
		  IwMessage(MSG_MRCP_SPEAK_STOPPED_EVT, NAME(MSG_MRCP_SPEAK_STOPPED_EVT)),
			  error(API_FAILURE){};

		  ApiErrorCode error;
	};

	class IW_TELEPHONY_API MsgMrcpTearDownReq:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpTearDownReq():
		  MsgRequest(MSG_MRCP_TEARDOWN_REQ, 
			  NAME(MSG_MRCP_TEARDOWN_REQ)){};

		 

	};

	class IW_TELEPHONY_API MsgMrcpTearDownEvt:
		public IwMessage, 
		public MrcpMixin
	{
	public:
		MsgMrcpTearDownEvt():
		  IwMessage(MSG_MRCP_TEARDOWN_EVT, 
			  NAME(MSG_MRCP_TEARDOWN_EVT)){};

		  

	};

	class IW_TELEPHONY_API MsgMrcpStopSpeakReq:
		public MsgRequest, 
		public MrcpMixin
	{

	public:
		MsgMrcpStopSpeakReq():
		  MsgRequest(MSG_MRCP_STOP_SPEAK_REQ, 
			  NAME(MSG_MRCP_STOP_SPEAK_REQ)){};

		 

	};


	class IW_TELEPHONY_API MsgMrcpStopSpeakAck:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpStopSpeakAck():
		  MsgRequest(MSG_MRCP_STOP_SPEAK_ACK, 
			  NAME(MSG_MRCP_STOP_SPEAK_ACK)){};

	};


	class IW_TELEPHONY_API MsgMrcpStopSpeakNack:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpStopSpeakNack():
		  MsgRequest(MSG_MRCP_STOP_SPEAK_NACK, 
			  NAME(MSG_MRCP_STOP_SPEAK_NACK)){};

	};



	

	
	class IW_TELEPHONY_API MsgMrcpRecognizeReq:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpRecognizeReq():
		  MsgRequest(MSG_MRCP_RECOGNIZE_REQ, 
			  NAME(MSG_MRCP_RECOGNIZE_REQ)){};


	};

	class IW_TELEPHONY_API MsgMrcpRecognizeAck:
		public MsgResponse, 
		public MrcpMixin
	{
	public:
		MsgMrcpRecognizeAck():
		  MsgResponse(MSG_MRCP_RECOGNIZE_ACK, 
			  NAME(MSG_MRCP_RECOGNIZE_ACK)){};

		  

	};

	class IW_TELEPHONY_API MsgMrcpRecognizeNack:
		public MsgResponse, 
		public MrcpMixin
	{
	public:
		MsgMrcpRecognizeNack():
		  MsgResponse(MSG_MRCP_RECOGNIZE_NACK, 
			  NAME(MSG_MRCP_RECOGNIZE_NACK)){};

	};

	class IW_TELEPHONY_API MsgMrcpRecognitionCompleteEvt:
		public IwMessage, 
		public MrcpMixin
	{
	public:
		MsgMrcpRecognitionCompleteEvt():
		  IwMessage(MSG_MRCP_RECOGNITION_COMPLETE_EVT, 
			  NAME(MSG_MRCP_RECOGNITION_COMPLETE_EVT)){};

	};


	class IW_TELEPHONY_API MsgMrcpDefineGrammarReq:
		public MsgRequest, 
		public MrcpMixin
	{
	public:
		MsgMrcpDefineGrammarReq():
		  MsgRequest(MSG_MRCP_DEFINE_GRAMMAR_REQ, 
			  NAME(MSG_MRCP_DEFINE_GRAMMAR_REQ)){};

	};

	class IW_TELEPHONY_API MsgMrcpDefineGrammarAck:
		public MsgResponse, 
		public MrcpMixin
	{
	public:
		MsgMrcpDefineGrammarAck():
		  MsgResponse(MSG_MRCP_DEFINE_GRAMMAR_ACK, 
			  NAME(MSG_MRCP_DEFINE_GRAMMAR_ACK)){};

	};

	class IW_TELEPHONY_API MsgMrcpDefineGrammarNack:
		public MsgResponse, 
		public MrcpMixin
	{
	public:
		MsgMrcpDefineGrammarNack():
		  MsgResponse(MSG_MRCP_DEFINE_GRAMMAR_NACK, 
			  NAME(MSG_MRCP_DEFINE_GRAMMAR_NACK)){};

	};


#pragma endregion Mrcp_Events

	typedef map<int,char> 
	DtmfMap;

	class IW_TELEPHONY_API MrcpSession :
		public ActiveObject
	{
	public:

		MrcpSession (IN ScopedForking &forking, IN HandleId handle_id);

		virtual ~MrcpSession (void);

		virtual ApiErrorCode	Allocate(IN MrcpResource rsc);

		virtual ApiErrorCode	Allocate(
			IN MrcpResource rsc,
			IN const AbstractOffer &localOffer, 
			IN Time timeout);

		virtual ApiErrorCode	ModifySession(
			IN const AbstractOffer &remote_offer, 
			IN Time timeout);

		virtual ApiErrorCode	StopSpeak();

		virtual ApiErrorCode Speak(
			IN const MrcpParams &p,
			IN const string &body,
			IN BOOL sync = TRUE);

		virtual ApiErrorCode Recognize(
			IN const MrcpParams &p,
			IN const string &body,
			IN Time timeout, 
			IN BOOL sync, 
			OUT string& answer);

		virtual ApiErrorCode WaitForRecogResult(
			IN Time timeout,
			OUT string &answer);

		virtual ApiErrorCode DefineGrammar(
			IN const MrcpParams &p,
			IN const string &body,
			IN Time timeout);

		virtual void TearDown();

		virtual void UponActiveObjectEvent(IwMessagePtr ptr);

		virtual void InterruptWithHangup();

		virtual MrcpHandle SessionHandle();

		AbstractOffer RemoteOffer(MrcpResource rsc);

		AbstractOffer LocalOffer(MrcpResource rsc);

	private:

		ApiErrorCode	SyncStreamFile();

		MrcpHandle _mrcpSessionHandle;

		ScopedForking &_forking;

		LpHandlePtr _hangupHandle;

		LpHandlePtr _playStoppedHandle;

		LpHandlePtr _recognitionStoppedHandle;

		LpHandlePair _mrcpSessionHandlerPair;

		HandleId _mrcpServiceHandleId;

		typedef map<MrcpResource, AbstractOffer> 
		OffersMap;

		OffersMap _remoteOffers;
		OffersMap _localOffers;


	};


	typedef shared_ptr<MrcpSession> MrcpSessionPtr;


}

