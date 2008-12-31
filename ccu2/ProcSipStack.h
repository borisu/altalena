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

#pragma once

#include "ccu.h"
#include "LightweightProcess.h"
#include "UASDialogUsageManager.h"
#include "UACDialogUsageManager.h"
#include "Call.h"
#include "CcuLogger.h"


using namespace resip;



class CcuResipLogger :
	public ExternalLogger
{
public:
	virtual ~CcuResipLogger();

	/** return true to also do default logging, false to supress default logging. */
	virtual bool operator()(Log::Level level,
		const Subsystem& subsystem, 
		const Data& appName,
		const char* file,
		int line,
		const Data& message,
		const Data& messageWithHeaders);

};


typedef
SharedPtr<DialogUsageManager> DialogUsageManagerPtr;

typedef
shared_ptr<UASDialogUsageManager> UASDialogUsageManagerPtr;

typedef
shared_ptr<UACDialogUsageManager> UACDialogUsageManagerPtr;


typedef
shared_ptr<SelectInterruptor> SelectInterruptorPtr;

class ResipInterruptor
	:public Interruptor, boost::noncopyable
{

public:

	ResipInterruptor(SelectInterruptorPtr ptr);

	virtual void SignalDataIn();

	virtual void SignalDataOut();

	SelectInterruptorPtr _siPtr;

};

typedef
shared_ptr<ResipInterruptor> ResipInterruptorPtr;




class ProcSipStack : 
	public LightweightProcess
{
public:

	ProcSipStack(
		IN LpHandlePair pair, 
		IN CnxInfo data);

	virtual ~ProcSipStack(void);

	virtual void real_run();

	virtual CcuApiErrorCode Init();

	virtual void UponMakeCall(CcuMsgPtr req);

	virtual void UponHangupCall(CcuMsgPtr req);

	virtual void UponStartRegistration(CcuMsgPtr req);

	virtual void ShutDown(CcuMsgPtr req);

	virtual void UponCallOfferedAck(CcuMsgPtr req);

	virtual void ShutDown();

	CnxInfo _ipAddr;

	Time _retryTimeout;

protected:

	
	virtual unsigned int getTimeTillNextProcessMS() const;

	virtual bool ProcessCcuMessages();

	SipStack _stack;

	ResipInterruptorPtr _si;

	bool _shutDownFlag;

	UACDialogUsageManagerPtr _dumUac;

	UASDialogUsageManagerPtr _dumUas;

	CcuResipLogger _logger;

	CcuHandlesMap _ccuHandlesMap;


};










