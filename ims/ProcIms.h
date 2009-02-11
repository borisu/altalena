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



#include "Ims.h"

using namespace std;

namespace ivrworx
{

	struct StreamingCtx
	{
		StreamingCtx();

		~StreamingCtx();

		AudioStream *stream;

		IpcAdddress curr_txn_handler;

	};

	typedef 
	shared_ptr<StreamingCtx> StreamingCtxPtr;

	class ProcIms :
		public LightweightProcess
	{

	public:
		ProcIms(LpHandlePair pair, CcuConfiguration &conf);

		void real_run();

		virtual ~ProcIms(void);

	protected:

		virtual void AllocatePlaybackSession(IxMsgPtr msg);

		virtual void StartPlayback(IxMsgPtr msg);

		virtual void StopPlayback(IxMsgPtr msg);

		virtual void TearDown(IxMsgPtr msg);

		virtual void TearDown(StreamingCtxPtr ctx);

		virtual void UponPlaybackStopped(OVERLAPPED* args);

		virtual void FreeResources();

	private:

		void InitCodecs();

		CnxInfo _localMedia;

		PortManager _portManager;

		MSTicker *_ticker;

		RtpProfile *av_profile;

		typedef	map<ImsHandleId, StreamingCtxPtr> StreamingCtxsMap;
		StreamingCtxsMap _streamingObjectSet;

		typedef map<wstring, PayloadType*> PayloadTypeMap;
		PayloadTypeMap _payloadTypeMap;

		CcuConfiguration &_conf;

		IocpInterruptorPtr _iocpPtr;

	};

}

