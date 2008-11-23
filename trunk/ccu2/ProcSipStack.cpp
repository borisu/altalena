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
#include "ProcSipStack.h"

#include "UASAppDialogSetFactory.h"
#include "UASShutdownHandler.h"

#include "UACUserProfile.h"
#include "UACSessionHandler.h"
#include "UACAppDialogSet.h"
#include "UACDialogUsageManager.h"
#include "UASDialogUsageManager.h"




using namespace resip;
using namespace std;

ResipInterruptor::ResipInterruptor(SelectInterruptorPtr ptr):
_siPtr(ptr)
{

}

void 
ResipInterruptor::SignalDataIn()
{
	_siPtr->interrupt();
}

void 
ResipInterruptor::SignalDataOut()
{

}

ProcSipStack::ProcSipStack(IN LpHandlePair pair, 
						   IN CcuMediaData data):
LightweightProcess(pair,__FUNCTIONW__),
_shutDownFlag(false),
_callHandlerCreator(ICallHandlerCreatorPtr((ICallHandlerCreator*)NULL)),
_ipAddr(data)
{
	FUNCTRACKER;
	Log::initialize(Log::Cout, Log::Info, NULL, _logger);
	LogDebug("Stack will use default UAS definitions");

}

ProcSipStack::ProcSipStack(IN LpHandlePair pair, 
						   IN CcuMediaData data,
						   IN ICallHandlerCreatorPtr handler_creator):
LightweightProcess(pair,__FUNCTIONW__),
_shutDownFlag(false),
_callHandlerCreator(handler_creator),
_ipAddr(data)
{
	FUNCTRACKER;
	Log::initialize(Log::Cout, Log::Info, NULL, _logger);


}

ProcSipStack::~ProcSipStack(void)
{
	FUNCTRACKER;
	ShutDown();
}


CcuApiErrorCode
ProcSipStack::Init()
{
	FUNCTRACKER;

	try 
	{
		//
		// UAS
		// All messages are sent to stack outbound channel
		//
		_dumUas = UASDialogUsageManagerPtr(new UASDialogUsageManager(
			_stack,
			_ipAddr,
			_callHandlerCreator,
			_ccuHandlesMap,
			*this));
		
		//
		// UAC
		//
		_dumUac = UACDialogUsageManagerPtr(new UACDialogUsageManager(
			_stack,
			_ipAddr,
			_ccuHandlesMap,
			*this));

		_si = ResipInterruptorPtr(
					new ResipInterruptor( 
					SelectInterruptorPtr(new SelectInterruptor())));

		_inbound->HandleInterruptor(shared_dynamic_cast<Interruptor>(_si));
		_outbound->Send(new CcuMsgProcReady());

		return CCU_API_SUCCESS;

	}
	catch (BaseException& e)
	{
		LogWarn("Caught >>exception<< while starting msg=[" << e.getMessage() << "]");
		return CCU_API_FAILURE;
	}

}

void 
ProcSipStack::UponHangupCall(CcuMsgPtr ptr)
{
	FUNCTRACKER;

	shared_ptr<CcuMsgHangupCallRequest> hangup_msg = 
		dynamic_pointer_cast<CcuMsgHangupCallRequest>(ptr);

	CcuStackHandle handle = hangup_msg->stack_call_handle;

	CcuHandlesMap::iterator iter = _ccuHandlesMap.find(handle);
	if (iter == _ccuHandlesMap.end())
	{
		return;
	}

	SipDialogContextPtr ctx_ptr = (*iter).second;

	if (ctx_ptr->transaction_type = CCU_UAC)
	{
		_dumUac->HangupCall(ctx_ptr);
	} 
	else 
	{
		_dumUas->HangupCall(ctx_ptr);
	}

	return;
}

void
ProcSipStack::ShutDown()
{
	FUNCTRACKER;

	if (_shutDownFlag)
	{
		return;
	}

	_shutDownFlag = true;

	if (_dumUas)
	{
		_dumUac->Shutdown();
	}
	if (_dumUas)
	{
		_dumUas->forceShutdown(NULL);
	}
	
	_stack.shutdown();
}

void
ProcSipStack::ShutDown(CcuMsgPtr req)
{
	FUNCTRACKER;

	ShutDown();

}

void
ProcSipStack::UponMakeCall(CcuMsgPtr ptr)
{
 	FUNCTRACKER;
 
 	CcuApiErrorCode res = _dumUac->MakeCall(ptr);

	if (res != CCU_API_SUCCESS)
	{
		SendResponse(ptr, new CcuMsgMakeCallFailure());
	}

}

