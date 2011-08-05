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
#include "ProcLive555RtpProxy.h"
#include "IwUsageEnvironment.h"
#include "MockRtpSink.h"

#define RTP_PROXY_POLL_TIME 10

namespace ivrworx
{


RtpConnection::RtpConnection()
:connection_id(NULL),
state(CONNECTION_STATE_ALLOCATED),
source(NULL),
sink(NULL),
rtcp_instance(NULL)
{

}

RtpConnection::~RtpConnection()
{

}

void processIwMessagesTask(void* clientData) 
{
	ProcLive555RtpProxy *proxy = (ProcLive555RtpProxy *)clientData;

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

IwSimpleRTPSource*
IwSimpleRTPSource::createNew(UsageEnvironment& env,
							 Groupsock* RTPgs,
							 unsigned char rtpPayloadFormat,
							 unsigned rtpTimestampFrequency,
							 char const* mimeTypeString,
							 unsigned offset, Boolean doNormalMBitRule)
{
	return new IwSimpleRTPSource(env, RTPgs, rtpPayloadFormat,
		rtpTimestampFrequency,
		mimeTypeString, offset, doNormalMBitRule);
}

Boolean 
IwSimpleRTPSource::processUnknownPayload(BufferedPacket* packet)
{
	
	if (!handler)
		return False;

	if (!_packetLogged )
	{
		_packetLogged = TRUE;
		LogDebug("IwSimpleRTPSource::processUnknownPayload received RTP timestamp:" << packet->RTPTimestamp() << ", ssrc: 0x" << hex << SSRC());

	}

	unsigned char payload = packet->header()[1];

	if (payload == dtmf_format.sdp_mapping() && 
		_lastTimestamp != packet->RTPTimestamp())
	{
		char event_field	= packet->data()[0];
		char ervolume_field = packet->data()[1];
		int  duration		= *(&(packet->data()[2]));
		
		// edge
		if (ervolume_field & 0x80)
		{
			_lastTimestamp = packet->RTPTimestamp();
			MsgRtpProxyDtmfEvt *evt = new MsgRtpProxyDtmfEvt();
			char buffer[32];
			buffer[0] = '\0';

			switch (event_field)
			{
				case 10: evt->signal = "*"; break;
				case 11: evt->signal = "#"; break;
				case 12: evt->signal = "A"; break;
				case 13: evt->signal = "B"; break;
				case 14: evt->signal = "C"; break;
				case 15: evt->signal = "D"; break;
				case 16: evt->signal = "E"; break;
				case 17: evt->signal = "F"; break;
				default:
					evt->signal = ::itoa(event_field,buffer,10);
			}
				
			handler->Send(evt);

		}
		
	}
	return False;
}

IwSimpleRTPSource::IwSimpleRTPSource(UsageEnvironment& env, 
 Groupsock* RTPgs,
 unsigned char rtpPayloadFormat,
 unsigned rtpTimestampFrequency,
 char const* mimeTypeString, unsigned offset,
 Boolean doNormalMBitRule):
	SimpleRTPSource(
		env, 
		RTPgs,
		rtpPayloadFormat, 
		rtpTimestampFrequency,
		mimeTypeString, 
		offset,
		doNormalMBitRule),
		_packetLogged(FALSE)
{
	_lastTimestamp = 0;
}


ProcLive555RtpProxy::ProcLive555RtpProxy(LpHandlePair pair, ConfigurationPtr conf):
LightweightProcess(pair,"ProcLive555RtpProxy"),
_conf(conf),
_env(NULL),
_scheduler(NULL),
_stopChar('\0')
{

	ServiceId(_conf->GetString("live555rtpproxy/uri"));
	
}

ApiErrorCode 
ProcLive555RtpProxy::InitSockets()
{
	FUNCTRACKER;

	
	int base_port	 = _conf->GetInt("live555rtpproxy/rtp_proxy_base_port");
	LogDebug("ProcLive555RtpProxy::InitSockets rtp_proxy_base_port=" << base_port);

	int top_port	 = _conf->GetInt("live555rtpproxy/rtp_proxy_top_port");
	LogDebug("ProcLive555RtpProxy::InitSockets rtp_proxy_top_port=" << top_port);

	unsigned int num_of_conns = _conf->GetInt("live555rtpproxy/rtp_proxy_num_of_connections");
	LogDebug("ProcLive555RtpProxy::InitSockets rtp_proxy_num_of_connections=" << num_of_conns);

	string rtp_ip = _conf->GetString("live555rtpproxy/rtp_proxy_ip");
	LogDebug("ProcLive555RtpProxy::InitSockets rtp_proxy_ip=" << rtp_ip);

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
		LogCrit("ProcLive555RtpProxy::InitSockets - unable open open num_of_conns=" << num_of_conns << " rtp connections");
		return API_FAILURE;
	};

	return API_SUCCESS;


}


void 
ProcLive555RtpProxy::real_run()
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
ProcLive555RtpProxy::UponAllocateReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyAllocateReq> req = 
		dynamic_pointer_cast<MsgRtpProxyAllocateReq>(msg);

