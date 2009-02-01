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
#include "ProcPipeIPCDispatcher.h"
#include "LocalProcessRegistrar.h"
#include "CcuLogger.h"



#define CCU_EVENT_HANDLE 0

ProcPipeIPCDispatcher::ProcPipeIPCDispatcher( 
	LpHandlePair pair,
	CcuProcId qId):
LightweightProcess(pair,__FUNCTIONW__),
_qId(qId),
_shutdown_flag(FALSE),
_intPtr(new SemaphoreInterruptor())
{
	FUNCTRACKER;

	_pipesDict[RTP_RELAY_Q] = L"\\\\.\\pipe\\RTP";
	_pipesDict[VCS_Q] = L"\\\\.\\pipe\\VCS";
	_pipesDict[IMS_Q] = L"\\\\.\\pipe\\IMS";
	_pipesDict[AIS_Q] = L"\\\\.\\pipe\\AIS";

	pair.inbound->HandleInterruptor(dynamic_pointer_cast<Interruptor>(_intPtr));
	
	init();
	 
}

ProcPipeIPCDispatcher::ProcPipeIPCDispatcher( 
	LpHandlePair pair,
	wstring pipe_name):
LightweightProcess(pair),
_qId(0),
_shutdown_flag(FALSE),
_intPtr(new SemaphoreInterruptor())
{
	FUNCTRACKER;

	pair.inbound->HandleInterruptor(dynamic_pointer_cast<Interruptor>(_intPtr));

	init();

	_pipeName = pipe_name;

}

void
ProcPipeIPCDispatcher::init()
{
	FUNCTRACKER;
	
	// Zero the memory
	::ZeroMemory(
		_hEvents,
		CCU_IPC_NUM_OF_EVENTS * sizeof(_hEvents[0]));

	// Zero the memory
	::ZeroMemory(
		_pipeStructs,
		CCU_MAX_IPC_CONNECTIONS * sizeof(_pipeStructs[0]));


	
}

ProcPipeIPCDispatcher::~ProcPipeIPCDispatcher(void)
{
	FUNCTRACKER;

	if (_hEvents[CCU_EVENT_HANDLE] != NULL)
	{
		::CloseHandle(_hEvents[CCU_EVENT_HANDLE]);
	}

	for (int i = 0; i < CCU_MAX_IPC_CONNECTIONS; i++)
	{
		
		PIPEINST &pipe = _pipeStructs[i];
		if (pipe.hPipeInst != NULL)
		{
			::CloseHandle(pipe.hPipeInst);
		}

		if (pipe.oReadOverlap.hEvent != NULL)
		{
			::CloseHandle(pipe.oReadOverlap.hEvent);
		}

		if (pipe.oConnectOverlap.hEvent != NULL)
		{
			::CloseHandle(pipe.oConnectOverlap.hEvent);
		}

		::ZeroMemory(
			&pipe,
			sizeof(pipe));
	}
}

