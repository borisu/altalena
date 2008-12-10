#include "StdAfx.h"
#include "ProcRtpReceiver.h"
#include "CcuLogger.h"

#define CCU_DEFAULT_RTP_TIMEOUT 10000

// ccu messages come with this completion keys
// any other message indicates message handled
// by RTP connection
#define IOCP_UNIQUE_COMPLETION_KEY 555 

ProcRtpReceiver::ProcRtpReceiver(LpHandlePair pair, RelayMemoryManager *mngr)
:LightweightProcess(pair, 
					__FUNCTIONW__),
_mngr(mngr)
{

	_iocpHandle = ::CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		0,
		1);

	IocpInterruptorPtr iocpPtr = 
		IocpInterruptorPtr(new IocpInterruptor(_iocpHandle,IOCP_UNIQUE_COMPLETION_KEY));

	_inbound->HandleInterruptor(iocpPtr);

	if (_iocpHandle == NULL)
	{
		LogSysError("::CreateIoCompletionPort");
		throw;
	}

}

ProcRtpReceiver::~ProcRtpReceiver(void)
{
	::CloseHandle(_iocpHandle);
}

void
ProcRtpReceiver::AddConnection(CcuMsgPtr message)
{
	
	FUNCTRACKER;

	shared_ptr<CcuMsgRtpReceiverAdd> add_msg
		= shared_dynamic_cast<CcuMsgRtpReceiverAdd> (message);

	RTPConnection *connection = add_msg->rtp_connection;

	CcuApiErrorCode res = connection->IssueAsyncIoReq(TRUE);
	if (CCU_FAILURE(res))
	{
		LogWarn(">>Cannot Issue<< the I/O request for RTP - the connection will stay idle.");
		return;
	}

	res = connection->IssueAsyncIoReq(FALSE);
	if (CCU_FAILURE(res))
	{
		LogWarn(">>Cannot Issue<< the I/O request for RTCP - the connection will stay idle.");
		return;
	}

	_connections.insert(connection);
	_haveToLogSet.insert(connection);

}

void
ProcRtpReceiver::RemoveConnection(RTPConnection *connection)
{
	RemoveFromCollections(connection);
}

void
ProcRtpReceiver::RemoveFromCollections(RTPConnection *connection)
{
	
	// remove connection from main collection
	_connections.erase(connection);
	
	
	// remove its bridge connection
	_bridges.erase(connection);
	

	// if we had to log it remove it also
	_haveToLogSet.erase(connection);

}

void
ProcRtpReceiver::BridgeConnections(RTPConnection *connection_source,
								   RTPConnection *connection_destination)
{
	
	//
	// full duplex
	//
	_bridges[connection_source] = connection_destination;
	_bridges[connection_destination] = connection_source;

	_haveToLogSet.insert(connection_source);
	_haveToLogSet.insert(connection_destination);

}

void
ProcRtpReceiver::ModifyConnection(IN RTPConnection *connection, 
								  IN CnxInfo &media_data)
{
	_haveToLogSet.insert(connection);
	connection->SetDestination(media_data);
}

