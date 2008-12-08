#include "StdAfx.h"
#include "ProcRtpReceiver.h"
#include "CcuLogger.h"

#define CCU_DEFAULT_RTP_TIMEOUT 10000

// ccu messages come with this completion keys
// any other message indicates message handled
// by RTP connection
#define IOCP_UNIQUE_COMPLETION_KEY 555 

ProcRtpReceiver::ProcRtpReceiver(LpHandlePair pair)
:LightweightProcess(pair, 
					__FUNCTIONW__)
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
ProcRtpReceiver::AddConnection(RTPConnection *connection)
{
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

	bool shutdown_flag = false;
	while (shutdown_flag == false)
	{
		RtpPacketsList packetsList;
		int overflow = 100;

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

		if (res == FALSE)
		{
			LogSysError("::GetQueuedCompletionStatus");
			throw;
		}



		if (completion_key == IOCP_UNIQUE_COMPLETION_KEY &&
			InboundPending())
		{
#pragma region CcuMessage
			
			CcuMsgPtr message = GetInboundMessage();

			switch (message->message_id)
			{

			case CCU_MSG_RTPRECEIVER_ADD:
				{

					shared_ptr<CcuMsgRtpReceiverAdd> add_msg
						= shared_dynamic_cast<CcuMsgRtpReceiverAdd> (message);

					AddConnection(add_msg->rtp_connection);
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

				}
			} // switch 
			continue;
#pragma endregion CcuMessage
		} // if 


		RtpOverlapped *rtpOvlap			 = (RtpOverlapped *)lpOverlapped;

		RTPConnection *source_connection = rtpOvlap->ctx;
		RTPConnection *destination		 = NULL;

		ConnectionBridgesMap::iterator bi = _bridges.find(source_connection);
		if (bi!=_bridges.end())
		{
			destination = (*bi).second;
		}

		packetsList.clear();
		source_connection->Poll(packetsList,overflow,true,rtpOvlap);

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

			continue;
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


		destination->Send(packetsList, true);

		
	} // while

}

HANDLE ProcRtpReceiver::IocpHandle() const 
{ 
	return _iocpHandle; 
}