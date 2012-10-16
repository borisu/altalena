#pragma once
#include "IwBase.h"
#include "Telephony.h"

using namespace ivrworx;

namespace ivrworx
{
	enum CallEvts
	{
		MSG_CALL_OFFERED = CALL_MSG_BASE,
		MSG_CALL_OFFERED_ACK,
		MSG_CALL_OFFERED_NACK,
		MSG_CALL_CONNECTED,
		MSG_CALL_HANG_UP_EVT,
		MSG_CALL_TERMINATED,
		MSG_CALL_DTMF_EVT,

		MSG_MAKE_CALL_REQ,
		MSG_MAKE_CALL_OK,
		MSG_MAKE_CALL_ACK,
		MSG_MAKE_CALL_NACK,

		MSG_CALL_REOFFER_REQ,
		MSG_CALL_REOFFER_ACK,
		MSG_CALL_REOFFER_NACK,

		
		MSG_HANGUP_CALL_REQ,

		MSG_CALL_BLIND_XFER_REQ,
		MSG_CALL_BLIND_XFER_ACK,
		MSG_CALL_BLIND_XFER_NACK,

		MSG_CALL_SUBSCRIBE_REQ,
		MSG_CALL_SUBSCRIBE_ACK,
		MSG_CALL_SUBSCRIBE_NACK,

		MSG_CALL_LAST_ONE

	};

	class IW_TELEPHONY_API  MsgVoipCallMixin 
	{
	public:

		MsgVoipCallMixin():
		  stack_call_handle(IW_UNDEFINED){};

		  int stack_call_handle;

		  string ani;

		  string dnis;

		  AbstractOffer localOffer;

		  AbstractOffer remoteOffer;

		  MapOfAny optional_params;

		  virtual void copy_data_on_response(IN IwMessage *msg)
		  {
			  MsgVoipCallMixin *req = dynamic_cast<MsgVoipCallMixin*>(msg);

			  stack_call_handle	 = stack_call_handle == IW_UNDEFINED ? req->stack_call_handle : stack_call_handle;
			  ani				 = ani.empty()  ?  req->ani  : ani;
			  dnis				 = dnis.empty() ?  req->dnis : dnis;
			  
		  };

	};

	class IW_TELEPHONY_API AuthenticationMixin
	{
	public:
		AuthenticationMixin(){};

		Credentials credentials;

	};


	//
	// CALL RELATED
	//
	class IW_TELEPHONY_API MsgMakeCallReq : 
		public MsgRequest, public MsgVoipCallMixin, public AuthenticationMixin
	{

	public:
		MsgMakeCallReq():
		  MsgRequest(MSG_MAKE_CALL_REQ, NAME(MSG_MAKE_CALL_REQ)){};

		  string destination_uri;

		  LpHandlePtr call_handler_inbound;

		  map<string,any> optional_params;

	};

	class IW_TELEPHONY_API MsgMakeCallOk : 
		public MsgResponse, public MsgVoipCallMixin
	{

	public:
		MsgMakeCallOk():
		  MsgResponse(MSG_MAKE_CALL_OK, NAME(MSG_MAKE_CALL_OK)){};

		  virtual void copy_data_on_response(IN IwMessage *request)
		  {
			  MsgVoipCallMixin::copy_data_on_response(request);
			  MsgResponse::copy_data_on_response(request);
		  }

	};


	class IW_TELEPHONY_API MsgMakeCallAckReq: 
		public MsgResponse, public MsgVoipCallMixin
	{
	public:

		MsgMakeCallAckReq(): 
		  MsgResponse(MSG_MAKE_CALL_ACK, NAME(MSG_MAKE_CALL_ACK)){};

		  int stack_call_handle;


	};


	class IW_TELEPHONY_API MsgMakeCallNack: 
		public MsgResponse, public MsgVoipCallMixin
	{
	public:
		MsgMakeCallNack():
		  MsgResponse(MSG_MAKE_CALL_NACK, NAME(MSG_MAKE_CALL_NACK)){};

		  virtual void copy_data_on_response(IN IwMessage *request)
		  {
			  MsgVoipCallMixin::copy_data_on_response(request);
			  MsgResponse::copy_data_on_response(request);
		  }
	};


	class IW_TELEPHONY_API MsgHangupCallReq: 
		public MsgRequest
	{
	public:
		MsgHangupCallReq():
		  MsgRequest(MSG_HANGUP_CALL_REQ, NAME(MSG_HANGUP_CALL_REQ)),
			  stack_call_handle(IW_UNDEFINED){};

		  MsgHangupCallReq(int handle):
		  MsgRequest(MSG_HANGUP_CALL_REQ, NAME(MSG_HANGUP_CALL_REQ)),
			  stack_call_handle(handle){};

		  unsigned long stack_call_handle;
	};

