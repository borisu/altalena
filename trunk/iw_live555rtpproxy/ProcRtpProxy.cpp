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
#include "IwUsageEnvironment.h"

#define RTP_PROXY_POLL_TIME 10

namespace ivrworx
{

static in_addr 
convert_hname_to_addrin(const char *name)
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


ProcRtpProxy::RtpConnection::RtpConnection()
:connection_id(NULL),
state(CONNECTION_STATE_AVAILABLE),
source(NULL),
sink(NULL),
rtcp_instance(NULL)
{

}

ProcRtpProxy::RtpConnection::~RtpConnection()
{

}

void processIwMessagesTask(void* clientData) 
{
	ProcRtpProxy *proxy = (ProcRtpProxy *)clientData;

	if (proxy->InboundPending())
	{

		ApiErrorCode res = API_SUCCESS;
		IwMessagePtr msg = proxy->GetInboundMessage(Seconds(0),res);
		if (!msg)
		{
			goto reschedule;
		}

		switch (msg->message_id)
		{
		case MSG_RTP_PROXY_ALLOCATE_REQ:
			{ 
				proxy->UponAllocateReq(msg);
				break ;
			}; 
		case MSG_RTP_PROXY_BRIDGE_REQ:
			{ 
				proxy->UponBridgeReq(msg);
				break ;
			}; 
		case MSG_RTP_PROXY_MODIFY_REQ:
			{ 
				proxy->UponModifyReq(msg);
				break; 
			}; 
		case MSG_RTP_PROXY_DEALLOCATE_REQ:
			{ 
				proxy->UponDeallocateReq(msg);
				break; 
			}; 
		case MSG_PROC_SHUTDOWN_REQ:
			{ 
				proxy->_stopChar = 'S';
				proxy->SendResponse(msg,new MsgShutdownAck());
				return;
				break; 
			}; 
		default:
			{
				if (proxy->HandleOOBMessage(msg) == FALSE)
				{
					LogWarn("Received unknown message " << msg->message_id_str);
				} // if
			}// default
		}// switch
	}// while

reschedule:

	// iw messages are polled once in 5 ms
	proxy->_env->taskScheduler().scheduleDelayedTask(RTP_PROXY_POLL_TIME,
		(TaskFunc*)processIwMessagesTask,proxy);

}


ProcRtpProxy::ProcRtpProxy(LpHandlePair pair, Configuration &conf):
LightweightProcess(pair,"ProcRtpProxy"),
_conf(conf),
_env(NULL),
_scheduler(NULL),
_stopChar('\0')
{
	static const size_t buf_len = 1024;
	char buf[buf_len];
	_snprintf_s(buf,buf_len,"proto=rtpproxy;vendor=live555;uid=%d",_processId);

	ServiceId(buf);
	
}

ApiErrorCode 
ProcRtpProxy::InitSockets()
{
	FUNCTRACKER;

	
	int base_port	 = _conf.GetInt("rtp_proxy_base_port");
	LogDebug("ProcRtpProxy::InitSockets rtp_proxy_base_port=" << base_port);

	int top_port	 = _conf.GetInt("rtp_proxy_top_port");
	LogDebug("ProcRtpProxy::InitSockets rtp_proxy_top_port=" << top_port);

	unsigned int num_of_conns = _conf.GetInt("rtp_proxy_num_of_connections");
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
	for (unsigned int i = 0; (i < num_of_conns) && (curr_port < top_port); curr_port+=2) 
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
	_env  =	IwUsageEnvironment::createNew(*_scheduler);
	
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

	I_AM_READY;

	
	_env->taskScheduler().scheduleDelayedTask(RTP_PROXY_POLL_TIME,
		(TaskFunc*)processIwMessagesTask,this);

	_env->taskScheduler().doEventLoop(&_stopChar);

	
	for (RtpConnectionsMap::iterator iter = _connectionsMap.begin();
		iter != _connectionsMap.end();
		++iter)
	{
		Unbridge(iter->second);
	};

	
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
	

	if (req->remote_media.is_ip_valid() && 
		req->remote_media.is_port_valid())
	{
		candidate->remote_cnx_ino = req->remote_media;

		candidate->live_rtp_socket->changeDestinationParameters(
			req->remote_media.inaddr(),
			req->remote_media.port_ho(),225);

		candidate->live_rtcp_socket->changeDestinationParameters(
			req->remote_media.inaddr(),
			req->remote_media.port_ho() + 1,225);
	}

	if (req->media_format != MediaFormat::UNKNOWN)
	{
		candidate->media_format = req->media_format; 
	}
	
	LogDebug("ProcRtpProxy::UponAllocateReq allocated rtph:" << candidate->connection_id << " local:" << candidate->local_cnx_ino << ", remote:" << req->remote_media.ipporttos() << ", media format:" << req->media_format.sdp_name_tos());
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

		LogWarn("ProcRtpProxy::UponDeallocateReq rtph:" << req->rtp_proxy_handle << " not found");
		SendResponse(req, new MsgRtpProxyNack());
		return;

	}

	RtpConnectionPtr conn = iter->second;
	
	if (conn->state == CONNECTION_STATE_AVAILABLE)
	{
		LogWarn("ProcRtpProxy::UponDeallocateReq rtph:" << req->rtp_proxy_handle << " not allocated");
		SendResponse(req, new MsgRtpProxyNack());
		return;
	}

	// safe side
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
		int port_ho = req->remote_media.port_ho();

		conn->remote_cnx_ino = req->remote_media;
		conn->live_rtp_socket->changeDestinationParameters(
			req->remote_media.inaddr(),
			Port(port_ho),
			255);

		// should be passed in message
		conn->live_rtcp_socket->changeDestinationParameters(
			req->remote_media.inaddr(),
			Port(port_ho + 1),
			255);
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
		LogWarn("ProcRtpProxy::UponBridgeReq - transcoding not supported src rtph:"
			<< source_connection->connection_id << "(" << source_connection->media_format << ")" << 
			 " <-> dst rtph:" << destination_connection->connection_id << "(" << destination_connection->media_format << ")");

		SendResponse(bridge_req, new MsgRtpProxyNack());
		return;
	}

	Unbridge(destination_connection);


	//
	// source
	//
	SimpleRTPSource *rtp_source		      = source_connection->source;
	RTCPInstance    *source_rtcp_instance = source_connection->rtcp_instance;

	SimpleRTPSink	*rtp_sink			  = destination_connection->sink;
	RTCPInstance    *sink_rtcp_instance	  = destination_connection->rtcp_instance;

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
					destination_connection->live_rtp_socket.get(),  // RTPgs
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
	if (sink_rtcp_instance != NULL) RTCPInstance::close(sink_rtcp_instance);

	
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
