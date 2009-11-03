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
sink(NULL),
rtcp_instance(NULL)
{

}

ProcRtpProxy::RtpConnection::RtpConnection(const RtpConnection &conn)
{
	connection_id	= conn.connection_id;
	live_rtp_socket		= conn.live_rtp_socket;
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
	LogDebug("ProcRtpProxy::InitSockets rtp_proxy_base_port=" << base_port);

	int top_port	 = _conf.GetInt("rtp_proxy_top_port");
	LogDebug("ProcRtpProxy::InitSockets rtp_proxy_top_port=" << top_port);

	int num_of_conns = _conf.GetInt("rtp_proxy_num_of_connections");
	LogDebug("ProcRtpProxy::InitSockets rtp_proxy_num_of_connections=" << num_of_conns);

	string rtp_ip = _conf.GetString("rtp_proxy_ip");
	LogDebug("ProcRtpProxy::InitSockets rtp_proxy_ip=" << rtp_ip);

	const char * input_address_str = rtp_ip.c_str();

	_localInAddr = convert_hname_to_addrin(input_address_str);
	
	if (base_port < 0		 || 
		top_port < base_port || 
		num_of_conns < 1	 ||  
		(top_port - base_port) < 2)
	{
		LogCrit("Error in ports configuration");
		return API_FAILURE;
	}

	//
	// Init Connections
	//
	// rtp connection will receive id according to their ports
	// 
	//
	int curr_port = (base_port % 2) ==  0 ? base_port : base_port + 1;
	for (int i = 0; (i < num_of_conns) && (curr_port < top_port); curr_port+=2) 
	{
		
		Port const input_rtp_port(curr_port);
		Port const input_rtcp_port(curr_port + 1);

		unsigned char const inputTTL = 0; 

		Groupsock * rtp_socket = 
			new Groupsock(*_env, _localInAddr, input_rtp_port, inputTTL);

		if (rtp_socket->socketNum() < 0)
		{
			delete rtp_socket;
			continue;
		}

		Groupsock * rtcp_socket = 
			new Groupsock(*_env, _localInAddr, input_rtcp_port , inputTTL);

		if (rtcp_socket->socketNum() < 0)
		{
			delete rtp_socket;
			delete rtcp_socket;
			continue;
		}

		RtpConnection *conn = new RtpConnection();

		conn->connection_id	  = curr_port;

		conn->live_rtp_socket  = GroupSockPtr(rtp_socket);
		conn->live_rtcp_socket = GroupSockPtr(rtcp_socket);
		conn->local_cnx_ino	   = CnxInfo(_localInAddr,curr_port);

		conn->state = CONNECTION_STATE_AVAILABLE;

		_connectionsMap[conn->connection_id] = RtpConnectionPtr(conn);
		i++;

		LogDebug("Created source connection id:" << i << " port:" << curr_port);

	};

	if (_connectionsMap.size() < num_of_conns)
	{
		LogCrit("ProcRtpProxy::InitSockets - unable open open num_of_conns=" << num_of_conns << " rtp connections");
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
		Unbridge(iter->second);
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
		LogWarn("ProcRtpProxy::UponAllocateReq - No available rtp resource");
		SendResponse(req, new MsgRtpProxyNack());
		return;
	}


	MsgRtpProxyAck* ack = new MsgRtpProxyAck();
	ack->rtp_proxy_handle = candidate->connection_id;
	ack->local_media = candidate->local_cnx_ino;

	candidate->state = CONNECTION_STATE_ALLOCATED;
	candidate->remote_cnx_ino = req->remote_media;

	if (req->remote_media.is_ip_valid() && req->remote_media.is_port_valid())
	{
		candidate->live_rtp_socket->changeDestinationParameters(
			req->remote_media.inaddr(),
			req->remote_media.port_ho(),225);
	}
	
	LogDebug("ProcRtpProxy::UponAllocateReq allocated rtph:" << candidate->connection_id << " local:" << candidate->local_cnx_ino << ", remote:" << req->remote_media.ipporttos());
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

		LogWarn("rtph:" << req->rtp_proxy_handle << " not found");
		SendResponse(req, new MsgRtpProxyNack());
		return;

	}

	RtpConnectionPtr conn = iter->second;

	Unbridge(conn);

	if (conn->rtcp_instance) 
	{
		RTCPInstance::close(conn->rtcp_instance);
		conn->rtcp_instance = NULL;
	}

	if (conn->sink) 
	{
		Medium::close(conn->sink);
		conn->sink = NULL;
	}

	if (conn->source)
	{
		Medium::close(conn->source);
		conn->source = NULL;

	}
	
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

		LogWarn("rtph:" << req->rtp_proxy_handle << " not found");
		SendResponse(req, new MsgRtpProxyNack());
		return;

	}

	RtpConnectionPtr conn = iter->second;

	if (req->media_format.get_media_type() != MediaFormat::MediaType_UNKNOWN)
	{
		conn->media_format = req->media_format;
	}
	

	if (req->remote_media.is_ip_valid() && req->remote_media.is_port_valid())
	{
		conn->remote_cnx_ino = req->remote_media;
		conn->live_rtp_socket->changeDestinationParameters(req->remote_media.inaddr(),
			req->remote_media.port_ho(),0);
	}

	SendResponse(req, new MsgRtpProxyAck());

}