	enum OfferType
	{
		OFFER_TYPE_INITIAL_OFFER,
		OFFER_TYPE_KEEP_ALIVE,
		OFFER_TYPE_HOLD,
		OFFER_TYPE_RESUME
	};

	class IW_TELEPHONY_API  MsgCallOfferedMixin 
	{
	public:

		MsgCallOfferedMixin():invite_type(OFFER_TYPE_INITIAL_OFFER),is_indialog(FALSE){};

		OfferType invite_type;

		BOOL is_indialog;

		virtual void copy_data_on_response(IN IwMessage *request)
		{
			MsgCallOfferedMixin *req = dynamic_cast<MsgCallOfferedMixin*>(request);
			is_indialog = req->is_indialog;
			invite_type = req->invite_type;
		}
	};

	class IW_TELEPHONY_API MsgCallOfferedReq:
		public MsgVoipCallMixin, public MsgRequest, public MsgCallOfferedMixin
	{
	public:
		MsgCallOfferedReq():
		  MsgRequest(MSG_CALL_OFFERED, 
			  NAME(MSG_CALL_OFFERED)){}

		  LpHandlePair call_handler_inbound;

    };


	class IW_TELEPHONY_API MsgCalOfferedAck:
		public MsgVoipCallMixin, public IwMessage, public MsgCallOfferedMixin
	{
	public:
		MsgCalOfferedAck():IwMessage(MSG_CALL_OFFERED_ACK, 
			NAME(MSG_CALL_OFFERED_ACK)){}

		virtual void copy_data_on_response(IN IwMessage *request)
		{
			MsgCallOfferedMixin::copy_data_on_response(request);
			MsgVoipCallMixin::copy_data_on_response(request);
			IwMessage::copy_data_on_response(request);
		}
	};


	class IW_TELEPHONY_API MsgCallOfferedNack:
		public MsgVoipCallMixin, public IwMessage, public MsgCallOfferedMixin
	{
	public:
		MsgCallOfferedNack():
		  IwMessage(MSG_CALL_OFFERED_NACK, NAME(MSG_CALL_OFFERED_NACK))
			  ,code(API_FAILURE){}

		  virtual void copy_data_on_response(IN IwMessage *request)
		  {
			  MsgCallOfferedMixin::copy_data_on_response(request);
			  MsgVoipCallMixin::copy_data_on_response(request);
			  IwMessage::copy_data_on_response(request);
		  }

		  ApiErrorCode code;

	};


	class IW_TELEPHONY_API MsgNewCallConnected:
		public MsgVoipCallMixin, public IwMessage, public MsgCallOfferedMixin
	{
	public:
		MsgNewCallConnected():IwMessage(MSG_CALL_CONNECTED, 
			NAME(MSG_CALL_CONNECTED)){}

		virtual void copy_data_on_response(IN IwMessage *request)
		{
			MsgCallOfferedMixin::copy_data_on_response(request);
			MsgVoipCallMixin::copy_data_on_response(request);
			IwMessage::copy_data_on_response(request);
		}

	};

	class IW_TELEPHONY_API MsgCallHangupEvt:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallHangupEvt():IwMessage(MSG_CALL_HANG_UP_EVT, 
			NAME(MSG_CALL_HANG_UP_EVT)){}

	};

	class IW_TELEPHONY_API MsgCallDtmfEvt:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallDtmfEvt():IwMessage(MSG_CALL_DTMF_EVT, 
			NAME(MSG_CALL_DTMF_EVT)){}

		string signal;

	};


	class IW_TELEPHONY_API MsgCallBlindXferReq:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallBlindXferReq():IwMessage(MSG_CALL_BLIND_XFER_REQ, 
			NAME(MSG_CALL_BLIND_XFER_REQ)){}

		string destination_uri;

	};

	class IW_TELEPHONY_API MsgCallBlindXferAck:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallBlindXferAck():IwMessage(MSG_CALL_BLIND_XFER_ACK, 
			NAME(MSG_CALL_BLIND_XFER_ACK)){}

	};

	class IW_TELEPHONY_API MsgCallBlindXferNack:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallBlindXferNack():IwMessage(MSG_CALL_BLIND_XFER_NACK, 
			NAME(MSG_CALL_BLIND_XFER_NACK)){}

	};

	class IW_TELEPHONY_API MsgCallSubscribeReq:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallSubscribeReq():IwMessage(MSG_CALL_SUBSCRIBE_REQ, 
			NAME(MSG_CALL_SUBSCRIBE_REQ)),once(TRUE){}

		LpHandlePtr listener_handle;

		BOOL once;

	};

	class IW_TELEPHONY_API MsgCallSubscribeAck:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallSubscribeAck():IwMessage(MSG_CALL_SUBSCRIBE_ACK, 
			NAME(MSG_CALL_SUBSCRIBE_ACK)){}

	};

	class IW_TELEPHONY_API MsgCallSubscribeNack:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallSubscribeNack():IwMessage(MSG_CALL_SUBSCRIBE_NACK, 
			NAME(MSG_CALL_SUBSCRIBE_NACK)){}

	};


	class IW_TELEPHONY_API MsgCallReofferReq:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallReofferReq():IwMessage(MSG_CALL_REOFFER_REQ, 
			NAME(MSG_CALL_REOFFER_REQ)){}

	};

	class IW_TELEPHONY_API MsgCallReofferAck:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallReofferAck():IwMessage(MSG_CALL_REOFFER_ACK, 
			NAME(MSG_CALL_REOFFER_ACK)){}

	};

	class IW_TELEPHONY_API MsgCallReofferNack:
		public MsgVoipCallMixin,public IwMessage
	{
	public:
		MsgCallReofferNack():IwMessage(MSG_CALL_REOFFER_NACK, 
			NAME(MSG_CALL_REOFFER_NACK)){}

	};

