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
	static in_addr convert_hname_to_addrin(const char *name)
	{
		hostent *phe = ::gethostbyname(name);
		if (phe == NULL)
		{
			DWORD last_error = ::GetLastError();
			cerr << "::gethostbyname returned error for host:" << name << ", le:" << last_error;
			throw configuration_exception();
		}


		// take only first result
		struct in_addr addr;
		addr.s_addr = *(u_long *) phe->h_addr;

		return addr;

	}

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
state(CONNECTION_STATE_AVAILABLE),
source(NULL),
sink(NULL)
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


	


static void dummyTask(void* clientData) 
{
	BasicUsageEnvironment *env = (BasicUsageEnvironment *)clientData;
	env->taskScheduler().scheduleDelayedTask(100000,
		(TaskFunc*)dummyTask, env);

}

DWORD WINAPI ScheduleThread(LPVOID lpParameter)
{
	
	ThreadParams *params = (ThreadParams *)lpParameter;

	do 
	{
		dummyTask(params->env);
		params->env->taskScheduler().doEventLoop(params->stop);
		 
	} while (*params->stop != 'S');

	

	::SetEvent(params->thread_terminated_evt);

	delete params;

	return 0;
}

ProcRtpProxy::ProcRtpProxy(LpHandlePair pair, Configuration &conf):
LightweightProcess(pair,RTP_PROXY_Q,"ProcRtpProxy"),
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

	string rtp_ip = _conf.GetString("rtp_proxy_ip");

	const char * input_address_str = rtp_ip.c_str();



	  
	_localInAddr = convert_hname_to_addrin(input_address_str);
	
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

		int curr_port = base_port + i;
		Port const inputPort(curr_port);

		unsigned char const inputTTL = 0; 
		Groupsock * socket = 
			new Groupsock(*_env, _localInAddr, inputPort, inputTTL);

		RtpConnection *conn = new RtpConnection();
		conn->connection_id = counter++;
		conn->live_socket = GroupSockPtr(socket);
		conn->local_cnx_ino = CnxInfo(_localInAddr,curr_port);

		_connectionsMap[conn->connection_id] = RtpConnectionPtr(conn);

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
	
	try
	{
		if (IW_FAILURE(InitSockets()))
		{
			LogCrit("Error initiating socket");
			return;
		}
	}
	catch (std::exception &e)
	{
		LogCrit("Exception initiating socket e:" << e.what());
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
				} // if
			}// default

			

		}// switch

	

	}// while

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

	RtpConnectionsMap::iterator iter = _connectionsMap.begin();
	while(iter != _connectionsMap.end())
	{
		UponUnBridgeReq(iter->second);
	};

	
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

	RtpConnectionPtr candidate;

	RtpConnectionsMap::iterator iter = _connectionsMap.begin();
	while(iter != _connectionsMap.end())
	{
		if (iter->second->state == CONNECTION_STATE_AVAILABLE)
		{
			candidate = iter->second;
			break;
		}

		++iter;
	};

	if (!candidate)
	{
		LogWarn("No available rtp resource");
		SendResponse(req, new MsgRtpProxyNack());
		return;
	}


	MsgRtpProxyAck* ack = new MsgRtpProxyAck();
	ack->rtp_proxy_handle = candidate->connection_id;
	ack->local_media = candidate->local_cnx_ino;

	candidate->state = CONNECTION_STATE_ALLOCATED;

	if (req->remote_media.is_ip_valid())
	{
// 		struct in_addr outputAddress;
// 		outputAddress.s_addr = our_inet_addr("10.0.0.1");

		candidate->live_socket->changeDestinationParameters(
			req->remote_media.inaddr(),
			req->remote_media.port_ho(),225);
	}
	
	LogDebug("ProcRtpProxy::UponAllocateReq allocated conn:" << candidate->connection_id << " local:" << candidate->local_cnx_ino << ", remote:" << req->remote_media.ipporttos());
	SendResponse(req, ack);
	
}

void 
ProcRtpProxy::UponDeallocateReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyDeallocateReq> req = 
		dynamic_pointer_cast<MsgRtpProxyDeallocateReq>(msg);

	// Find source.
	RtpConnectionsMap::iterator iter = 
		_connectionsMap.find(req->rtp_proxy_handle);
	if (iter == _connectionsMap.end())
	{

		LogWarn("conn:" << req->rtp_proxy_handle << " not found");
		SendResponse(req, new MsgRtpProxyNack());
		return;

	}

	RtpConnectionPtr conn = iter->second;

	UponUnBridgeReq(conn);
	conn->state = CONNECTION_STATE_AVAILABLE;
}

