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
#include "Ccu.h"
#include "ProcAudio.h"
#include "ProcRtpReceiver.h"
#include "RtpConnection.h"
#include "CcuLogger.h"
#include "ProcPipeIpcDispatcher.h"
#include "RtpRelay.h"

#define CCU_DEFAULT_SAMPLING_RATE	8000
#define CCU_UNDEFINED_PORT_SLOT		-1
#define CCU_RTP_PORT_RANGE_START	6000
#define CCU_RTP_PORT_RANGE_END		6010


ProcAudio::ProcAudio(IN LpHandlePair pair, IN CcuMediaData data)
:LightweightProcess(pair,
					RTP_RELAY_Q,
					__FUNCTIONW__),
_receiverInbound(new LpHandle()),
_receiverOutbound(new LpHandle()),
_mediaData(data),
_portManager(CCU_RTP_PORT_RANGE_END,CCU_RTP_PORT_RANGE_START)
{
	
	FUNCTRACKER;
}

void
ProcAudio::real_run()
{
	FUNCTRACKER;

	WSADATA dat;
	if (WSAStartup(MAKEWORD(2,2),&dat)!=0)
	{
		LogSysError("Cannot initiate WIN sockets");
		throw;
	}


	START_FORKING_REGION;



	DECLARE_NAMED_HANDLE_PAIR(ipc_pair);
	FORK(new ProcPipeIPCDispatcher(ipc_pair,RTP_RELAY_Q));
	if (CCU_FAILURE(WaitTillReady(Seconds(5), ipc_pair)))
	{
		return;
	}



	DECLARE_NAMED_HANDLE_PAIR(receiver_pair);
	_receiverInbound  = receiver_pair.inbound;
	_receiverOutbound = receiver_pair.outbound;

	FORK(new ProcRtpReceiver(receiver_pair));
	if (CCU_FAILURE(WaitTillReady(Seconds(5), receiver_pair)))
	{
		return;
	}


	I_AM_READY;

	CcuApiErrorCode err_code = CCU_API_SUCCESS;
	BOOL shutdownFlag = FALSE;
	CcuMsgPtr shutdown_req = CCU_NULL_MSG;
	while (shutdownFlag  == FALSE)
	{
		CcuMsgPtr ptr =  _inbound->WaitForMessages(
			Seconds(60),
			err_code,
			SL(
			CCU_MSG_ALLOCATE_NEW_CONNECTION_REQ,
			CCU_MSG_BRIDGE_CONNECTIONS_REQ,
			CCU_CLOSE_CONNECTION_REQ,
			CCU_MSG_PROC_SHUTDOWN_REQ,
			CCU_MSG_MODIFY_CONNECTION_REQ));

		
		if (err_code == CCU_API_TIMEOUT)
		{
			LogDebug("Timeout = Keep Alive");
			continue;
		}

	
		switch (ptr->message_id)
		{
		case CCU_MSG_ALLOCATE_NEW_CONNECTION_REQ:
			{
				AllocateAudioConnection(ptr);
				break;
			}
		case CCU_MSG_BRIDGE_CONNECTIONS_REQ:
			{
				BridgeConnections(ptr);
				break;
			}
		case CCU_CLOSE_CONNECTION_REQ:
			{
				CloseAudioConnection(ptr, forking);
				break;
			}
		case CCU_MSG_MODIFY_CONNECTION_REQ:
			{
				ModifyConnection(ptr);
				break;
			}
		case CCU_MSG_PROC_SHUTDOWN_REQ:
			{
#pragma TODO ("Handle Shutdown Correctly")
				shutdownFlag = TRUE;
				shutdown_req = ptr;
				continue;
			}
		default:
			{
				LogWarn("Received OOB message id=[" << ptr->message_id << "]");
				_ASSERT	(false);
			}

		}
	}


	
	Shutdown(Time(Seconds(5)),receiver_pair);

	if (shutdown_req != CCU_NULL_MSG)
	{
		SendResponse(shutdown_req, new CcuMsgShutdownAck());
	}
	
	END_FORKING_REGION;

	WSACleanup();

}

