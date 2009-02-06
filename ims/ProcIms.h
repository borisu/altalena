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
#include "LightweightProcess.h"
#include "StreamingObject.h"

using namespace std;


struct StreamingCtx
{
	StreamingCtx(StreamingObject *streaming_object, IxMsgPtr orig_req);

	StreamingCtx(const StreamingCtx &other);
	
	StreamingObject *streaming_object;

	IxMsgPtr orig_req;
};

typedef
map<ImsHandleId, StreamingCtx> StreamingCtxsMap;

class ProcIms :
	public LightweightProcess
{
public:
	ProcIms(LpHandlePair pair, CnxInfo local_media);

	void real_run();

	virtual ~ProcIms(void);

	virtual void AllocatePlaybackSession(IxMsgPtr msg);

	virtual void StartPlayback(IxMsgPtr msg);

	virtual void StopPlayback(IxMsgPtr msg, ScopedForking &forking);

	virtual void UponPlaybackStopped(IxMsgPtr msg);

	virtual void FreeResources();

private:

	CnxInfo _localMedia;

	LpHandlePtr _streamerInbound;

	StreamingCtxsMap _streamingObjectSet;

	PortManager _portManager;
};

class ProcStreamingObjectRemover :
	public LightweightProcess 
{
public:

	ProcStreamingObjectRemover(
		IN LpHandlePair pair,
		IN LpHandlePtr stream_handle,
		IN ImsHandleId handle,
		IN PortManager &ports_map);

	~ProcStreamingObjectRemover();

	void virtual real_run();

private:

	LpHandlePtr _streamerInbound;

	PortManager &_portsManager;

	int _streamHandle;

};