	if (!req || req->offer.body.empty())
	{
		LogWarn("ProcLive555RtpProxy::UponAllocateReq - must supply sdp");
		SendResponse(req, new MsgRtpProxyNack());
		return;
	}

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
		LogWarn("ProcLive555RtpProxy::UponAllocateReq - No available rtp resource");
		SendResponse(req, new MsgRtpProxyNack());
		return;
	}

	candidate->state = CONNECTION_STATE_ALLOCATED;

	SdpParser p(req->offer.body);
	SdpParser::Medium m = p.first_audio_medium();

	if (m.list.empty())
	{
		LogWarn("ProcLive555RtpProxy::UponAllocateReq - must supply sdp with codec list");
		SendResponse(req, new MsgRtpProxyNack());
		return;
	}

	candidate->remote_cnx_ino = m.connection;
	candidate->media_format = *(m.list.begin()); 
	candidate->cn_format	= m.cn_format;
	candidate->dtmf_format	= m.dtmf_format;

	candidate->handler		= req->handler;

	if (m.connection.is_ip_valid() && 
		m.connection.is_port_valid())
	{
		
		candidate->live_rtp_socket->changeDestinationParameters(
			m.connection.inaddr(),
			m.connection.port_ho(),225);

		candidate->live_rtcp_socket->changeDestinationParameters(
			m.connection.inaddr(),
			m.connection.port_ho() + 1,225);
	}

	DWORD time = ::GetTickCount();

	stringstream str;
	str <<  "v=0\r\n"
		<<  "o=live555proxy "   << time << " " << time << " IN IP4 " << candidate->local_cnx_ino.iptoa() << "\r\n"
		<<  "s=live555proxy\r\n"
		<<  "c=IN IP4 " << candidate->local_cnx_ino.iptoa() << "\r\n"
		<<  "t=0 0\r\n"
		<<  "m=audio " << candidate->local_cnx_ino.port_ho() << " RTP/AVP";

	m.append_codec_list(str) ; str << "\r\n"; 
	m.append_rtp_map(str) ; str << "\r\n\r\n";

	MsgRtpProxyAck* ack = new MsgRtpProxyAck();
	ack->rtp_proxy_handle = candidate->connection_id;
	ack->offer.type = "application/sdp";
	ack->offer.body = str.str();
	
	LogDebug("ProcLive555RtpProxy::UponAllocateReq allocated rtph:" << candidate->connection_id );

	if (IW_FAILURE(Bridge(candidate, RtpConnectionPtr(),FALSE)))
	{
		delete ack;
		SendResponse(req, new MsgRtpProxyNack());
	} 
	else
	{
		SendResponse(req, ack);
	}
	
	
}