void 
ProcRtpProxy::UponModifyReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyModifyReq> req = 
		dynamic_pointer_cast<MsgRtpProxyModifyReq>(msg);

	// Find source.
	RtpConnectionsMap::iterator iter = 
		_connectionsMap.find(req->rtp_proxy_handle);
	if (iter == _connectionsMap.end())
	{

		LogWarn("conn:" << req->rtp_proxy_handle << " not found");
		SendResponse(req, new MsgRtpProxyNack());
		return;

	}

	RtpConnectionPtr conn = iter->second;

	conn->live_socket->changeDestinationParameters(req->remote_media.inaddr(),
		req->remote_media.port_ho(),0);

	SendResponse(req, new MsgRtpProxyNack());
	

}


ApiErrorCode
ProcRtpProxy::UponUnBridgeReq(RtpConnectionPtr conn)
{
	FUNCTRACKER;

	
	// Find source.

	RtpConnectionPtr source_conn;
	RtpConnectionPtr dest_conn;

	switch (conn->state)
	{
	case CONNECTION_STATE_AVAILABLE:
	case CONNECTION_STATE_ALLOCATED:
		{
			
			return API_SUCCESS;
			break;
		}
	case CONNECTION_STATE_INPUT:
		{
			source_conn = conn;
			dest_conn = conn->destination_conn;
			break;
		}
	case CONNECTION_STATE_OUTPUT:
		{
			source_conn = conn->source_conn;
			dest_conn = conn;
			break;
		}
	}

	source_conn->source->stopGettingFrames();
	source_conn->state = CONNECTION_STATE_ALLOCATED;
	source_conn->destination_conn = RtpConnectionPtr();



	dest_conn->sink->stopPlaying();
	dest_conn->state = CONNECTION_STATE_ALLOCATED;
	dest_conn->source_conn = RtpConnectionPtr();

	Medium::close(dest_conn->sink);
	Medium::close(source_conn->source);

	return API_SUCCESS;

	




}


void 
ProcRtpProxy::UponBridgeReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyBridgeReq> req = 
		dynamic_pointer_cast<MsgRtpProxyBridgeReq>(msg);


	// Find source.
	RtpConnectionsMap::iterator iter = 
		_connectionsMap.find(req->rtp_proxy_handle);
	if (iter == _connectionsMap.end())
	{

		LogWarn("conn:" << req->rtp_proxy_handle << " not found");
		SendResponse(req, new MsgRtpProxyNack());
		return;

	}

	RtpConnectionPtr source_connection = iter->second;
	UponUnBridgeReq(source_connection);
	

	// Find destination
	iter = _connectionsMap.find(req->output_conn);
	if (iter == _connectionsMap.end())
	{

		LogWarn("conn:" << req->output_conn << " not found");
		SendResponse(req, new MsgRtpProxyNack());
		return;

	}

	RtpConnectionPtr destination_connection = iter->second;
	UponUnBridgeReq(destination_connection);

	// Bridge
	FramedSource* source = 
		BasicUDPSource::createNew(*_env, source_connection->live_socket.get());

	unsigned const maxPacketSize = 65536; // allow for large UDP packets
	MediaSink* sink = 
		BasicUDPSink::createNew(*_env, destination_connection->live_socket.get(), maxPacketSize);

	source_connection->state = CONNECTION_STATE_INPUT;
	source_connection->source = source;
	source_connection->destination_conn = destination_connection;

	destination_connection->state = CONNECTION_STATE_OUTPUT;
	destination_connection->sink = sink;
	destination_connection->source_conn = source_connection;

	
	if ( sink->startPlaying(*source, NULL, NULL) == FALSE)
	{
		SendResponse(req, new MsgRtpProxyNack());
		LogWarn("ProcRtpProxy::UponBridgeReq error: startPlaying");
		return;
	}

	SendResponse(req, new MsgRtpProxyAck());

	
	//_env->taskScheduler().doEventLoop();

	

	LogDebug(
		"conn:" << source_connection->connection_id << " :" << source_connection->local_cnx_ino << " ==> " <<
		"conn:" << destination_connection->connection_id << " dst:" << destination_connection->local_cnx_ino );

}

ProcRtpProxy::~ProcRtpProxy(void)
{

}

}
