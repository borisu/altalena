#pragma once
#include "MrcpSession.h"

namespace ivrworx
{
	enum MrcpSessionState
	{
		MRCP_INITIAL,
		MRCP_CONNECTING,
		MRCP_ALLOCATED,
		MRCP_SPEAKING,
		MRCP_STOPPED,
		MRCP_TORNDOWN
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
	};

	class ProcMrcp : 
		public LightweightProcess
	{
	public:

		ProcMrcp(LpHandlePair pair, Configuration &vonf);

		virtual ~ProcMrcp(void);

		void real_run();	

		virtual void UponAllocateSessionReq(IwMessagePtr msg);

		virtual void UponSpeakReq(IwMessagePtr msg);

		virtual void UponTearDownReq(IwMessagePtr msg);

		virtual void UponChnannelConnected( MrcpOverlapped* mrcpOlap);

	private:
		
		ApiErrorCode Init();

		void Destroy();

		void TearDownSession(MrcpSessionCtxPtr ctx);

		Configuration &_conf;

		IocpInterruptorPtr _iocpPtr;

		BOOL _logInititiated;

		mrcp_application_t *_application;

		apr_pool_t *_pool;

		mrcp_client_t *_client;
	
		MrcpCtxMap _mrcpCtxMap;
	};


}
