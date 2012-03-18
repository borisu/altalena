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
	class ProcMaxSobolevRtpProxy :
		public LightweightProcess
	{
	public:

		ProcMaxSobolevRtpProxy(ConfigurationPtr conf,LpHandlePair pair);

		virtual ~ProcMaxSobolevRtpProxy(void);

		virtual void real_run();

	protected:

		virtual void UponAllocateReq(IwMessagePtr msg);

		virtual void UponDeallocateReq(IwMessagePtr msg);

		virtual void UponModifyReq(IwMessagePtr msg);

		virtual void UponBridgeReq(IwMessagePtr msg);

		ApiErrorCode InitSockets();

		ApiErrorCode onSocketOverlappedReceived(const string &response);

	private:

		ConfigurationPtr _conf;

		SOCKET _s;

		int _sessionCounter;

		HANDLE g_hIOCompletionPort;

		IocpInterruptorPtr _iocpPtr;

		enum REQUEST_TYPE
		{
			REQUEST_TYPE_NONE,
			REQUEST_TYPE_UPDATE,
			REQUEST_TYPE_DELETE,
		};

		struct ReqCtx
		{
			ReqCtx();

			IwMessagePtr request;

			REQUEST_TYPE type;
		};

		typedef map<int,ReqCtx>
		RequestsMap;

		RequestsMap _reqMap;

		int _correlationCounter;

	};


}