void
ProcPipeIPCDispatcher::real_run()
{
	FUNCTRACKER;
	
	// bind to private pipe
	if (_pipeName.empty())
	{
		PipeDictionary::iterator pdIter = _pipesDict.find(_qId);
		if (pdIter == _pipesDict.end())
		{
			LogWarn("Cannot map queue id=[" << _qId << "] to pipe URI.");
			_outbound->Send(new CcuMsgProcFailure());
			return;
		}

		_pipeName = _pipesDict[_qId];
	}

	// create pipes
	for (int i = 0; i < CCU_MAX_IPC_CONNECTIONS; i++) 
	{
		
		PIPEINST &pipe = _pipeStructs[i];

		pipe.hPipeInst = ::CreateNamedPipe( 
			_pipeName .c_str(),       // pipe name 
			PIPE_ACCESS_INBOUND |     // read/write access 
			FILE_FLAG_OVERLAPPED,     // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFSIZE,                  // output buffer size 
			BUFSIZE,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 

		
		if (pipe.hPipeInst == INVALID_HANDLE_VALUE)
		{
			LogSysError(L"Cannot ::CreateNamedPipe.");
			_outbound->Send(new CcuMsgProcFailure());
			return;
		}

		LogTrace("Server pipe instance created on HANDLE=[0x" << hex << pipe.hPipeInst << "]");
	}
	

	// Create pipe instances and issue asynchronous connect request
	_hEvents[CCU_EVENT_HANDLE] = _intPtr->Handle();
	for (int i = CCU_EVENT_HANDLE + 1; i < CCU_IPC_NUM_OF_EVENTS; i++) 
	{ 

		//find corresponding pipe index
		int index = (i-1)/2;
		PIPEINST &pipe = _pipeStructs[index];
		

		HANDLE &hEvent  = _hEvents[i];
		hEvent = ::CreateEvent( 
			NULL,    // default security attribute 
			TRUE,	 // manual-reset event 
			FALSE,   // initial state = not signaled 
			NULL);   // unnamed event object 

		if (hEvent == NULL)
		{
			LogSysError(L"Cannot ::CreateEvent");
			_outbound->Send(new CcuMsgProcFailure());
			return;
		}

		switch (i%2)
		{
			case 0:
				{
					LogTrace("Connect event of pipe=[" << index << "] in slot=[" << i << "]");
					pipe.oConnectOverlap.hEvent = hEvent;
					break;
				}
			case 1:
				{
					LogTrace("Read    event of pipe=[" << index << "] in slot=[" << i << "]");
					pipe.oReadOverlap.hEvent = hEvent;
					break;
				}
			default:
				{
					throw;
				}
		}

	}

	// Call the subroutine to connect to the new client
	for (int i = 0; i < CCU_MAX_IPC_CONNECTIONS; i++) 
	{
		AsyncPipeConnect(_pipeStructs[i]); 
	}


	LogInfo("Successfully started overlapped comm on pipe=[" << _pipeName <<"], listening...");

	I_AM_READY;

	while (_shutdown_flag == FALSE)
	{
		
		DWORD res = ::WaitForMultipleObjects(
			CCU_IPC_NUM_OF_EVENTS, 
			_hEvents,
			FALSE,
			INFINITE
			);



		LogDebug("::WaitForMultipleObjects res=[" << res << "]");

		int signaledEventIndex = res - WAIT_OBJECT_0;  // determines which pipe 
		if (signaledEventIndex < 0 || 
			signaledEventIndex > (CCU_IPC_NUM_OF_EVENTS - 1)) 
		{
			LogSysError(" ::WaitForMultipleObjects failed"); 
			throw;
		}

		
		LogDebug("Signaled event slot=[" << signaledEventIndex << "]");
		switch (signaledEventIndex)
		{
		case WAIT_OBJECT_0 + 0 :
			{
				HandleLocalMsg();
				break;
			}
		default:
			{
				int index = (signaledEventIndex - 1)/2;
				PIPEINST &pipe = _pipeStructs[index];

				
				switch (signaledEventIndex%2)
				{
				case 0:
					{
						LogDebug("Connect activity on server pipe=[" << _pipeName << "] instance=[" << index << "]");
						HandleConnectCompletion(pipe);
						break;
					}
				case 1:
					{
						LogDebug("Read    activity on server pipe=[" << _pipeName << "] instance=[" << index << "]");
						HandleReadCompletion(pipe);
						break;
					}
				default:
					{
						throw;
					}

				} // switch
			} // switch
		} //while
	}
}

void
ProcPipeIPCDispatcher::HandleConnectCompletion(PIPEINST &pipe) 
{
	
	DWORD cbRet = 0;
	BOOL fSuccess = ::GetOverlappedResult( 
		pipe.hPipeInst,			// handle to pipe 
		&pipe.oConnectOverlap,	// OVERLAPPED structure 
		&cbRet,					// bytes transferred 
		FALSE);					// do not wait 

	if (fSuccess == FALSE && 
		::GetLastError()!= ERROR_IO_PENDING) 
	{
		LogSysError("Error in ::GetOverlappedResult"); 
		throw;
	}

	pipe.dwState = CCU_PIPE_READ_STATE; 
	::ResetEvent(pipe.oConnectOverlap.hEvent);

	// start reading
	this->AsyncPipeRead(pipe);

}


void
ProcPipeIPCDispatcher::HandleReadCompletion(PIPEINST &pipe) 
{

	DWORD cbRet = 0;
	BOOL res = ::GetOverlappedResult( 
		pipe.hPipeInst,		// handle to pipe 
		&pipe.oReadOverlap,	// OVERLAPPED structure 
		&cbRet,				// bytes transferred 
		FALSE);				// do not wait 

	if (res == FALSE || 
		cbRet == 0) 
	{
		LogSysError("::GetOverlappedResult");
		this->DisconnectAndReconnect(pipe); 
		this->AsyncPipeRead(pipe);
		return;
	}

	
	LogDebug("Pipe RCV msg=[" <<pipe.chRequest << "]");


	// un-marshaling
	//
	std::wistringstream iss(
		pipe.chRequest, 
		std::ios::binary);
	 	 
	CcuMessage *ccu_msg = NULL;
	boost::archive::text_wiarchive ia(iss);
	ia >> ccu_msg;

	
	// responses should be preferably
	// sent via the same interface 
	//
	ccu_msg->preferrable_ipc_interface = this->ProcessId();

	this->AsyncPipeRead(pipe);	
	 
	 
	LpHandlePtr procHandle = LocalProcessRegistrar::Instance().GetHandle(ccu_msg->dest.proc_id);
	if (procHandle == CCU_NULL_LP_HANDLE)
	{
		LogWarn(Name() << " Received message to non-existent process qid=[" << ccu_msg->dest.proc_id<<"]");
	 	return;
	}
	 
	procHandle->Send(ccu_msg);

}

void
ProcPipeIPCDispatcher::AsyncPipeRead(PIPEINST &pipe)
{
	// issue async read operation from the pipe. 
	::ZeroMemory(pipe.chRequest,BUFSIZE * sizeof(pipe.chRequest[0]));
	BOOL res = ::ReadFile( 
		pipe.hPipeInst,			// pipe handle 
		pipe.chRequest,			// buffer to receive reply 
		BUFSIZE*sizeof(TCHAR),  // size of buffer 
		&pipe.cbRead,			// number of bytes read 
		&pipe.oReadOverlap);	// not overlapped 

	DWORD dwErr = ::GetLastError(); 
	if (res == TRUE || 
		dwErr == ERROR_IO_PENDING)
	{
		return;
	}
	
	LogSysError("::ReadFile");
	throw;

}

void
ProcPipeIPCDispatcher::AsyncPipeConnect(PIPEINST &pipe) 
{ 
	
	// Start an overlapped connection for this pipe instance. 
	BOOL res = ::ConnectNamedPipe(
		pipe.hPipeInst, 
		&pipe.oConnectOverlap); 

	// Overlapped ConnectNamedPipe should return zero. 
	if (res) 
	{
		LogSysError("ConnectNamedPipe failed"); 
		throw;
	}

	switch (::GetLastError()) 
	{ 
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING: 
		{
			pipe.fPendingIO = TRUE; 
			pipe.dwState = CCU_PIPE_CONNECTING_STATE;
			break; 
		}

		// Client is already connected, so signal an event. 
	case ERROR_PIPE_CONNECTED: 
		{
			if (::SetEvent(pipe.oConnectOverlap.hEvent) == NULL) 
			{
				LogSysError("::SetEvent"); 
				throw;
			}
			LogDebug("Pipe is already in connected state");
			pipe.dwState = CCU_PIPE_READ_STATE;
			break; 
		}
		
		// If an error occurs during the connect operation... 
	default: 
		{
			LogSysError("ConnectNamedPipe failed"); 
			throw;
		}
	} 
}



CcuApiErrorCode 
ProcPipeIPCDispatcher::HandleLocalMsg()
{
	FUNCTRACKER;

	if (!_inbound->InboundPending())
	{
		LogWarn(Name()<< "IPC Signaled without message.");
		return CCU_API_FAILURE;
	}

	CcuApiErrorCode res;
	CcuMsgPtr msg = _inbound->Wait(Seconds(0),res);
	if (CCU_FAILURE(res))
	{
		throw;
	}


	if (msg->message_id == CCU_MSG_PROC_SHUTDOWN_REQ)
	{
		_shutdown_flag = TRUE;

		LogWarn("Received shutdown message. Exiting...");

		// send response and to outbound channel
		SendResponse(msg,new CcuMsgShutdownAck());

		return CCU_API_SUCCESS;

	}

	msg->source.queue_path = _pipeName;
	CcuProcId &proc_id = msg->dest.proc_id;

	
	//
	// Find pipe name for well known process id.
	//
	if (msg->dest.queue_path.empty())
	{
		if (!IsWellKnownPid(proc_id))
		{
			LogWarn("IPC received local message to unknown proc_id=[" << proc_id << "]");
			return CCU_API_FAILURE;
		}
		
		PipeDictionary::iterator iter = _pipesDict.find(proc_id);
		if (iter == _pipesDict.end())
		{
			LogWarn("IPC received local message to unknown global queue=[" << proc_id << "]");
			return CCU_API_FAILURE;
		}

		msg->dest.queue_path = (*iter).second;
	}

	wstring &dest_queue_path = msg->dest.queue_path;

	HANDLE hDestPipe = NULL;

	//
	// Determine pipe handle
	//
	if (_handleCache.find(dest_queue_path) == _handleCache.end())
	{
		//
		// Cache Miss
		//
		while (1) 
		{ 
			hDestPipe = ::CreateFile( 
				dest_queue_path.c_str(),// pipe name 
				GENERIC_WRITE,			// write access 
				0,						// no sharing 
				NULL,					// default security attributes
				OPEN_EXISTING,			// opens existing pipe 
				0,						// default attributes 
				NULL);					// no template file 

			// Break if the pipe handle is valid. 
			if (hDestPipe != INVALID_HANDLE_VALUE) 
				break; 

			// Exit if an error other than ERROR_PIPE_BUSY occurs. 
			if (::GetLastError() != ERROR_PIPE_BUSY) 
			{
				LogWarn("Could not open pipe=[" << dest_queue_path << "]"); 
				return CCU_API_FAILURE;
			}

			// All pipe instances are busy, so wait for 20 seconds. 
			if (::WaitNamedPipe(
				dest_queue_path.c_str(), 
				20000) == NULL) 
			{ 
				LogWarn("Timeout opening pipe=[" << dest_queue_path << "]"); 
				return CCU_API_FAILURE;
			} 

			
		} 

		DWORD dwMode = PIPE_READMODE_MESSAGE; 
		BOOL fSuccess = ::SetNamedPipeHandleState( 
			hDestPipe,		// pipe handle 
			&dwMode,		// new pipe mode 
			NULL,		    // don't set maximum bytes 
			NULL);			// don't set maximum time 

		if (!fSuccess) 
		{
			LogSysError("::SetNamedPipeHandleState failed"); 
			throw;
		}

		_handleCache[dest_queue_path] = hDestPipe;
		LogDebug("Client=[" << _pipeName << "] connected to server=[" << dest_queue_path << "]" );

	} 
	else
	{
		hDestPipe = _handleCache[dest_queue_path];
	}

	//
	// Finally send the message
	// 
	std::wostringstream ostream;
	boost::archive::text_woarchive oa(ostream);
	CcuMessage *msg_ptr = msg.get();
	oa & msg_ptr;

	
	
	DWORD cbWritten = 0;
	// Send a message to the pipe server. 
	BOOL fSuccess = ::WriteFile( 
		hDestPipe,												// pipe handle 
		ostream.str().c_str(),									// message 
		(ULONG)(ostream.str().length() + 1) * sizeof(wchar_t),	// message length 
		&cbWritten,												// bytes written 
		NULL);													// not overlapped 

	if (!fSuccess) 
	{
		LogSysError("::WriteFile failed"); 
		throw;
	}

	LogDebug("Pipe SND buf=[" << ostream.str() << "]");

	return CCU_API_SUCCESS;

}

void
ProcPipeIPCDispatcher::DisconnectAndReconnect(PIPEINST &pipe) 
{ 
	FUNCTRACKER;

	LogWarn("Reconnecting pipe client");

	// Disconnect the pipe instance. 
	if (::DisconnectNamedPipe(pipe.hPipeInst) != NULL ) 
	{
		LogSysError("::DisconnectNamedPipe");
		throw;
	}

	// Call a subroutine to connect to the new client. 
	this->AsyncPipeConnect(pipe); 
} 