void
ProcAudio::ModifyConnection(IN CcuMsgPtr ptr)
{
	FUNCTRACKER;

	shared_ptr<CcuMsgModifyConnectionReq> request =
		shared_dynamic_cast<CcuMsgModifyConnectionReq> (ptr);

	RTPConnectionMap::iterator iter = _connMap.find(request->connection_id);
	if ( iter == _connMap.end())
	{
		SendResponse(ptr, 
			new CcuMsgNack());
		return;
	}

	CcuMsgRtpReceiverModify *msg = 
		new CcuMsgRtpReceiverModify();

	msg->rtp_connection_source = (*iter).second;
	msg->remote_media_data = request->remote_media_data;
	_receiverInbound->Send(msg);

	SendResponse(ptr,
		new CcuMsgAck());
}

void 
ProcAudio::CloseAudioConnection(CcuMsgPtr ptr, ScopedForking &forking)
{
	FUNCTRACKER;

	shared_ptr<CcuMsgRtpCloseConnectionReq> request =
		shared_dynamic_cast<CcuMsgRtpCloseConnectionReq> (ptr);

	RTPConnectionMap::iterator iter = _connMap.find(request->connection_id);
	if ( iter == _connMap.end())
	{
		SendResponse(ptr, 
			new CcuMsgNack());
		return;
	}
		
	RTPConnection *conn = (*iter).second;
	_connMap.erase(iter);

	// we have to receive ack on rtp remove in order
	// to reclaim port but we cannot wait for response
	// that's why we spawn another project
	//
	DECLARE_NAMED_HANDLE_PAIR(remover_pair);
	forking.forkInThisThread(
		new ProcRTPConnectionRemover(
		remover_pair,
		_receiverInbound,
		conn,
		_portManager));

	SendResponse(ptr, 
		new CcuMsgAck());
	
}

void 
ProcAudio::BridgeConnections(CcuMsgPtr ptr)
{
	FUNCTRACKER;

	shared_ptr<CcuMsgRtpBridgeConnectionsReq> request =
		shared_dynamic_cast<CcuMsgRtpBridgeConnectionsReq> (ptr);

	if (request->connection_id1 == 
		request->connection_id2)
	{
		LogWarn(">>Cannot<< bridge connection to itself, conn1=[" 
			<< request->connection_id1 <<"], conn2=[" << request->connection_id2 << "].");

		SendResponse(ptr, 
			new CcuMsgRtpBridgeConnectionsNack());

		return;

	}

	if (_connMap.find(request->connection_id1) == _connMap.end() || 
		_connMap.find(request->connection_id2) == _connMap.end())
	{
		
		LogWarn("One of the bridged connection was not found conn1=[" 
			<< request->connection_id1 <<"], conn2=[" << request->connection_id2 << "].");

		SendResponse(ptr, 
			new CcuMsgRtpBridgeConnectionsNack());

		return;
	}

	RTPConnection *conn1 = _connMap[request->connection_id1];
	RTPConnection *conn2 = _connMap[request->connection_id2];

	CcuMsgRtpReceiverBridge	*msg = 
		new CcuMsgRtpReceiverBridge();

	msg->rtp_connection_source = conn1;
	msg->rtp_connection_destination = conn2;

	_receiverInbound->Send(msg);

	LogDebug(">>Bridged<< Full-Duplex, new connection1=[" << conn1->ConnectionId() << "] " << 
		"and connection1=[" << conn2->ConnectionId() << "]  ");

#pragma TODO ("Log correctly more than one destination")

	CcuMediaData remote1;
	if (!conn1->DestinationsList().empty())
	{
		remote1 =*conn1->DestinationsList().begin();
	}

	CcuMediaData remote2;
	if (!conn2->DestinationsList().empty())
	{
		remote2 =*conn2->DestinationsList().begin();
	}

	 
	LogDebug("Data Flow " << remote1 << " <-network-> [local:" << conn1->Port()<<  "<== Rtp Relay ==>" << "local:" << conn2->Port()<<  "] <- network ->" << remote2);

	SendResponse(ptr, 
		new CcuMsgRtpBridgeConnectionsAck());

}