#define CALL_RESET_STATE(X) ResetState(X,#X)

	enum CallState
	{
		CALL_STATE_UNKNOWN,
		CALL_STATE_REQUESTED_OFFER,
		CALL_STATE_OFFERING,
		CALL_STATE_REMOTE_OFFERED,
		CALL_STATE_INITIAL_OFFERED,
		CALL_STATE_IN_DIALOG_OFFERED,
		CALL_STATE_CONNECTED,
		CALL_STATE_HELD,
		CALL_STATE_TERMINATED
	};

	typedef int
	IwStackHandle;

	IW_TELEPHONY_API IwStackHandle GenerateCallHandle();

	IW_TELEPHONY_API ApiErrorCode 
		SubscribeToIncomingCalls(
		IN LpHandlePtr stackIncomingHandle, 
		IN LpHandlePtr listenerHandle);


	class IW_TELEPHONY_API GenericOfferAnswerSession :
		public ActiveObject
	{
	public:

		GenericOfferAnswerSession(IN ScopedForking &forking);
			
		GenericOfferAnswerSession(IN ScopedForking &forking, 
			IN HandleId stack_handle_id);

		GenericOfferAnswerSession(IN ScopedForking &forking,
			IN HandleId stack_handle_id,
			IN shared_ptr<MsgCallOfferedReq> offered_msg);

		virtual ~GenericOfferAnswerSession(void);

		virtual void Accept(IN const string& service, csp::Time timeout);

		virtual void CleanDtmfBuffer();

		virtual ApiErrorCode WaitForDtmf(
			OUT string &signal, 
			IN const Time timeout);

		virtual ApiErrorCode RejectCall();

		virtual void UponCallTerminated(
			IN IwMessagePtr ptr);

		virtual void UponActiveObjectEvent(
			IN IwMessagePtr ptr);

		virtual ApiErrorCode HangupCall();

		virtual ApiErrorCode BlindXfer(
			IN const string &destination_uri);

		virtual void WaitTillHangup();

		virtual const string& Dnis();

		virtual const string& Ani();

		virtual AbstractOffer LocalOffer();

		virtual AbstractOffer RemoteOffer();

		virtual ApiErrorCode MakeCall(
			IN const string			&destinationUri, 
			IN const AbstractOffer  &localOffer,
			IN const Credentials	&credentials, 
			IN OUT  MapOfAny		&keyValueMap,
			IN csp::Time			ringTimeout);

		virtual ApiErrorCode ReOffer(
			IN const AbstractOffer  &localOffer,
			IN OUT  MapOfAny		&keyValueMap,
			IN csp::Time			ringTimeout);

		virtual ApiErrorCode Answer(
			IN const AbstractOffer  &localOffer,
			IN const MapOfAny &keyValueMap,
			IN csp::Time	  ringTimeout);

		virtual void UponDtmfEvt(
			IN IwMessagePtr ptr);

		virtual string DtmfBuffer();

		virtual int StackCallHandle() const;

	protected:

		void ResetState(CallState state, const char *stateName);

		void Init(IN ScopedForking &forking,
			IN HandleId stack_handle_id,
			IN shared_ptr<MsgCallOfferedReq> offered_msg);

		HandleId _serviceHandleId;

		LpHandlePair _handlerPair;

		int _iwCallHandle;

		AbstractOffer _localOffer;

		AbstractOffer _remoteOffer;

		CallState _callState;

		string _dnis;

		string _ani;

		BOOL _hangupDetected;

		LpHandlePtr _dtmfChannel;

		LpHandlePtr _hangupChannel;

		shared_ptr<MsgMakeCallOk> _answerMsg;

		BOOL _uac;

		stringstream _dtmfBuffer;

		typedef map<string,LpHandlePtr> 
		AcceptingHandlesMap;

	};
}


