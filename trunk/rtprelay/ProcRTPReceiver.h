#pragma once
#include "LpHandle.h"
#include "LightweightProcess.h"
#include "Ccu.h"
#include "RTPConnection.h"
#include "RelayMemoryManager.h"


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
	public CcuMsgRequest
{
public:
	CcuMsgRtpReceiverAdd():
	  CcuMsgRequest(CCU_MSG_RTPRECEIVER_ADD, NAME(CCU_MSG_RTPRECEIVER_ADD)){};

    RTPConnection *rtp_connection;

};


class CcuMsgRtpReceiverRemove :
	public CcuMsgRequest
{
public:
	CcuMsgRtpReceiverRemove():
	  CcuMsgRequest(CCU_MSG_RTPRECEIVER_REMOVE, NAME(CCU_MSG_RTPRECEIVER_REMOVE)){};

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
	public CcuMsgRequest
{
public:
	CcuMsgRtpReceiverBridge():
	  CcuMsgRequest(CCU_MSG_RTPRECEIVER_BRIDGE, NAME(CCU_MSG_RTPRECEIVER_BRIDGE)){};

	  RTPConnection *rtp_connection_source;
	  RTPConnection *rtp_connection_destination;

};

class CcuMsgRtpReceiverModify :
	public CcuMsgRequest
{
public:
	CcuMsgRtpReceiverModify():
	  CcuMsgRequest(CCU_MSG_RTPRECEIVER_MODIFY, NAME(CCU_MSG_RTPRECEIVER_MODIFY)){};

	  RTPConnection *rtp_connection_source;

	  CnxInfo remote_media_data;
	  
};


class ProcRtpReceiver : 
	public LightweightProcess
{

public:

	void real_run();

	ProcRtpReceiver(
		IN LpHandlePair pair, IN RelayMemoryManager *mngr);

	void AddConnection(CcuMsgPtr ptr);

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

	void ProcessCcuMessage(
		OUT BOOL &shutdown_flag);

	void ProccessWriteCompletion(
		IN RtpOverlapped *ovlap
		);

	void ProccessReadCompletion(
		IN RtpOverlapped *ovlap
		);

	HANDLE IocpHandle() const;

	virtual ~ProcRtpReceiver(void);

private:

	RelayMemoryManager* GetMemoryManager();

	ConnectionBridgesMap _bridges;

	ConnectionSet _connections;

	HaveToLogConnectionSet _haveToLogSet;

	HANDLE _iocpHandle;

	RelayMemoryManager *_mngr;
	
	
};
