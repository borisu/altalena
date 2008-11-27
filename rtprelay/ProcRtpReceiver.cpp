#include "StdAfx.h"
#include "ProcRtpReceiver.h"
#include "CcuLogger.h"

#define CCU_DEFAULT_RTP_POLL_TIME 20



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
	ConnectionSet::iterator i = 
		_connections.find(connection);
	if (i != _connections.end())
	{
		_connections.erase(i);
	}
	
	RTPConnection *counterpart_connection= NULL;

	ConnectionBridgesMap::iterator j = 
		_bridges.find(connection);
	if (j != _bridges.end())
	{
		_bridges.erase(j);
	}

	HaveToLogConnectionSet::iterator k = 
		_haveToLogSet.find(connection);

	if (k != _haveToLogSet.end())
	{
		_haveToLogSet.erase(k);
	}


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
								  IN CcuMediaData &media_data)
{
	_haveToLogSet.insert(connection);
	connection->SetDestination(media_data);
}




void
ProcRtpReceiver::real_run()
{
	
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
#pragma message ("TODO: add_msg?")
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
			RTPConnection *source = (*i);
			RTPConnection *destination = NULL;

			ConnectionBridgesMap::iterator bi = _bridges.find(source);
			if (bi!=_bridges.end())
			{
				destination = (*bi).second;
			}

			packetsList.clear();
			source->Poll(packetsList,overflow);

			bool have_to_log =	
				// we still didn't log first packet
				(_haveToLogSet.find(source) != _haveToLogSet.end()) && 
				// and we have something to log
				(!packetsList.empty());

			if (have_to_log)
			{
				_haveToLogSet.erase(source);
			}

			
			if (destination == NULL)
			{
				
				if (have_to_log)
				{
					//
					// log packet packet which arrived to nowhere...
					//
					RtpReceiveMsg msg_to_log = *packetsList.begin();


					CcuMediaData remote_dest; 
					if (!source->DestinationsList().empty())
					{
						remote_dest = (source->DestinationsList()).front();
					}

					LogDebug("New RTP packet >>arrived<< at NON-BRIDGED connection" 
						"id=[" << source->GetObjectUid() << "]," 
						"from=[" << remote_dest.ipporttos() << "],"
						"at local_port=[" << source->Port() << "]," 
						"ssrc=[" << msg_to_log.sourceID << "],"
						"timestamp=[" << msg_to_log.timestampUnit << "]");

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
				RtpReceiveMsg msg_to_log = *packetsList.begin();

				CcuMediaData remote_dest; 
				if (!destination->DestinationsList().empty())
				{
					remote_dest = source->DestinationsList().front();
				}
				
				LogDebug("New RTP packet >>arrived<< at BRIDGED connection" 
					"from=[" << remote_dest.ipporttos() << "],"
					"id=[" << source->GetObjectUid() << "]," 
					"at local_port=[" << source->Port() << "]," 
					"ssrc=[" << msg_to_log.sourceID << "],"
					"timestamp=[" << msg_to_log.timestampUnit << "]," 
					"and ===> to conn=[" << destination->GetObjectUid()<< "],"
					"to remote_dest=[" << remote_dest.ipporttos() << "],");

			}
			
			
			destination->Send(packetsList);

		} // for
		
	} // while

}