void
ProcAudio::AllocateAudioConnection(CcuMsgPtr ptr)
{
	FUNCTRACKER;

	
	//
	// find new port slot
	// 
	_portManager.BeginSearch();
	int curr_port_slot_candidate = _portManager.GetNextCandidate();

	shared_ptr<CcuMsgRtpAllocateNewConnectionReq> ac_msg =
		shared_dynamic_cast<CcuMsgRtpAllocateNewConnectionReq>(ptr);

	RTPConnection *conn = NULL;
	while(curr_port_slot_candidate != CCU_UNDEFINED)
	{
			
		conn = 	new RTPConnection(curr_port_slot_candidate);

		if (conn->Init() == CCU_API_SUCCESS )
		{
			if (ac_msg->remote_end.IsValid() &&
				conn->SetDestination(ac_msg->remote_end.ip_addr,ac_msg->remote_end.port ) != CCU_API_SUCCESS)
			{

				LogWarn(" Error adding destination " << ac_msg->remote_end);

				delete conn;
				conn = NULL;
				break;

			}

			_portManager.MarkUnavailable(curr_port_slot_candidate);
			break;
		}


		if (conn!=NULL)
		{
			delete conn;
			conn = NULL;
		}
		
	} 

	if (conn == NULL)
	{
		SendResponse(ptr, 
			new CcuMsgRtpAllocateConnectionNack());
		return;
	}

	_connMap[conn->ConnectionId()] = conn;

   CcuMsgRtpReceiverAdd	*msg = 
	   new CcuMsgRtpReceiverAdd();
   msg->rtp_connection = conn;

	_receiverInbound->Send(msg);

	LogDebug(">>Allocated<< new connection id=[" << conn->ConnectionId() 
		<< "] remote_end=[" << ac_msg->remote_end << "]");

	CcuMsgRtpAllocateConnectionAck 
		*success = new CcuMsgRtpAllocateConnectionAck();

	success->connection_id = conn->ConnectionId();
	success->connection_media = CcuMediaData(
		_mediaData.ip_addr,
		curr_port_slot_candidate);

	SendResponse(ptr, success);

}

ProcAudio::~ProcAudio(void)
{
	FUNCTRACKER;
	
}

ProcRTPConnectionRemover::ProcRTPConnectionRemover(
	IN LpHandlePair pair, 
	IN LpHandlePtr receiver_handle,
	IN RTPConnection *conn,
	IN PortManager &ports_map
	):
LightweightProcess(pair,__FUNCTIONW__ ),
_receiverHandle(receiver_handle),
_portsManager(ports_map),
_rtpConn(conn)
{

}

ProcRTPConnectionRemover::~ProcRTPConnectionRemover()
{
	
}

void 
ProcRTPConnectionRemover::real_run()
{
	
	int port = _rtpConn->Port();

	CcuMsgRtpReceiverRemove *msg = 
		new CcuMsgRtpReceiverRemove();

	msg->rtp_connection = _rtpConn;

	CcuMsgPtr response_ptr;
	EventsSet responses;
	responses.insert(CCU_MSG_RTPRECEIVER_REMOVE_ACK);

	CcuApiErrorCode res = DoRequestResponseTransaction(
		_receiverHandle,
		CcuMsgPtr(msg),
		responses,
		response_ptr,
		Seconds(60),
		L"Close RTP Connection TXN"
		);

	if (res!=CCU_API_SUCCESS)
	{
		LogCrit("Couldn't delete RTP object in timely fashion - RTP should be restarted!!!");
		throw;
	}

	delete _rtpConn;

	_portsManager.MarkAvailable(port); 


}