void
ProcRtpReceiver::real_run()
{
	
	BOOL os_res = ::SetThreadPriority(::GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	if (os_res == FALSE)
	{
		LogSysError("::SetThreadPriority");
		throw;
	}


	I_AM_READY;

	BOOL shutdown_flag = FALSE;
	while (shutdown_flag == FALSE)
	{
		
		DWORD number_of_bytes    = 0;
		ULONG_PTR completion_key = 0;
		LPOVERLAPPED lpOverlapped = NULL;

		BOOL res = ::GetQueuedCompletionStatus(
			_iocpHandle,		// A handle to the completion port. To create a completion port, use the CreateIoCompletionPort function.
			&number_of_bytes,	// A pointer to a variable that receives the number of bytes transferred during an I/O operation that has completed.
			&completion_key,	// A pointer to a variable that receives the completion key value associated with the file handle whose I/O operation has completed. A completion key is a per-file key that is specified in a call to CreateIoCompletionPort.
			&lpOverlapped,		// A pointer to a variable that receives the address of the OVERLAPPED structure that was specified when the completed I/O operation was started. 
			CCU_DEFAULT_RTP_TIMEOUT // The number of milliseconds that the caller is willing to wait for a completion packet to appear at the completion port. If a completion packet does not appear within the specified time, the function times out, returns FALSE, and sets *lpOverlapped to NULL.
			);

		int err = ::GetLastError() ;
		if (res == FALSE && err != WAIT_TIMEOUT)
		{
			LogSysError("::GetQueuedCompletionStatus");
			throw;
		} 
		else if (err == WAIT_TIMEOUT)
		{
			LogDebug(">>Keep Alive<<");
			continue;
		}

		if (completion_key == IOCP_UNIQUE_COMPLETION_KEY)
		{
			ProcessCcuMessage(shutdown_flag);
			continue;
		} // if 


		RtpOverlapped *rtpOvlap	= (RtpOverlapped *)lpOverlapped;

		switch (rtpOvlap->opcode)
		{
		case OPCODE_WRITE:
			{
				ProccessWriteCompletion(rtpOvlap);
				break;
			}
		case OPCODE_READ:
			{
				ProccessReadCompletion(rtpOvlap);
				break;
			}
		default:
			{
				LogCrit("Unknown I/O operation");
				throw;
			}
		}
	
	} // while
}

void
ProcRtpReceiver::ProcessCcuMessage(OUT BOOL &shutdown_flag)
{
	if (!InboundPending())
	{
		return;
	}
	
	CcuMsgPtr message = GetInboundMessage();

	switch (message->message_id)
	{

	case CCU_MSG_RTPRECEIVER_ADD:
		{

			AddConnection(message);
			break;

		}
	case CCU_MSG_RTPRECEIVER_REMOVE:
		{
			shared_ptr<CcuMsgRtpReceiverRemove> add_msg
				= shared_dynamic_cast<CcuMsgRtpReceiverRemove> (message);

			RemoveConnection(add_msg->rtp_connection);
			SendResponse(add_msg, new CcuMsgRtpReceiverRemoveAck());
			break;

		}
	case CCU_MSG_RTPRECEIVER_BRIDGE:
		{
			shared_ptr<CcuMsgRtpReceiverBridge> add_msg
				= shared_dynamic_cast<CcuMsgRtpReceiverBridge> (message);

			BridgeConnections(
				add_msg->rtp_connection_source,
				add_msg->rtp_connection_destination);
			break;

		}
	case CCU_MSG_RTPRECEIVER_MODIFY:
		{
			shared_ptr<CcuMsgRtpReceiverModify> add_msg
				= shared_dynamic_cast<CcuMsgRtpReceiverModify> (message);

			ModifyConnection(
				add_msg->rtp_connection_source,
				add_msg->remote_media_data);

			break;

		}
	case CCU_MSG_PROC_SHUTDOWN_REQ:
		{
			shutdown_flag = true;
			SendResponse(message,new CcuMsgShutdownAck());
		}
	default:
		{
			BOOL res = HandleOOBMessage(message);
			if (res == FALSE)
			{
				LogCrit("OOB message");
				throw;
			}

		}
	} // switch 

}

void 
ProcRtpReceiver::ProccessWriteCompletion(RtpOverlapped *ovlap)
{
	//
	// delete all buffers
	//
	RTPDelete(ovlap->send_buf->buf, GetMemoryManager());
	RTPDelete(ovlap->send_buf,GetMemoryManager());
	RTPDelete(ovlap,GetMemoryManager());
}

RelayMemoryManager*
ProcRtpReceiver::GetMemoryManager()
{
	return _mngr;
}

void 
ProcRtpReceiver::ProccessReadCompletion(RtpOverlapped *ovlap)
{
	RtpPacketsList packetsList;
	int overflow = 100;

	RTPConnection *source_connection = ovlap->ctx;


	//
	// Don't poll connections that are not in collection.
	// As it might be they were deleted while their I/O 
	// request was waiting in queue
	//
	if (_connections.find(source_connection) == _connections.end())
	{
		// For reading unlike writing we use static buffer 
		// which should be released when deleting CcuRtpConnection.
		//
		RTPDelete(ovlap,GetMemoryManager());
		return;
	}


	RTPConnection *destination = NULL;

	ConnectionBridgesMap::iterator bi = _bridges.find(source_connection);
	if (bi!=_bridges.end())
	{
		destination = (*bi).second;
	}


	CcuApiErrorCode res = 
		source_connection->Poll(packetsList,overflow,true,ovlap);
	RTPDelete(ovlap, GetMemoryManager());

	if (CCU_FAILURE(res))
	{
#pragma TODO ("Clean up all packets")
		LogWarn(">>Cannot complete reading<< from conn=[" << source_connection->GetObjectUid() << "]");
		return;
	}


	bool have_to_log =	
		// we still didn't log first packet
		(_haveToLogSet.find(source_connection) != _haveToLogSet.end()) && 
		// and we have something to log
		(!packetsList.empty());

	if (have_to_log)
	{
		_haveToLogSet.erase(source_connection);
	}


	if (destination == NULL)
	{

		if (have_to_log)
		{
			//
			// log packet packet which arrived to nowhere...
			//
			RtpReceiveMsg &msg_to_log = **packetsList.begin();


			CnxInfo remote_dest; 
			if (!source_connection->DestinationsList().empty())
			{
				remote_dest = (source_connection->DestinationsList()).front();
			}

			LogDebug("New RTP packet >>arrived<< at NON-BRIDGED connection" 
				"id=[" << source_connection->GetObjectUid() << "]," 
				"from=[" << remote_dest.ipporttows() << "],"
				"at local_port=[" << source_connection->Port() << "]," 
				"ssrc=[" << msg_to_log.source_id << "],"
				"timestamp=[" << msg_to_log.timestamp_unit << "]");

		}

	}


	if (destination == NULL || 
		packetsList.empty())
	{

		return;
	}

	//
	// log packet which arrived to bridged cnxn...
	//
	if (have_to_log)
	{
		RtpReceiveMsg &msg_to_log = **packetsList.begin();

		CnxInfo remote_dest; 
		if (!destination->DestinationsList().empty())
		{
			remote_dest = destination->DestinationsList().front();
		}

		LogDebug("New RTP packet >>arrived<< at BRIDGED connection" 
			"from=[" << remote_dest.ipporttows() << "],"
			"id=[" << source_connection->GetObjectUid() << "]," 
			"at local_port=[" << source_connection->Port() << "]," 
			"ssrc=[" << msg_to_log.source_id << "],"
			"timestamp=[" << msg_to_log.timestamp_unit << "]," 
			"and ===> to conn=[" << destination->GetObjectUid()<< "],"
			"to remote_dest=[" << remote_dest.ipporttows() << "],");

	}

	destination->AsyncRelayRtpPacket(packetsList, true);
}

HANDLE ProcRtpReceiver::IocpHandle() const 
{ 
	return _iocpHandle; 
}