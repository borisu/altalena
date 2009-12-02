#pragma once

namespace ivrworx
{
	struct RtspCtx
	{

	};

	typedef shared_ptr<RtspCtx> 
	RtspCtxPtr;

	typedef int
	RtspHandle;

	class ProcRtsp :
		public LightweightProcess
	{
	public:

		ProcRtsp(Configuration &conf,LpHandlePair pair);

		void real_run();

		void SetupSession(IwMessagePtr msg);

		void Play(IwMessagePtr msg);

		void Pause(IwMessagePtr msg);

		void Teardown(IwMessagePtr msg);

		void Teardown();

		virtual ~ProcRtsp(void);

	private:

		Configuration &_conf;

		TaskScheduler *_scheduler;

		BasicUsageEnvironment *_env;

		RTSPClient *_rtspClient;

		MediaSession *_session;

		string _rtspHost;

		int _rtspPort;

		string _controlPath;

		RtspHandle _rtspHandle;

	};


}

