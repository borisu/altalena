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

#include "StdAfx.h"
#include "ProcLive555Rtsp.h"
#include "Live555RtspSession.h"

namespace ivrworx
{
	Live555RtspSession::Live555RtspSession(ScopedForking &forking, ConfigurationPtr conf):
	RtspSession(forking,conf,IW_UNDEFINED)
	{
		

		
	}

	Live555RtspSession::~Live555RtspSession(void)
	{
		FUNCTRACKER;

		if (_rtspProcPair.inbound)
		{
			GetCurrRunningContext()->Shutdown(Seconds(5),_rtspProcPair);
		}
	}

	

	ApiErrorCode
	Live555RtspSession::Init()
	{
		FUNCTRACKER;

		if (_rtspProcPair.inbound)
		{
			return API_SUCCESS;
		}

		// due to limitation of live555 (synchronous) 
		// we currently open thread per session, sad but true
		DECLARE_NAMED_HANDLE_PAIR(rtsp_proc_pair);
		_rtspProcPair = rtsp_proc_pair;
		_rtspServiceHandleId = rtsp_proc_pair.inbound->GetObjectUid();

		_forking.fork(new ProcLive555Rtsp(_conf,rtsp_proc_pair));

		ApiErrorCode res = GetCurrRunningContext()->WaitTillReady(Seconds(5),_rtspProcPair);


		return res;

	}


}
