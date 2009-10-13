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
#include "ProcRtpProxy.h"


namespace ivrworx
{

struct ThreadParams
{
	ThreadParams():
	env(NULL),stop(NULL),thread_terminated_evt(NULL){};

	BasicUsageEnvironment *env;

	// used to stop the loop
	char *stop;

	// signaled when worker thread is over
	HANDLE thread_terminated_evt;
};

ProcRtpProxy::RtpConnection::RtpConnection()
:connection_id(NULL),
live_socket((::Groupsock *)NULL)
{

}

ProcRtpProxy::RtpConnection::RtpConnection(const RtpConnection &conn)
{
	connection_id	= conn.connection_id;
	live_socket		= conn.live_socket;
}


ProcRtpProxy::RtpConnection::~RtpConnection()
{

}



DWORD WINAPI ScheduleThread(LPVOID lpParameter)
{
	
	ThreadParams *params = (ThreadParams *)lpParameter;

	params->env->taskScheduler().doEventLoop(params->stop);

	::SetEvent(params->thread_terminated_evt);

	delete params;

	return 0;
}

ProcRtpProxy::ProcRtpProxy(LpHandlePair pair, Configuration &conf):
LightweightProcess(pair),
_conf(conf),
_env(NULL),
_scheduler(NULL)
{

	
}

ApiErrorCode 
ProcRtpProxy::InitSockets()
{
	FUNCTRACKER;

	int base_port	 = _conf.GetInt("rtp_proxy_base_port");
	int top_port	 = _conf.GetInt("rtp_proxy_top_port");
	int num_of_conns = _conf.GetInt("rtp_proxy_num_of_connections");

	if (base_port < 0 || top_port < base_port || num_of_conns < 1 )
	{
		LogCrit("Error in ports configuration");
		return API_FAILURE;
	}

	//
	// Init Connections
	//
	int counter = 0;
	for (int i = 0; i < num_of_conns; i++) 
	{
		if ( i > top_port )
		{
			break;
		}

		char const* inputAddressStr = 
			_conf.GetString("rtp_proxy_ip").c_str();

		struct in_addr inputAddress;
		inputAddress.s_addr = our_inet_addr(inputAddressStr);

		int curr_port = base_port + i;
		Port const inputPort(curr_port);

		unsigned char const inputTTL = 0; 
		Groupsock * socket = 
			new Groupsock(*_env, inputAddress, inputPort, inputTTL);

		RtpConnection *conn = new RtpConnection();
		conn->connection_id = counter++;
		conn->live_socket = GroupSockPtr(socket);

		_map[conn->connection_id] = RtpConnectionPtr(conn);

		LogDebug("Created source connection id:" << i << " port:" << curr_port);

	};

	if (counter == 0)
	{
		LogCrit("Cannot open connections");
		return API_FAILURE;
	};

	return API_SUCCESS;


}


void 
ProcRtpProxy::real_run()
{
	FUNCTRACKER;

	
	_scheduler	=	BasicTaskScheduler::createNew();
	_env  =	BasicUsageEnvironment::createNew(*_scheduler);
	
	

	if (IW_FAILURE(InitSockets()))
	{
		return;
	}
	
	
	// will be deleted in thread
	ThreadParams *params 
		= new ThreadParams();

	char stop = NULL;

	params->env = _env;
	params->stop = &stop;

	HANDLE thread_terminated_evt = 
		::CreateEvent(NULL,TRUE,FALSE,NULL);

	if (thread_terminated_evt == NULL)
	{
		LogSysError("::CreateEvent");
		return;
	}

	params->thread_terminated_evt = thread_terminated_evt;

	
	DWORD thread_id = 0;
	HANDLE thread_handle = ::CreateThread( 
		NULL,                   // default security attributes
		0,                      // use default stack size  
		ScheduleThread,         // thread function name
		params,				    // argument to thread function 
		0,                      // use default creation flags 
		&thread_id);			// returns the thread identifier 

	if (thread_handle == NULL)
	{
		LogSysError("::CreateThread");
		goto exit;
	}

	I_AM_READY;

	BOOL shutdown_flag = FALSE;
	while (shutdown_flag == FALSE)
	{

		ApiErrorCode res = API_SUCCESS;
		IwMessagePtr msg = GetInboundMessage(Seconds(60),res);
		if (res == API_TIMEOUT)
		{
			LogInfo("Rtp Relay Keep Alive");
			continue;
		}

		switch (msg->message_id)
		{
		case MSG_RTP_PROXY_ALLOCATE_REQ:
			{ 
				UponAllocateReq(msg);
				break ;
			}; 
		case MSG_RTP_PROXY_BRIDGE_REQ:
			{ 
				UponBridgeReq(msg);
				break ;
			}; 
		case MSG_RTP_PROXY_MODIFY_REQ:
			{ 
				UponModifyReq(msg);
				break; 
			}; 
		case MSG_RTP_PROXY_DEALLOCATE_REQ:
			{ 
				UponDeallocateReq(msg);
				break; 
			}; 
		default:
			{
				if (HandleOOBMessage(msg) == FALSE)
				{
					LogWarn("Received unknown message " << msg->message_id_str);
				}
			}
		}
	}

exit:
	if (thread_handle)
	{
		stop = 'S';
		DWORD res = ::WaitForSingleObject(thread_handle,INFINITE);
		if (res != WAIT_OBJECT_0)
		{
			LogSysError("::WaitForSingleObject");
			return;
		}

	}
	
	if (thread_terminated_evt)	::CloseHandle(thread_terminated_evt);
	if (_env) _env->reclaim();
	if (_scheduler) delete _scheduler;

}

void 
ProcRtpProxy::UponAllocateReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyAllocateReq> req = 
		dynamic_pointer_cast<MsgRtpProxyAllocateReq>(msg);
}

void 
ProcRtpProxy::UponDeallocateReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyDeallocateReq> req = 
		dynamic_pointer_cast<MsgRtpProxyDeallocateReq>(msg);
}

void 
ProcRtpProxy::UponModifyReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyModifyReq> req = 
		dynamic_pointer_cast<MsgRtpProxyModifyReq>(msg);
}


void 
ProcRtpProxy::UponBridgeReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyBridgeReq> req = 
		dynamic_pointer_cast<MsgRtpProxyBridgeReq>(msg);
}

ProcRtpProxy::~ProcRtpProxy(void)
{
}

}
