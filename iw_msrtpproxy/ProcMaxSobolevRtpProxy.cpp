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
#include "ProcMaxSobolevRtpProxy.h"
#include "RtpProxySession.h"

const ULONG SOCKET_COMPLETION_KEY = 123;
const int   DATA_BUFSIZE = 4096;



namespace ivrworx
{
	ProcMaxSobolevRtpProxy::ProcMaxSobolevRtpProxy(ConfigurationPtr conf,LpHandlePair pair):
	LightweightProcess(pair,"ProcMaxSobolevRtpProxy"),
	_conf(conf),
	_s(NULL),
	_sessionCounter(800000),
	_correlationCounter(IW_UNDEFINED),
	g_hIOCompletionPort(NULL)
	{
		FUNCTRACKER;

		ServiceId(_conf->GetString("msrtpproxy/uri"));

		_iocpPtr = IocpInterruptorPtr(new IocpInterruptor());
		_inbound->HandleInterruptor(_iocpPtr);
	}

	ProcMaxSobolevRtpProxy::ReqCtx::ReqCtx():type(REQUEST_TYPE_NONE)
	{

	};

	void 
	ProcMaxSobolevRtpProxy::real_run()
	{
		FUNCTRACKER;

		if (IW_FAILURE(InitSockets()))
			throw critical_exception("ProcMaxSobolevRtpProxy::real_run - Cannot init neworking layer");

		I_AM_READY;

		WSAOVERLAPPED RecvOverlapped;
		memset(&RecvOverlapped,0,sizeof(RecvOverlapped));

		char buffer[DATA_BUFSIZE];
		buffer[0] = '\0';
		WSABUF DataBuf;
		DataBuf.buf = buffer;
		DataBuf.len = sizeof(buffer);

		DWORD RecvBytes = 0, Flags = 0;

		int error = ::WSARecv(
			_s,				 // A descriptor identifying a connected socket.	
			&DataBuf,		 // A pointer to an array of WSABUF structures. Each WSABUF structure contains a pointer to a buffer and the length, in bytes, of the buffer.
			1,				 // The number of WSABUF structures in the lpBuffers array.
			&RecvBytes,		 // A pointer to the number, in bytes, of data received by this call if the receive operation completes immediately. 
			&Flags,			 // A pointer to flags used to modify the behavior of the WSARecv function call. For more information, see the Remarks section.
			&RecvOverlapped, // A pointer to a WSAOVERLAPPED structure (ignored for nonoverlapped sockets).
			NULL);			 // A pointer to the completion routine called when the receive operation has been completed (ignored for nonoverlapped sockets).

		if ( (error == SOCKET_ERROR) && (WSA_IO_PENDING != (error = WSAGetLastError()))) {
			LogSysError("::WSARecv(1st)");
			return;
		}

		BOOL shutdown_flag = FALSE;
		while (shutdown_flag == FALSE)
		{

			DWORD number_of_bytes    = 0;
			ULONG_PTR completion_key = 0;
			LPOVERLAPPED lpOverlapped = NULL;

			BOOL res = ::GetQueuedCompletionStatus(
				_iocpPtr->WinHandle(),		// A handle to the completion port. To create a completion port, use the CreateIoCompletionPort function.
				&number_of_bytes,			// A pointer to a variable that receives the number of bytes transferred during an I/O operation that has completed.
				&completion_key,			// A pointer to a variable that receives the completion key value associated with the file handle whose I/O operation has completed. A completion key is a per-file key that is specified in a call to CreateIoCompletionPort.
				&lpOverlapped,				// A pointer to a variable that receives the address of the OVERLAPPED structure that was specified when the completed I/O operation was started. 
				60000						// The number of milliseconds that the caller is willing to wait for a completion packet to appear at the completion port. If a completion packet does not appear within the specified time, the function times out, returns FALSE, and sets *lpOverlapped to NULL.
				);

			if (completion_key == SOCKET_COMPLETION_KEY)
			{
				// safety
				DataBuf.buf[number_of_bytes >= DATA_BUFSIZE ? DATA_BUFSIZE : number_of_bytes]= 0;
				string s = DataBuf.buf;

				onSocketOverlappedReceived(s);

				// issue another read request
				int error = ::WSARecv(_s, &DataBuf, 1, &RecvBytes, &Flags, &RecvOverlapped, NULL);
				if ( (error == SOCKET_ERROR) && (WSA_IO_PENDING != (error = WSAGetLastError()))) {
					LogSysError("::WSARecv(2nd)");
					return;
				}
			}

			//IX_PROFILE_CHECK_INTERVAL(10000);

			// error during overlapped I/O?
			int last_err = ::GetLastError();
			if (res == FALSE)
			{
				if (last_err != WAIT_TIMEOUT)
				{
					LogSysError("::GetQueuedCompletionStatus");
					throw;
				}
				else 
				{
					LogInfo("Ims keep alive.");
					continue;
				}
			}

			if (!InboundPending())
				continue;

			ApiErrorCode iw_res = API_SUCCESS;
			IwMessagePtr msg = GetInboundMessage(Seconds(0),iw_res);
			if (!msg)
				continue;

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
			case MSG_PROC_SHUTDOWN_REQ:
				{ 
					shutdown_flag = TRUE;
					SendResponse(msg,new MsgShutdownAck());
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


		WSACleanup();
	}

	ProcMaxSobolevRtpProxy::~ProcMaxSobolevRtpProxy(void)
	{
		FUNCTRACKER;
	}

	ApiErrorCode
	ProcMaxSobolevRtpProxy::onSocketOverlappedReceived(const string &response)
	{
		FUNCTRACKER;
		LogDebug("ProcMaxSobolevRtpProxy::onSocketOverlappedReceived - rsp:" << response);

		if (response[0] == 'E')
			return API_SUCCESS;

		stringstream stream(response);

		int correlation_id = IW_UNDEFINED;
		stream >> correlation_id;

		if (correlation_id < 0)
		{
			LogDebug("ProcMaxSobolevRtpProxy::onSocketOverlappedReceived - error parsing correlation id");
			return API_FAILURE;
		}
			

		RequestsMap::iterator iter = _reqMap.find(correlation_id);
		if (  iter == _reqMap.end())
		{
			LogDebug("ProcMaxSobolevRtpProxy::onSocketOverlappedReceived - strayed request");
			return API_SUCCESS;
		}

		ReqCtx &ctx = (*iter).second;
		switch (ctx.type)
		{
		case REQUEST_TYPE_UPDATE:
			{
// 				DWORD time = ::GetTickCount();
// 
// 				stringstream str;
// 				str <<  "v=0\r\n"
// 					<<  "o=live555proxy "   << time << " " << time << " IN IP4 " << candidate->local_cnx_ino.iptoa() << "\r\n"
// 					<<  "s=live555proxy\r\n"
// 					<<  "c=IN IP4 " << candidate->local_cnx_ino.iptoa() << "\r\n"
// 					<<  "t=0 0\r\n"
// 					<<  "m=audio " << candidate->local_cnx_ino.port_ho() << " RTP/AVP";
// 
// 				m.append_codec_list(str) ; str << "\r\n"; 
// 				m.append_rtp_map(str) ; str << "\r\n\r\n";
// 
// 				MsgRtpProxyAck* ack = new MsgRtpProxyAck();
// 				ack->rtp_proxy_handle = candidate->connection_id;
// 				ack->offer.type = "application/sdp";
// 				ack->offer.body = str.str();
// 
// 				LogDebug("ProcLive555RtpProxy::UponAllocateReq allocated rtph:" << candidate->connection_id );
// 				SendResponse(req, ack);

			}

		}



		
		return API_SUCCESS;
	}


	ApiErrorCode
	ProcMaxSobolevRtpProxy::InitSockets()
	{
		FUNCTRACKER;

		int port = IW_UNDEFINED;
		string host;


		WSADATA wsadata;
		int error = ::WSAStartup(0x0202, &wsadata);

		//Did something happen?
		if (error)
		{
			LogSysError("::WSAStartup");
			return API_FAILURE;
		}

		//Did we get the right Winsock version?
		if (wsadata.wVersion != 0x0202)
		{
			LogWarn("Incorrect socket version:" <<wsadata.wVersion);
			goto socket_error;
		}


		//
		// create listening socket
		//
		_s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //Create socket
		if (_s == INVALID_SOCKET)
		{
			LogSysError("::socket");
			goto socket_error;
		};

		
		
		//
		// associate with completion port
		//
		HANDLE h = ::CreateIoCompletionPort(
			(HANDLE)_s,
			_iocpPtr->WinHandle(),
			SOCKET_COMPLETION_KEY,
			0
			);
		if (h == NULL)
		{
			LogSysError("::CreateIoCompletionPort");
			goto socket_error;
		}

		

		//
		// connect
		//
		port = _conf->GetInt("msrtpproxy/control_port");
		host = _conf->GetString("msrtpproxy/control_host");

		struct hostent *remoteHost = NULL;
		remoteHost = ::gethostbyname(host.c_str());
		if (remoteHost == NULL)
		{
			LogSysError("::gethostbyname");
			goto socket_error;
		}

		SOCKADDR_IN target; //Socket address information
		target.sin_family	= AF_INET; // address family Internet
		target.sin_port		= htons (port); //Port to connect on
		target.sin_addr.s_addr = *(u_long *) remoteHost->h_addr; //Target IP

		//Try connecting...
		if (::connect(_s, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
		{
			LogSysError("::connect");
			goto socket_error;
		}


		return API_SUCCESS;

socket_error:

		if (_s)
			::closesocket(_s);

		if (g_hIOCompletionPort)
			::CloseHandle(g_hIOCompletionPort);

		::WSACleanup();

		return API_FAILURE;
		
	}

	


	void 
	ProcMaxSobolevRtpProxy::UponAllocateReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

		shared_ptr<MsgRtpProxyAllocateReq> req = 
			dynamic_pointer_cast<MsgRtpProxyAllocateReq>(msg);

		if (!req || req->offer.body.empty())
		{
			LogWarn("ProcMaxSobolevRtpProxy::UponAllocateReq - must supply sdp");
			SendResponse(req, new MsgRtpProxyNack());
			return;
		}

		SdpParser p(req->offer.body);
		SdpParser::Medium m = p.first_audio_medium();

		if (m.list.empty())
		{
			LogWarn("ProcMaxSobolevRtpProxy::UponAllocateReq - must supply sdp with codec list");
			SendResponse(req, new MsgRtpProxyNack());
			return;
		}

		if (!m.connection.is_ip_valid() || 
			!m.connection.is_port_valid())
		{

			LogWarn("ProcMaxSobolevRtpProxy::UponAllocateReq - must supply sdp with codec list");
			SendResponse(req, new MsgRtpProxyNack());
			return;
		}

		_sessionCounter++;
		_correlationCounter++;

		
		stringstream update_command;
		update_command << _correlationCounter << " U " << _sessionCounter << " " << m.connection.iptoa() << " " << m.connection.port_ho() <<  " from_tag_" << _sessionCounter;
		
		LogDebug("ProcMaxSobolevRtpProxy::UponAllocateReq  - Sending " << update_command.str());
		int res = ::send (_s,update_command.str().c_str(),update_command.str().length(),NULL);
		if (!res)
		{
			LogSysError("::send");
			SendResponse(req, new MsgRtpProxyNack());
			return;
		}

		ReqCtx ctx;
		ctx.type = REQUEST_TYPE_UPDATE;
		ctx.request = req;

		_reqMap[_correlationCounter] = ctx;



// 		
// 
// 		
// 		
// 		
// 
// 		if (!candidate)
// 		{
// 			LogWarn("ProcMaxSobolevRtpProxy::UponAllocateReq - No available rtp resource");
// 			SendResponse(req, new MsgRtpProxyNack());
// 			return;
// 		}
// 
// 		candidate->state = CONNECTION_STATE_ALLOCATED;
// 
// 		SdpParser p(req->offer.body);
// 		SdpParser::Medium m = p.first_audio_medium();
// 
// 		if (m.list.empty())
// 		{
// 			LogWarn("ProcMaxSobolevRtpProxy::UponAllocateReq - must supply sdp with codec list");
// 			SendResponse(req, new MsgRtpProxyNack());
// 			return;
// 		}
// 
// 		candidate->remote_cnx_ino = m.connection;
// 		candidate->media_format = *m.list.begin(); 
// 
// 
// 		if (m.connection.is_ip_valid() && 
// 			m.connection.is_port_valid())
// 		{
// 
// 			candidate->live_rtp_socket->changeDestinationParameters(
// 				m.connection.inaddr(),
// 				m.connection.port_ho(),225);
// 
// 			candidate->live_rtcp_socket->changeDestinationParameters(
// 				m.connection.inaddr(),
// 				m.connection.port_ho() + 1,225);
// 		}
// 
// 		DWORD time = ::GetTickCount();
// 
// 		stringstream str;
// 		str <<  "v=0\r\n"
// 			<<  "o=msrtpproxy "   << time << " " << time << " IN IP4 " << candidate->local_cnx_ino.iptoa() << "\r\n"
// 			<<  "s=msrtpproxy \r\n"
// 			<<  "c=IN IP4 " << candidate->local_cnx_ino.iptoa() << "\r\n"
// 			<<  "t=0 0\r\n"
// 			<<  "m=audio " << candidate->local_cnx_ino.port_ho() << " RTP/AVP";
// 
// 		m.append_codec_list(str) ; str << "\r\n"; 
// 		m.append_rtp_map(str) ; str << "\r\n\r\n";
// 
// 		MsgRtpProxyAck* ack = new MsgRtpProxyAck();
// 		ack->rtp_proxy_handle = candidate->connection_id;
// 		ack->offer.type = "application/sdp";
// 		ack->offer.body = str.str();
// 
// 		LogDebug("ProcMaxSobolevRtpProxy::UponAllocateReq allocated rtph:" << candidate->connection_id );
// 		SendResponse(req, ack);

	}

	void 
	ProcMaxSobolevRtpProxy::UponDeallocateReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

		shared_ptr<MsgRtpProxyDeallocateReq> req = 
			dynamic_pointer_cast<MsgRtpProxyDeallocateReq>(msg);

		// Find source.
// 		RtpConnectionsMap::iterator iter = 
// 			_connectionsMap.find(req->rtp_proxy_handle);
// 		if (iter == _connectionsMap.end())
// 		{
// 
// 			LogWarn("ProcMaxSobolevRtpProxy::UponDeallocateReq rtph:" << req->rtp_proxy_handle << " not found");
// 			SendResponse(req, new MsgRtpProxyNack());
// 			return;
// 
// 		}
// 
// 		RtpConnectionPtr conn = iter->second;
// 
// 		if (conn->state == CONNECTION_STATE_AVAILABLE)
// 		{
// 			LogWarn("ProcMaxSobolevRtpProxy::UponDeallocateReq rtph:" << req->rtp_proxy_handle << " not allocated");
// 			SendResponse(req, new MsgRtpProxyNack());
// 			return;
// 		}
// 
// 		// safe side
// 		Unbridge(conn);



// 		if (conn->rtcp_instance) 
// 		{
// 			RTCPInstance::close(conn->rtcp_instance);
// 			conn->rtcp_instance = NULL;
// 		}
// 
// 		if (conn->sink) 
// 		{
// 			Medium::close(conn->sink);
// 			conn->sink = NULL;
// 		}
// 
// 		if (conn->source)
// 		{
// 			Medium::close(conn->source);
// 			conn->source = NULL;
// 
// 		}
// 
// 		conn->state = CONNECTION_STATE_AVAILABLE;
	}

	void 
		ProcMaxSobolevRtpProxy::UponModifyReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

		shared_ptr<MsgRtpProxyModifyReq> req = 
			dynamic_pointer_cast<MsgRtpProxyModifyReq>(msg);

		// Find source.
// 		RtpConnectionsMap::iterator iter = 
// 			_connectionsMap.find(req->rtp_proxy_handle);
// 		if (iter == _connectionsMap.end())
// 		{
// 
// 			LogWarn("rtph:" << req->rtp_proxy_handle << " not found");
// 			SendResponse(req, new MsgRtpProxyNack());
// 			return;
// 
// 		}
// 
// 
// 		RtpConnectionPtr conn = iter->second;
// 
// 		SdpParser p(req->offer.body);
// 		SdpParser::Medium m = p.first_audio_medium();
// 		MediaFormat media_format = *m.list.begin();
// 
// 
// 
// 		if (media_format.get_media_type() != MediaFormat::MediaType_UNKNOWN)
// 		{
// 			conn->media_format = media_format;
// 		}
// 
// 
// 		if (m.connection.is_ip_valid() && m.connection.is_port_valid())
// 		{
// 			int port_ho = m.connection.port_ho();
// 
// 			conn->remote_cnx_ino = m.connection;
// 			conn->live_rtp_socket->changeDestinationParameters(
// 				m.connection.inaddr(),
// 				Port(port_ho),
// 				255);
// 
// 			// should be passed in message
// 			conn->live_rtcp_socket->changeDestinationParameters(
// 				m.connection.inaddr(),
// 				Port(port_ho + 1),
// 				255);
// 		}
// 
// 		SendResponse(req, new MsgRtpProxyAck());

	}


// 	ApiErrorCode
// 	ProcMaxSobolevRtpProxy::Unbridge(RtpConnectionPtr conn)
// 	{
// 		FUNCTRACKER;


// 		Find source.
// 
// 		RtpConnectionPtr source_conn;
// 		RtpConnectionPtr dest_conn;
// 
// 		switch (conn->state)
// 		{
// 		case CONNECTION_STATE_AVAILABLE:
// 		case CONNECTION_STATE_ALLOCATED:
// 			{
// 
// 				return API_SUCCESS;
// 				break;
// 			}
// 		case CONNECTION_STATE_INPUT:
// 			{
// 				source_conn = conn;
// 				dest_conn = conn->destination_conn;
// 				break;
// 			}
// 		case CONNECTION_STATE_OUTPUT:
// 			{
// 				source_conn = conn->source_conn;
// 				dest_conn = conn;
// 				break;
// 			}
// 		}
// 
// 		source_conn->source->stopGettingFrames();
// 		source_conn->state = CONNECTION_STATE_ALLOCATED;
// 		source_conn->destination_conn = RtpConnectionPtr();
// 
// 		dest_conn->sink->stopPlaying();
// 		dest_conn->state = CONNECTION_STATE_ALLOCATED;
// 		dest_conn->source_conn = RtpConnectionPtr();

// 		return API_SUCCESS;
// 
// 	}


	void 
		ProcMaxSobolevRtpProxy::UponBridgeReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

// 		shared_ptr<MsgRtpProxyBridgeReq> bridge_req = 
// 			dynamic_pointer_cast<MsgRtpProxyBridgeReq>(msg);
// 
// 
// 		// Find source.
// 		RtpConnectionsMap::iterator iter = 
// 			_connectionsMap.find(bridge_req->rtp_proxy_handle);
// 		if (iter == _connectionsMap.end())
// 		{
// 
// 			LogWarn("ProcMaxSobolevRtpProxy::UponBridgeReq - rtph:" << bridge_req->rtp_proxy_handle << " not found");
// 			SendResponse(bridge_req, new MsgRtpProxyNack());
// 			return;
// 
// 		}
// 
// 		RtpConnectionPtr source_connection = iter->second;
// 		Unbridge(source_connection);
// 
// 
// 		// Find destination
// 		iter = _connectionsMap.find(bridge_req->output_conn);
// 		if (iter == _connectionsMap.end())
// 		{
// 
// 			LogWarn("rtph:" << bridge_req->output_conn << " not found");
// 			SendResponse(bridge_req, new MsgRtpProxyNack());
// 			return;
// 
// 		}
// 
// 		RtpConnectionPtr destination_connection = iter->second;
// 
// 		if (source_connection->media_format.get_media_type()      == MediaFormat::MediaType_UNKNOWN	|| 
// 			destination_connection->media_format.get_media_type() == MediaFormat::MediaType_UNKNOWN ||
// 			destination_connection->media_format != source_connection->media_format)
// 		{
// 			LogWarn("ProcMaxSobolevRtpProxy::UponBridgeReq - transcoding not supported src rtph:"
// 				<< source_connection->connection_id << "(" << source_connection->media_format << ")" << 
// 				" <-> dst rtph:" << destination_connection->connection_id << "(" << destination_connection->media_format << ")");
// 
// 			SendResponse(bridge_req, new MsgRtpProxyNack());
// 			return;
// 		}
// 
// 		Unbridge(destination_connection);
// 
// 
// 		//
// 		// source
// 		//
// 		SimpleRTPSource *rtp_source		      = source_connection->source;
// 		RTCPInstance    *source_rtcp_instance = source_connection->rtcp_instance;
// 
// 		SimpleRTPSink	*rtp_sink			  = destination_connection->sink;
// 		RTCPInstance    *sink_rtcp_instance	  = destination_connection->rtcp_instance;
// 
// 		MediaFormat		media_format		  = source_connection->media_format;
// 
// 		{
// 			//
// 			// source
// 			//
// 			if (!rtp_source)
// 			{
// 				rtp_source = 
// 					SimpleRTPSource::createNew(
// 					*_env,											// env
// 					source_connection->live_rtp_socket.get(),		// RTPgs
// 					media_format.sdp_mapping(),			// rtpPayloadFormat
// 					media_format.sampling_rate(),		// rtpTimestampFrequency
// 					media_format.sdp_name_tos().c_str()	// mimeTypeString
// 					);
// 
// 				if (rtp_source == NULL)
// 				{
// 					LogWarn("ProcMaxSobolevRtpProxy::UponBridgeReq - Cannot create source");
// 					goto error;
// 				};
// 			}
// 
// 
// 			if (!source_rtcp_instance)
// 			{
// 				source_rtcp_instance  = 
// 					RTCPInstance::createNew(
// 					*_env,											// env	
// 					source_connection->live_rtcp_socket.get(),		// RTCPgs
// 					500,											// totSessionBW
// 					(const unsigned char *)"ivrworx",				// cname
// 					NULL,											// sink
// 					rtp_source										// source
// 					);
// 				if (source_rtcp_instance == NULL)
// 				{
// 					LogWarn("ProcMaxSobolevRtpProxy::UponBridgeReq - Cannot create source rtcp instance");
// 					goto error;
// 				};
// 			}
// 
// 
// 			//
// 			// sink
// 			//
// 			if (!rtp_sink)
// 			{
// 				rtp_sink = 
// 					SimpleRTPSink::createNew(
// 					*_env,											 // env
// 					destination_connection->live_rtp_socket.get(),  // RTPgs
// 					media_format.sdp_mapping(),			 // rtpPayloadFormat
// 					media_format.sampling_rate(),		 // rtpTimestampFrequency
// 					media_format.sdp_name_tos().c_str(), // sdpMediaTypeString
// 					media_format.sdp_name_tos().c_str()  // rtpPayloadFormatName
// 					);
// 				if (rtp_sink == NULL)
// 				{
// 					LogWarn("ProcMaxSobolevRtpProxy::UponBridgeReq - Cannot create sink instance");
// 					goto error;
// 				}
// 			}
// 
// 
// 			if (!sink_rtcp_instance)
// 			{
// 				sink_rtcp_instance  = 
// 					RTCPInstance::createNew(
// 					*_env,											 // env	
// 					destination_connection->live_rtcp_socket.get(),  // RTCPgs
// 					500,											 // totSessionBW
// 					(const unsigned char *)"ivrworx",				 // cname
// 					rtp_sink,										 // sink	
// 					NULL											 // source 	
// 					);					
// 				if (sink_rtcp_instance == NULL)
// 				{
// 					LogWarn("ProcMaxSobolevRtpProxy::UponBridgeReq - Cannot create sink instance");
// 					goto error;
// 				}
// 			}
// 
// 
// 			source_connection->state			= CONNECTION_STATE_INPUT;
// 			source_connection->source			= rtp_source;
// 			source_connection->destination_conn = destination_connection;
// 			source_connection->rtcp_instance	= source_rtcp_instance;
// 
// 			destination_connection->state			= CONNECTION_STATE_OUTPUT;
// 			destination_connection->sink			= rtp_sink;
// 			destination_connection->source_conn		= source_connection;
// 			destination_connection->rtcp_instance	= sink_rtcp_instance;
// 
// 
// 
// 			if (rtp_sink->startPlaying(*rtp_source, NULL, NULL) == FALSE)
// 			{
// 				LogWarn("ProcMaxSobolevRtpProxy::UponBridgeReq error: startPlaying");
// 				goto error;
// 			}
// 
// 			LogDebug(
// 				source_connection->remote_cnx_ino << " -> " <<
// 				source_connection->local_cnx_ino  << 
// 				" (rtph:" << source_connection->connection_id << ") ==> (" <<
// 				"rtph:" << destination_connection->connection_id << ") " <<
// 				destination_connection->local_cnx_ino << " ->" <<
// 				destination_connection->remote_cnx_ino );
// 
// 		}
// 		SendResponse(bridge_req, new MsgRtpProxyAck());
// 
// 		return;
// 
// error:
// 
// 		if (rtp_source != NULL) Medium::close(rtp_source);
// 		if (source_rtcp_instance!= NULL) RTCPInstance::close(source_rtcp_instance);
// 
// 		if (rtp_sink != NULL) Medium::close(rtp_sink);
// 		if (sink_rtcp_instance != NULL) RTCPInstance::close(sink_rtcp_instance);
// 
// 
// 		source_connection->source			= NULL;
// 		source_connection->destination_conn = RtpConnectionPtr();
// 		source_connection->rtcp_instance	= NULL;
// 
// 
// 		destination_connection->sink			= NULL;
// 		destination_connection->source_conn		= RtpConnectionPtr();
// 		destination_connection->rtcp_instance	= NULL;
// 
// 		SendResponse(bridge_req, new MsgRtpProxyNack());
// 		return;

	}


}

