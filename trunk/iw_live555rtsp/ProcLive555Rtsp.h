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

namespace ivrworx
{
	struct RtspCtx
	{

	};

	typedef shared_ptr<RtspCtx> 
	RtspCtxPtr;

	typedef int
	RtspHandle;

	class ProcLive555Rtsp :
		public LightweightProcess
	{
	public:

		ProcLive555Rtsp(ConfigurationPtr conf,LpHandlePair pair);

		void real_run();

		void SetupSession(IwMessagePtr msg);

		void Play(IwMessagePtr msg);

		void Pause(IwMessagePtr msg);

		void Teardown(IwMessagePtr msg);

		void Teardown();

		virtual ~ProcLive555Rtsp(void);

	private:

		ConfigurationPtr _conf;

		TaskScheduler *_scheduler;

		BasicUsageEnvironment *_env;

		RTSPClient *_rtspClient;

		MediaSession *_session;

		RtspHandle _rtspHandle;

	};


}

