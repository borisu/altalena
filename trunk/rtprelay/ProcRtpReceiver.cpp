#include "StdAfx.h"
#include "ProcRtpReceiver.h"
#include "CcuLogger.h"

#define CCU_DEFAULT_RTP_POLL_TIME 10



ProcRtpReceiver::ProcRtpReceiver(LpHandlePair pair)
:LightweightProcess(pair, 
					__FUNCTIONW__)
{
}

ProcRtpReceiver::~ProcRtpReceiver(void)
{
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


	_ioPort = ::CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		0,
		1);

	if (_ioPort == NULL)
	{
		LogSysError("::CreateIoCompletionPort");
		throw;
	}



	I_AM_READY;

	bool shutdown_flag = false;
	while (shutdown_flag == false)
	{
		RtpPacketsList packetsList;
		int overflow = 100;

		::Sleep(CCU_DEFAULT_RTP_POLL_TIME);

		if (InboundPending())
		{
			
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
					continue;
				}
			default:
				{

				}
			}
		}

#pragma  TODO( "I am sure this can be optimized for speed")

		//
		// poll all connections;
		//
		for (ConnectionSet::iterator i = _connections.begin();
			i != _connections.end(); 
			i++)
		{
			RTPConnection *source_connection = (*i);
			RTPConnection *destination = NULL;

			ConnectionBridgesMap::iterator bi = _bridges.find(source_connection);
			if (bi!=_bridges.end())
			{
				destination = (*bi).second;
			}

			packetsList.clear();
			source_connection->Poll(packetsList,overflow,true);

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

		} // for
		
	} // while

}

