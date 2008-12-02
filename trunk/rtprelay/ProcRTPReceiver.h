#pragma once
#include "LpHandle.h"
#include "LightweightProcess.h"
#include "Ccu.h"
#include "RTPConnection.h"


enum RtpReceiverEvents
{
	CCU_MSG_RTPRECEIVER_ADD = CCU_MSG_USER_DEFINED, 
	CCU_MSG_RTPRECEIVER_REMOVE,	
	CCU_MSG_RTPRECEIVER_BRIDGE,
	CCU_MSG_RTPRECEIVER_MODIFY,	
	CCU_MSG_RTPRECEIVER_REMOVE_ACK
};


typedef 
std::set<RTPConnection*> ConnectionSet;

typedef 
std::map<RTPConnection*,RTPConnection*> ConnectionBridgesMap;

typedef 
std::set<RTPConnection*> HaveToLogConnectionSet;



//
// RTP RELATED
//
class CcuMsgRtpReceiverAdd :
	public CcuMessage
{
public:
	CcuMsgRtpReceiverAdd():
	  CcuMessage(CCU_MSG_RTPRECEIVER_ADD, NAME(CCU_MSG_RTPRECEIVER_ADD)){};

    RTPConnection *rtp_connection;

};


class CcuMsgRtpReceiverRemove :
	public CcuMessage
{
public:
	CcuMsgRtpReceiverRemove():
	  CcuMessage(CCU_MSG_RTPRECEIVER_REMOVE, NAME(CCU_MSG_RTPRECEIVER_REMOVE)){};

	  RTPConnection *rtp_connection;

};

class CcuMsgRtpReceiverRemoveAck :
	public CcuMessage
{
public:
	CcuMsgRtpReceiverRemoveAck():
	  CcuMessage(CCU_MSG_RTPRECEIVER_REMOVE_ACK, NAME(CCU_MSG_RTPRECEIVER_REMOVE_ACK)){};

};


class CcuMsgRtpReceiverBridge :
	public CcuMessage
{
public:
	CcuMsgRtpReceiverBridge():
	  CcuMessage(CCU_MSG_RTPRECEIVER_BRIDGE, NAME(CCU_MSG_RTPRECEIVER_BRIDGE)){};

	  RTPConnection *rtp_connection_source;
	  RTPConnection *rtp_connection_destination;

};

class CcuMsgRtpReceiverModify :
	public CcuMessage
{
public:
	CcuMsgRtpReceiverModify():
	  CcuMessage(CCU_MSG_RTPRECEIVER_MODIFY, NAME(CCU_MSG_RTPRECEIVER_MODIFY)){};

	  RTPConnection *rtp_connection_source;

	  CnxInfo remote_media_data;
	  
};


class ProcRtpReceiver : 
	public LightweightProcess
{

public:

	void real_run();

	ProcRtpReceiver(
		IN LpHandlePair pair);

	void AddConnection(
		IN RTPConnection *connection);

	void ModifyConnection(
		IN RTPConnection *connection, 
		IN CnxInfo &media_data);

	void RemoveFromCollections(
		IN RTPConnection *connection);

	void RemoveConnection(
		IN RTPConnection *connection);

	void BridgeConnections(
		IN RTPConnection *connection_source,
		IN RTPConnection *connection_destination);

	virtual ~ProcRtpReceiver(void);

private:

	ConnectionBridgesMap _bridges;

	ConnectionSet _connections;

	HaveToLogConnectionSet _haveToLogSet;

	HANDLE _ioPort;

};
