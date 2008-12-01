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
#include "RTPConnection.h"
#include "PortManager.h"
#include "CcuConfiguration.h"

using namespace std;

typedef map<int,RTPConnection*> RTPConnectionMap;

class ProcAudio : 
	public LightweightProcess
{
public:

	ProcAudio(
		IN LpHandlePair pair, 
		IN CnxInfo data);

	ProcAudio(
		IN LpHandlePair pair, 
		IN CcuConfiguration *conf);

	virtual ~ProcAudio(void);
	
	void real_run();

private:

	void AllocateAudioConnection(IN CcuMsgPtr ptr);

	void CloseAudioConnection(IN CcuMsgPtr ptr, IN ScopedForking &forking);

	void BridgeConnections(IN CcuMsgPtr ptr);

	void ModifyConnection(IN CcuMsgPtr ptr);

	CnxInfo _mediaData;

	RTPConnectionMap _connMap;

	LpHandlePtr _receiverInbound;

	LpHandlePtr _receiverOutbound;

	PortManager _portManager;

	CcuConfiguration *_conf;

	RelayMemoryManager _memManager;
};

class ProcRTPConnectionRemover :
	public LightweightProcess 
{
public:

	ProcRTPConnectionRemover(
		IN LpHandlePair pair,
		IN LpHandlePtr receiver_handle,
		IN RTPConnection *conn,
		IN PortManager &ports_map);

	~ProcRTPConnectionRemover();

	void virtual real_run();

private:

	LpHandlePtr _receiverHandle;

	PortManager &_portsManager;

	RTPConnection *_rtpConn;

	

};