void 
ProcLive555RtpProxy::UponDeallocateReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyDeallocateReq> req = 
		dynamic_pointer_cast<MsgRtpProxyDeallocateReq>(msg);

	// Find source.
	RtpConnectionsMap::iterator iter = 
		_connectionsMap.find(req->rtp_proxy_handle);
	if (iter == _connectionsMap.end())
	{

		LogWarn("ProcLive555RtpProxy::UponDeallocateReq rtph:" << req->rtp_proxy_handle << " not found");
		SendResponse(req, new MsgRtpProxyNack());
		return;

	}

	RtpConnectionPtr conn = iter->second;
	
	if (conn->state == CONNECTION_STATE_AVAILABLE)
	{
		LogWarn("ProcLive555RtpProxy::UponDeallocateReq rtph:" << req->rtp_proxy_handle << " not allocated");
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
ProcLive555RtpProxy::UponModifyReq(IwMessagePtr msg)
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

	SdpParser p(req->offer.body);
	SdpParser::Medium m = p.first_audio_medium();
	MediaFormat media_format = *m.list.begin();

	

	if (media_format.get_media_type() != MediaFormat::MediaType_UNKNOWN)
	{
		conn->media_format = media_format;
	}
	

	if (m.connection.is_ip_valid() && m.connection.is_port_valid())
	{
		int port_ho = m.connection.port_ho();

		conn->remote_cnx_ino = m.connection;
		conn->live_rtp_socket->changeDestinationParameters(
			m.connection.inaddr(),
			Port(port_ho),
			255);

		// should be passed in message
		conn->live_rtcp_socket->changeDestinationParameters(
			m.connection.inaddr(),
			Port(port_ho + 1),
			255);
	}

	SendResponse(req, new MsgRtpProxyAck());

}
ApiErrorCode
ProcLive555RtpProxy::DoUnbridge(RtpConnectionPtr src, RtpConnectionPtr dst)
{
	FUNCTRACKER;

	if (src && src->source)
	{
		src->source->stopGettingFrames();
		src->state = (CONNECTION_STATE) (src->state & (!CONNECTION_STATE_INPUT));
		src->destination_conn = RtpConnectionPtr();
	}

	if (dst && dst->sink)
	{
		dst->sink->stopPlaying();
		dst->state = (CONNECTION_STATE) (dst->state & (!CONNECTION_STATE_OUTPUT));
		dst->source_conn = RtpConnectionPtr();
	}

	return API_SUCCESS;

}

ApiErrorCode
ProcLive555RtpProxy::Unbridge(RtpConnectionPtr conn)
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
			DoUnbridge(conn,conn->destination_conn);
			break;
		}
	case CONNECTION_STATE_OUTPUT:
		{
			DoUnbridge(conn->destination_conn,conn);
			break;
		}

	case CONNECTION_STATE_FULLDUPLEX:
		{
			DoUnbridge(conn,conn->destination_conn);
			DoUnbridge(conn->destination_conn,conn);
			break;
		}
	}

	return API_SUCCESS;

}
ApiErrorCode
ProcLive555RtpProxy::Bridge(RtpConnectionPtr source_connection, RtpConnectionPtr destination_connection, BOOL fullDuplex)
{	
	Unbridge(source_connection);

	// Find destination
	if (destination_connection) 
		Unbridge(destination_connection);

	ApiErrorCode res = DoBridge(source_connection,destination_connection);
	if (IW_FAILURE(res)) 
		return res;

	if (fullDuplex && destination_connection)
		res = DoBridge(destination_connection,source_connection);

	if (IW_FAILURE(res))
		Unbridge(source_connection);
	
	return res;

}


