#pragma once
#include "MrcpSession.h"

namespace ivrworx
{
	enum MrcpSessionState
	{
		MRCP_INITIAL,
		MRCP_CONNECTING,
		MRCP_ALLOCATED
	};


	struct MrcpSessionCtx
	{
		MrcpSessionCtx();

		virtual ~MrcpSessionCtx();

		MrcpSessionState state;

		// used to send unsolicited events
		//
		//
		LpHandlePair session_handler;

		// used to send solicited responses
		//  
		IwMessagePtr last_user_request;

		MrcpHandle mrcp_handle;

		mrcp_session_t *session;

		mrcp_channel_t *channel;

		mrcp_message_t *last_message;

	};

	typedef 
	shared_ptr<MrcpSessionCtx> MrcpSessionCtxPtr;

	typedef	
	map<MrcpHandle, MrcpSessionCtxPtr> MrcpCtxMap;

	struct MrcpOverlapped:
		public OVERLAPPED
	{
		MrcpHandle mrcp_handle_id;

		mrcp_sig_status_code_e status;

		mrcp_channel_t *channel;

		mrcp_session_t *session;

		mrcp_message_t *message;
	};

	class ProcMrcp : 
		public LightweightProcess
	{
	public:

		ProcMrcp(IN LpHandlePair pair, IN Configuration &conf);

		virtual ~ProcMrcp(void);

		void real_run();	

		
		// client events
		virtual void UponMrcpAllocateSessionReq(
			IN IwMessagePtr msg);

		virtual void UponSpeakReq(
			IN IwMessagePtr msg);

		virtual void UponStopSpeakReq(
			IN IwMessagePtr msg);

		virtual void UponTearDownReq(
			IN IwMessagePtr msg);

		// mrcp server events
		virtual void onMrcpChanndelAddEvt(
			IN MrcpOverlapped *olap);

		virtual void onMrcpMessageReceived(
			IN MrcpOverlapped *olap);

		virtual void onMrcpSessionTerminatedRsp(
			IN MrcpOverlapped *olap);

		virtual void onMrcpSessionTerminatedEvt(
			IN MrcpOverlapped *olap);

	private:
		
		ApiErrorCode Init();

		void Destroy();

		void FinalizeSessionContext(MrcpSessionCtxPtr ctx);

		Configuration &_conf;

		IocpInterruptorPtr _iocpPtr;

		BOOL _logInititiated;

		mrcp_application_t *_application;

		apr_pool_t *_pool;

		mrcp_client_t *_mrcpClient;
	
		MrcpCtxMap _mrcpCtxMap;
	};


}
