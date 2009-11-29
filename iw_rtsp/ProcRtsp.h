#pragma once

namespace ivrworx
{
	struct RtspCtx
	{

	};

	typedef shared_ptr<RtspCtx> 
	RtspCtxPtr;


	class ProcRtsp :
		public LightweightProcess
	{
	public:

		ProcRtsp(Configuration &conf,LpHandlePair pair);

		void real_run();

		void AllocateSession(IwMessagePtr msg);

		void Play(IwMessagePtr msg);

		void Teardown(IwMessagePtr msg);

		virtual ~ProcRtsp(void);

	private:

		Configuration &_conf;

		TaskScheduler *_scheduler;

		BasicUsageEnvironment *_env;

	};


}

