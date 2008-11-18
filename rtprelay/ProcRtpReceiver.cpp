#include "StdAfx.h"
#include "ProcRtpReceiver.h"

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

	counterpart_connection = NULL;
	ConnectionMap::iterator j = _bridges.find(connection);
		
	if (j != _bridges.end())
	{
		_bridges.erase(j);
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

}

void
ProcRtpReceiver::ModifyConnection(IN RTPConnection *connection, 
								  IN CcuMediaData &media_data)
{
	connection->SetDestination(media_data.ip_addr,media_data.port);
}




void
ProcRtpReceiver::real_run()
{
	
	_outbound->Send(new CcuMsgProcReady());

	bool shutdown_flag = false;
	while (shutdown_flag == false)
	{
		RtpPacketsList packetsList;
		int overflow = 100;

#pragma message ("Where is the hell is SLEEP ????????!!!!!!!!!!!")
	
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

		//
		// poll all connections;
		//
		for (ConnectionSet::iterator i = _connections.begin();
			i != _connections.end(); 
			i++)
		{
			RTPConnection *source = (*i);
			RTPConnection *destination = NULL;

			ConnectionMap::iterator bi = _bridges.find(source);
			if (bi!=_bridges.end())
			{
				destination = (*bi).second;
			}

			packetsList.clear();
			source->Poll(packetsList,overflow);
			if (destination == NULL || 
				packetsList.size() == 0)
			{
				continue;
			}

			destination->Send(packetsList);

		} // for
		
	} // while

}