void
ProcSipStack::UponStartRegistration(CcuMsgPtr ptr)
{
// 	FUNCTRACKER;
// 
// 	CcuMsgStartRegisterRequest *req  = 
// 		boost::shared_dynamic_cast<CcuMsgStartRegisterRequest>(ptr).get();
// 
// 	
// 	NameAddr nameAddr(req->proxy_id.c_str());
// 	LogInfo (L"sipstack> send sip:register proxy=[" << nameAddr <<"]");
// 
// 	SharedPtr<SipMessage> session = 
// 		_dumUac->makeRegistration(
// 			nameAddr, 
// 			_dumUac->getMasterProfile(), 
// 			new UACAppDialogSet(req->handle,*_dumUac)); 
// 
// 	_dumUac->send(session);

}



void
ProcSipStack::UponCallOfferedAck(CcuMsgPtr req)
{
	FUNCTRACKER;

	shared_ptr<CcuMsgCalOfferedlAck> ack = 
		dynamic_pointer_cast<CcuMsgCalOfferedlAck>(req);

	CcuHandlesMap::iterator iter = _ccuHandlesMap.find(ack->stack_call_handle);
	if (iter == _ccuHandlesMap.end())
	{
		LogWarn("Handle=[" << ack->stack_call_handle<< "] not found.");
		SendResponse(req, new CcuMsgNack());
		return;
	}


	_dumUas->UponCallOfferedAck(req,(*iter).second);

}

bool 
ProcSipStack::ProcessCcuMessages()
{
	
	bool shutdown = false;
	while (InboundPending())
	{
		CcuMsgPtr msg = GetInboundMessage();

		LogInfo(L" Processing msg=[" << msg->message_id_str <<"]");

		if (msg.get() == NULL)
			break;

		switch (msg->message_id)
		{
		case CCU_MSG_MAKE_CALL_REQUEST:
			{
				UponMakeCall(msg);
				break;
			}
// 		case CCU_MSG_START_REGISTRATION_REQUEST:
// 			{
// 				UponStartRegistration(msg);
// 				break;
// 			}
		case CCU_MSG_PROC_SHUTDOWN_REQ:
			{
				
				ShutDown(msg);
				shutdown = true;
				SendResponse(msg, new CcuMsgShutdownAck());
				break;
			}
		case CCU_MSG_HANGUP_CALL_REQUEST:
			{
				UponHangupCall(msg);
				break;

			}
		case CCU_MSG_CALL_OFFERED_ACK:
			{
				UponCallOfferedAck(msg);
				break;
			}
		default:
			{ 
				if (HandleOOBMessage(msg) == FALSE)
				{
					LogInfo(L" Received unknown message " << msg->message_id_str)
				}
				
			}
		}
	}

	return shutdown;
}


void 
ProcSipStack::real_run()
{
	FUNCTRACKER;

	if (Init() != CCU_API_SUCCESS)
	{
		return;
	}

	BOOL shutdown_flag = FALSE;
	while (shutdown_flag == FALSE)
	{
		try
		{
			FdSet fdset;
			_stack.process(fdset); 
			_si->_siPtr->buildFdSet(fdset);
			_stack.buildFdSet(fdset);

#pragma warning (push)
#pragma warning (disable :4267)
			int ret = fdset.selectMilliSeconds(
				resipMin(
				 _stack.getTimeTillNextProcessMS(), 
				 getTimeTillNextProcessMS()));
#pragma warning (pop)

			if (ret < 0)
			{
				LogCrit(L" -1 on select !!!");
				continue;
			}

			shutdown_flag = ProcessCcuMessages();
			if (shutdown_flag)
			{
				break;
			}

			_si->_siPtr->process(fdset);
			_stack.process(fdset);
			while(_dumUas->process());
			while(_dumUac->process());

		}
		catch (BaseException& e)
		{
			LogWarn(L"Unhandled exception msg=[" << e.getMessage() <<"]");
		}
	}


}

unsigned int
ProcSipStack::getTimeTillNextProcessMS() const
{
	return 100000;   
}




CcuResipLogger::~CcuResipLogger()
{

}

/** return true to also do default logging, false to suppress default logging. */
bool 
CcuResipLogger::operator()(Log::Level level,
		const Subsystem& subsystem, 
		const Data& appName,
		const char* file,
		int line,
		const Data& message,
		const Data& messageWithHeaders)
{

	
	switch (level)
	{
	case Log::Info:
		{
			LogInfoRaw(message.c_str());
			break;
		}
	case Log::Debug:
		{
			LogDebugRaw(message.c_str());
			break;
		}
	case Log::Warning:
	case Log::Err:
		{
			LogWarnRaw(message.c_str());
			break;

		}
	case Log::Crit:
	
		{
			LogCritRaw(message.c_str());
			break;
		}
	default:
		{

		}
	}
	
	return false;

}




