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
#include "ProcAsyncLive555RtspClient.h"

namespace ivrworx
{

	ProcAsyncLive555RtspClient::ProcAsyncLive555RtspClient
		(ConfigurationPtr conf,LpHandlePair pair):
	LightweightProcess(pair,"ProcLive555Rtsp"),
	_conf(conf)
	{

	}

	void
	ProcAsyncLive555RtspClient::real_run()
	{
		FUNCTRACKER;

		//
		// Additional initialization routines go here.
		//

		

		I_AM_READY;

		BOOL shutdown_flag = FALSE;
		while(shutdown_flag == FALSE)
		{

			ApiErrorCode res = API_SUCCESS;
			IwMessagePtr msg = _inbound->Wait(Seconds(180), res);

			if (res == API_TIMEOUT)
			{
				LogDebug("ProcLive555Rtsp::real_run - Keep Alive rtsph:")
					continue;
			}

			switch (msg->message_id)
			{
			case MSG_PROC_SHUTDOWN_REQ:
				{
					shutdown_flag = TRUE;
					SendResponse(msg,new MsgShutdownAck());
					break;
				}
			default:
				{
					BOOL res = HandleOOBMessage(msg);
					if (res == FALSE)
					{
						LogWarn("ProcLive555Rtsp::real_run received unknown message id:" << msg->message_id_str);
					}
				} // default
			} // switch
		} // while


	}


	ProcAsyncLive555RtspClient::~ProcAsyncLive555RtspClient(void)
	{
	}
}
