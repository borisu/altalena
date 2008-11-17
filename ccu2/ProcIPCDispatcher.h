#pragma once
#include "Ccu.h"
#include "LpHandle.h"
#include "LightweightProcess.h"
#include "boost\static_assert.hpp"
#include "CcuLogger.h"

#define CCU_MAX_BODY_LEN			1024

#define CCU_MAX_NUMER_OF_PROPERTIES 10

typedef std::map<CcuProcId,LPWSTR> QueueDictionary;

typedef std::map<wstring,HANDLE> MSMQHandleCache;


class ProcIPCDispatcher : 
	public LightweightProcess,
	public InterruptibleBySemaphore
{
public:

	ProcIPCDispatcher(
		LpHandlePtr inbound,
		LpHandlePtr outbound,
		CcuProcId qId);

	ProcIPCDispatcher(void);

	virtual ~ProcIPCDispatcher(void);

	
	virtual void real_run(void);

private:

	virtual HRESULT CreateMSMQQueue(
		LPWSTR wszPathName,
		HANDLE *hQueue
		);

	HRESULT LocateMSMQQueue(
		LPWSTR wszPathName,
		DWORD  accessMask,
		HANDLE *hQueue);

	HRESULT SendMSMQMessage(
		CcuMsgPtr msg);

	CcuApiErrorCode EnableAsyncRead();

	void HandleMsgFromMSMQ();

	void HandleMsgLocal();

	CcuProcId _qId;

	HANDLE _qHandle;

	HANDLE _hRecv;

	QueueDictionary _qDict;

	MSMQHandleCache _msmqHandleCache;
	
	LPWSTR _qName;

	friend class ProcMSMQReceiver;

	//
	// MSMQ Property Structure
	//
	static const int NUMBEROFPROPERTIES = CCU_MAX_NUMER_OF_PROPERTIES;

	MQMSGPROPS msgprops;
	MSGPROPID aMsgPropId[NUMBEROFPROPERTIES];
	MQPROPVARIANT aMsgPropVar[NUMBEROFPROPERTIES];
	HRESULT aMsgStatus[NUMBEROFPROPERTIES];

	WCHAR wszLabelBuffer[MQ_MAX_MSG_LABEL_LEN];
	WCHAR wszReceiveBodyBuffer[CCU_MAX_BODY_LEN]; 

	DWORD messageIdArrayIndex;


	
};

