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
#include "lightweightprocess.h"


using namespace std;

typedef map<wstring,HANDLE>		WritePipeHandleCache;
typedef map<IxProcId,wstring>	PipeDictionary;

#define BUFSIZE 4096
#define CCU_MAX_IPC_CONNECTIONS 10

// *2 (Read/Connect per pipe) + 1 for local messages semaphore
#define CCU_IPC_NUM_OF_EVENTS 2*CCU_MAX_IPC_CONNECTIONS + 1 


enum CcuPipeState
{
	CCU_PIPE_CONNECTING_STATE,
	CCU_PIPE_READ_STATE,
	CCU_PIPE_WRITE_STATE
};

typedef struct 
{ 
	OVERLAPPED oReadOverlap;
	OVERLAPPED oConnectOverlap;
	HANDLE hPipeInst; 
	TCHAR chRequest[BUFSIZE]; 
	DWORD cbRead;
	TCHAR chReply[BUFSIZE];
	DWORD cbToWrite; 
	DWORD dwState; 
	BOOL fPendingIO; 
} PIPEINST, *LPPIPEINST; 


class ProcPipeIPCDispatcher :
	public LightweightProcess
{
public:

	ProcPipeIPCDispatcher(IN LpHandlePair pair, IN IxProcId proc_id);

	ProcPipeIPCDispatcher(IN LpHandlePair pair, IN wstring pipe_name);

	virtual ~ProcPipeIPCDispatcher(void);

	virtual void real_run();

private:

	void init();

	void AsyncPipeConnect(IN PIPEINST &pipe);

	void AsyncPipeRead(IN PIPEINST &pipe);

	IxApiErrorCode HandleLocalMsg();

	void HandleConnectCompletion(IN PIPEINST &pipe);

	void HandleReadCompletion(IN PIPEINST &pipe);

	void DisconnectAndReconnect(IN PIPEINST &pipe);

	HANDLE _hEvents[CCU_IPC_NUM_OF_EVENTS]; 

	PIPEINST _pipeStructs [CCU_MAX_IPC_CONNECTIONS];

	IxProcId _qId;

	wstring _pipeName;

	WritePipeHandleCache _handleCache;

	PipeDictionary _pipesDict;

	BOOL _shutdown_flag;

	SemaphoreInterruptorPtr _intPtr;

	
};