ApiErrorCode
ProcRtpProxy::Unbridge(RtpConnectionPtr conn)
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

	return API_SUCCESS;

}


void 
ProcRtpProxy::UponBridgeReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyBridgeReq> bridge_req = 
		dynamic_pointer_cast<MsgRtpProxyBridgeReq>(msg);


	// Find source.
	RtpConnectionsMap::iterator iter = 
		_connectionsMap.find(bridge_req->rtp_proxy_handle);
	if (iter == _connectionsMap.end())
	{

		LogWarn("ProcRtpProxy::UponBridgeReq - rtph:" << bridge_req->rtp_proxy_handle << " not found");
		SendResponse(bridge_req, new MsgRtpProxyNack());
		return;

	}

	RtpConnectionPtr source_connection = iter->second;
	Unbridge(source_connection);
	

	// Find destination
	iter = _connectionsMap.find(bridge_req->output_conn);
	if (iter == _connectionsMap.end())
	{

		LogWarn("rtph:" << bridge_req->output_conn << " not found");
		SendResponse(bridge_req, new MsgRtpProxyNack());
		return;

	}

	RtpConnectionPtr destination_connection = iter->second;

	if (source_connection->media_format.get_media_type()      == MediaFormat::MediaType_UNKNOWN	|| 
		destination_connection->media_format.get_media_type() == MediaFormat::MediaType_UNKNOWN ||
		destination_connection->media_format != source_connection->media_format)
	{
		LogWarn("ProcRtpProxy::UponBridgeReq - transcoding not supported.");
		SendResponse(bridge_req, new MsgRtpProxyNack());
		return;
	}

	Unbridge(destination_connection);


	//
	// source
	//
	SimpleRTPSource *rtp_source		      = source_connection->source;
	RTCPInstance    *source_rtcp_instance = source_connection->rtcp_instance;

	SimpleRTPSink	*rtp_sink			  = source_connection->sink;
	RTCPInstance    *sink_rtcp_instance	  = source_connection->rtcp_instance;

	MediaFormat		media_format		  = source_connection->media_format;

	{
		//
		// source
		//
		if (!rtp_source)
		{
			rtp_source = 
				SimpleRTPSource::createNew(
				*_env,											// env
				source_connection->live_rtp_socket.get(),		// RTPgs
				media_format.sdp_mapping(),			// rtpPayloadFormat
				media_format.sampling_rate(),		// rtpTimestampFrequency
				media_format.sdp_name_tos().c_str()	// mimeTypeString
				);

			if (rtp_source == NULL)
			{
				LogWarn("ProcRtpProxy::UponBridgeReq - Cannot create source");
				goto error;
			};
		}
		

		if (!source_rtcp_instance)
		{
			source_rtcp_instance  = 
				RTCPInstance::createNew(
				*_env,											// env	
				source_connection->live_rtcp_socket.get(),		// RTCPgs
				500,											// totSessionBW
				(const unsigned char *)"ivrworx",				// cname
				NULL,											// sink
				rtp_source										// source
				);
			if (source_rtcp_instance == NULL)
			{
				LogWarn("ProcRtpProxy::UponBridgeReq - Cannot create source rtcp instance");
				goto error;
			};
		}
		

		//
		// sink
		//
		if (!rtp_sink)
		{
			rtp_sink = 
				SimpleRTPSink::createNew(
					*_env,											 // env
					destination_connection->live_rtcp_socket.get(),  // RTPgs
					media_format.sdp_mapping(),			 // rtpPayloadFormat
					media_format.sampling_rate(),		 // rtpTimestampFrequency
					media_format.sdp_name_tos().c_str(), // sdpMediaTypeString
					media_format.sdp_name_tos().c_str()  // rtpPayloadFormatName
					);
			if (rtp_sink == NULL)
			{
				LogWarn("ProcRtpProxy::UponBridgeReq - Cannot create sink instance");
				goto error;
			}
		}
		

		if (!sink_rtcp_instance)
		{
			sink_rtcp_instance  = 
				RTCPInstance::createNew(
					*_env,											 // env	
					destination_connection->live_rtcp_socket.get(),  // RTCPgs
					500,											 // totSessionBW
					(const unsigned char *)"ivrworx",				 // cname
					rtp_sink,										 // sink	
					NULL											 // source 	
					);					
			if (sink_rtcp_instance == NULL)
			{
				LogWarn("ProcRtpProxy::UponBridgeReq - Cannot create sink instance");
				goto error;
			}
		}


		source_connection->state			= CONNECTION_STATE_INPUT;
		source_connection->source			= rtp_source;
		source_connection->destination_conn = destination_connection;
		source_connection->rtcp_instance	= source_rtcp_instance;

		destination_connection->state			= CONNECTION_STATE_OUTPUT;
		destination_connection->sink			= rtp_sink;
		destination_connection->source_conn		= source_connection;
		destination_connection->rtcp_instance	= sink_rtcp_instance;


		
		if (rtp_sink->startPlaying(*rtp_source, NULL, NULL) == FALSE)
		{
			LogWarn("ProcRtpProxy::UponBridgeReq error: startPlaying");
			goto error;
		}

		LogDebug(
			source_connection->remote_cnx_ino << " -> " <<
			source_connection->local_cnx_ino  << 
			" (rtph:" << source_connection->connection_id << ") ==> (" <<
			"rtph:" << destination_connection->connection_id << ") " <<
			destination_connection->local_cnx_ino << " ->" <<
			destination_connection->remote_cnx_ino );

	}
	SendResponse(bridge_req, new MsgRtpProxyAck());

	return;

error:

	if (rtp_source != NULL) Medium::close(rtp_source);
	if (source_rtcp_instance!= NULL) RTCPInstance::close(source_rtcp_instance);

	if (rtp_sink != NULL) Medium::close(rtp_sink);
	if (sink_rtcp_instance != NULL) Medium::close(sink_rtcp_instance);

	
	source_connection->source			= NULL;
	source_connection->destination_conn = RtpConnectionPtr();
	source_connection->rtcp_instance	= NULL;

	
	destination_connection->sink			= NULL;
	destination_connection->source_conn		= RtpConnectionPtr();
	destination_connection->rtcp_instance	= NULL;

	SendResponse(bridge_req, new MsgRtpProxyNack());
	return;
	
}

ProcRtpProxy::~ProcRtpProxy(void)
{

}

}