ApiErrorCode
ProcLive555RtpProxy::DoBridge(RtpConnectionPtr src, RtpConnectionPtr destination_connection)
{
	
	RtpConnectionPtr source_connection = src;
	
	BOOL using_dummy_sink = FALSE;
	// Find destination
	if (!destination_connection) 
	{
		using_dummy_sink = TRUE;
		destination_connection = RtpConnectionPtr(new RtpConnection());
	}

	//
	// source
	//
	IwSimpleRTPSource *rtp_source		  = source_connection->source;
	RTCPInstance    *source_rtcp_instance = source_connection->rtcp_instance;

	MediaSink		*rtp_sink			  = destination_connection->sink;
	RTCPInstance    *sink_rtcp_instance	  = destination_connection->rtcp_instance;

	MediaFormat		media_format		  = source_connection->media_format;

	{
		//
		// source
		//
		if (!rtp_source)
		{
			rtp_source = 
				IwSimpleRTPSource::createNew(
				*_env,											// env
				source_connection->live_rtp_socket.get(),		// RTPgs
				media_format.sdp_mapping(),			// rtpPayloadFormat
				media_format.sampling_rate(),		// rtpTimestampFrequency
				media_format.sdp_name_tos().c_str()	// mimeTypeString
				);

			rtp_source->cn_format   = source_connection->cn_format;
			rtp_source->dtmf_format = source_connection->dtmf_format;
			rtp_source->handler		= source_connection->handler;

			if (rtp_source == NULL)
			{
				LogWarn("ProcLive555RtpProxy::UponBridgeReq - Cannot create source");
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
				LogWarn("ProcLive555RtpProxy::UponBridgeReq - Cannot create source rtcp instance");
				goto error;
			};
		}

		if (using_dummy_sink)
		{
			rtp_sink = new MockRtpSink(*_env);

		} // dummy sink
		else
		{
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
					LogWarn("ProcLive555RtpProxy::UponBridgeReq - Cannot create sink instance");
					goto error;
				}

				((SimpleRTPSink*)rtp_sink)->setPacketSizes(
					_conf->GetInt("preferred_rtp_size"),
					_conf->GetInt("max_rtp_size"));
			}// rtp_sink creation


			if (!sink_rtcp_instance)
			{
				sink_rtcp_instance  = 
					RTCPInstance::createNew(
					*_env,											 // env	
					destination_connection->live_rtcp_socket.get(),  // RTCPgs
					500,											 // totSessionBW
					(const unsigned char *)"ivrworx",				 // cname
					(SimpleRTPSink*)rtp_sink,						 // sink	
					NULL											 // source 	
					);					
				if (sink_rtcp_instance == NULL)
				{
					LogWarn("ProcLive555RtpProxy::UponBridgeReq - Cannot create sink instance");
					goto error;
				}
			} // rtcp creation
		}// non dummy sink

		source_connection->state			= (CONNECTION_STATE) (source_connection->state | CONNECTION_STATE_INPUT);
		source_connection->source			= rtp_source;
		source_connection->destination_conn = destination_connection;
		source_connection->rtcp_instance	= source_rtcp_instance;

		destination_connection->state			= (CONNECTION_STATE)( destination_connection->state |CONNECTION_STATE_OUTPUT);
		destination_connection->sink			= rtp_sink;
		destination_connection->source_conn		= source_connection;
		destination_connection->rtcp_instance	= sink_rtcp_instance;

		if (rtp_sink->startPlaying(*rtp_source, NULL, NULL) == FALSE)
		{
			LogWarn("ProcLive555RtpProxy::UponBridgeReq error: startPlaying");
			goto error;
		}
		LogDebug( 
			    source_connection->remote_cnx_ino << " -> " << source_connection->local_cnx_ino  
			<< " (rtph:" << source_connection->connection_id << "," << source_connection->state 
			<< ") ==> (" 
			<<	"rtph:" << destination_connection->connection_id << "," << destination_connection->state << ")" 
			<< destination_connection->local_cnx_ino << " ->" 
			<< destination_connection->remote_cnx_ino );

	}
	;

	return API_SUCCESS;

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

	return API_FAILURE;


}


void 
ProcLive555RtpProxy::UponBridgeReq(IwMessagePtr msg)
{
	FUNCTRACKER;

	shared_ptr<MsgRtpProxyBridgeReq> bridge_req = 
		dynamic_pointer_cast<MsgRtpProxyBridgeReq>(msg);


	//
	// Find source.
	//
	RtpConnectionsMap::iterator iter = 
		_connectionsMap.find(bridge_req->rtp_proxy_handle);
	if (iter == _connectionsMap.end())
	{

		LogWarn("ProcLive555RtpProxy::UponBridgeReq - rtph:" << bridge_req->rtp_proxy_handle << " not found");
		SendResponse(bridge_req, new MsgRtpProxyNack());
		return;
	};

	RtpConnectionPtr source_connection = iter->second;

	//
	// Find destination.
	//
	iter = _connectionsMap.find(bridge_req->output_conn);
	if (iter == _connectionsMap.end())
	{

		LogWarn("dst rtph:" << bridge_req->output_conn << " not found");
		SendResponse(bridge_req, new MsgRtpProxyNack());
		return;
	}

	RtpConnectionPtr destination_connection = iter->second;

	// check codec settings
	if (source_connection->media_format.get_media_type()      == MediaFormat::MediaType_UNKNOWN	|| 
		destination_connection->media_format.get_media_type() == MediaFormat::MediaType_UNKNOWN ||
		destination_connection->media_format != source_connection->media_format)
	{
		LogWarn("ProcLive555RtpProxy::UponBridgeReq - transcoding not supported src rtph:"
			<< source_connection->connection_id << "(" << source_connection->media_format << ")" << 
			" <-> dst rtph:" << destination_connection->connection_id << "(" << destination_connection->media_format << ")");

		SendResponse(bridge_req, new MsgRtpProxyNack());
		return;
	}

	if (IW_FAILURE(Bridge(source_connection, destination_connection,bridge_req->full_duplex)))
	{
		SendResponse(bridge_req, new MsgRtpProxyNack());
		return;
	} 
	else
	{
		SendResponse(bridge_req, new MsgRtpProxyAck());
		return;
	}
	
}

ProcLive555RtpProxy::~ProcLive555RtpProxy(void)
{

}

}
