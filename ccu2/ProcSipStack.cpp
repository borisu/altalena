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


#define TimerInit() //int g_start = ::GetTickCount(); int g_end


#define LogTimer(X) //g_end=::GetTickCount();  LogCrit( __FUNCTIONW__ << X << " took " << (g_end - g_start)); g_start =::GetTickCount();


using namespace resip;
using namespace std;

ResipInterruptor::ResipInterruptor()
{

}

void 
ResipInterruptor::SignalDataIn()
{
	interrupt();
}

void 
ResipInterruptor::SignalDataOut()
{

}

ProcSipStack::ProcSipStack(IN LpHandlePair pair, 
						   IN CcuConfiguration &conf):
LightweightProcess(pair,__FUNCTIONW__),
_shutDownFlag(false),
_conf(conf)
{
	FUNCTRACKER;
	Log::initialize(Log::Cout, Log::Debug, NULL, _logger);
	Log::initialize(Log::Cerr, Log::Debug, NULL, _logger);

// #pragma TODO ("Make it configurable")
// 
// 	// in debug we only print TRANSPORT subsystems
// 	if (IsDebug())
// 	{
// 		Subsystem::TRANSPORT.setLevel(Log::Debug);
// 		Subsystem::TRANSACTION.setLevel(Log::Debug);
// 	}
		
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
			_conf,
			_stack,
			_ccuHandlesMap,
			*this));
		
		//
		// UAC
		//
		_dumUac = UACDialogUsageManagerPtr(new UACDialogUsageManager(
			_stack,
			_conf.VcsCnxInfo(),
			_ccuHandlesMap,
			*this));

		_handleInterruptor = ResipInterruptorPtr(new ResipInterruptor());

		_inbound->HandleInterruptor(
			shared_dynamic_cast<IxInterruptor>(_handleInterruptor));

		I_AM_READY;

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

	shared_ptr<CcuMsgHangupCallReq> hangup_msg = 
		dynamic_pointer_cast<CcuMsgHangupCallReq>(ptr);

	CcuStackHandle handle = hangup_msg->stack_call_handle;

	CcuHandlesMap::iterator iter = _ccuHandlesMap.find(handle);
	if (iter == _ccuHandlesMap.end())
	{
		LogWarn("The call with IX handle =[" << hangup_msg->stack_call_handle << "] already hanged up");
		return;
	}

	SipDialogContextPtr ctx_ptr = (*iter).second;

	if (ctx_ptr->transaction_type == CCU_UAC)
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
		SendResponse(ptr, new CcuMsgMakeCallNack());
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
	_dumUas->UponCallOfferedAck(req);

}

void
ProcSipStack::UponCallOfferedNack(CcuMsgPtr req)
{
	FUNCTRACKER;
	_dumUas->UponCallOfferedNack(req);
}

bool 
ProcSipStack::ProcessCcuMessages()
{
	
	TimerInit();

	bool shutdown = false;
	while (InboundPending())
	{
		LogTimer("InboundPending()");
		CcuApiErrorCode res;

		int start = ::GetTickCount();

		CcuMsgPtr msg = GetInboundMessage(Seconds(0),res);
		int end = ::GetTickCount();

		LogTimer("GetInboundMessage(Seconds(0),res);");
		LogCrit("GetInboundMessage(Seconds(0),res); took " << (start - end));
		if (CCU_FAILURE(res))
		{
			throw;
		}

		//LogInfo(L" Processing msg=[" << msg->message_id_str <<"]");

		if (msg.get() == NULL)
			break;

		switch (msg->message_id)
		{
		case CCU_MSG_MAKE_CALL_REQ:
			{
				UponMakeCall(msg);
				LogTimer("UponMakeCall(msg)");
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
				LogTimer("ShutDown;");
				shutdown = true;
				SendResponse(msg, new CcuMsgShutdownAck());
				LogTimer("SendResponse(msg, new CcuMsgShutdownAck());");
				break;
			}
		case CCU_MSG_HANGUP_CALL_REQ:
			{
				UponHangupCall(msg);
				LogTimer("UponHangupCall(msg);");
				break;

			}
		case CCU_MSG_CALL_OFFERED_ACK:
			{
				UponCallOfferedAck(msg);
				LogTimer("UponCallOfferedAck(msg);");
				break;
			}
		case CCU_MSG_CALL_OFFERED_NACK:
			{
				UponCallOfferedNack(msg);
				LogTimer("UponCallOfferedNack(msg);");
				break;
			}
		default:
			{ 
				if (HandleOOBMessage(msg) == FALSE)
				{
					LogInfo(L" Received unknown message " << msg->message_id_str)
				}
				LogTimer("HandleOOBMessage(msg)");
				
			}
		}
	}

	return shutdown;
}


void 
ProcSipStack::real_run()
{
	FUNCTRACKER;

	TimerInit();

	if (Init() != CCU_API_SUCCESS)
	{
		return;
	}

	BOOL shutdown_flag = FALSE;

	
	while (shutdown_flag == FALSE)
	{

		FdSet fdset;
		_handleInterruptor->buildFdSet(fdset);
		_stack.buildFdSet(fdset);
		
		LogTimer("Build fd set");

		int ret = fdset.selectMilliSeconds(_stack.getTimeTillNextProcessMS());
		LogTimer("select");
		if (ret < 0)
		{
			LogCrit("Error while selecting in sip stack res=[" << ret << "].");
			throw;
		}

		_handleInterruptor->process(fdset);
		LogTimer("_handleInterruptor->process(fdset);");
		_stack.process(fdset);
		LogTimer("_stack.process(fdset);");
		while(_dumUas->process());
		LogTimer("while(_dumUas->process());");
		while(_dumUac->process());
		LogTimer("while(_dumUac->process());");

		shutdown_flag = ProcessCcuMessages();
		LogTimer("ProcessCcuMessages()");
		if (shutdown_flag)
		{
			break;
		}

	}
}





IxResipLogger::~IxResipLogger()
{

}

/** return true to also do default logging, false to suppress default logging. */
bool 
IxResipLogger::operator()(Log::Level level,
		const Subsystem& subsystem, 
		const Data& appName,
		const char* file,
		int line,
		const Data& message,
		const Data& messageWithHeaders)
{
	return false;
	
	
	
	switch (level)
	{
	case Log::Info:
	case Log::Warning:
		{
			LogInfo(subsystem.getSubsystem().c_str() << " " << message.c_str());
			break;
		}
	case Log::Debug:
		{
			// nasty hack to print sip messages only once
// 			if (strncmp(message.c_str(),"Send to TU:", 11) == 0)
// 			{
// 				return false;
// 			}
		
			LogDebug(subsystem.getSubsystem().c_str() << " " << message.c_str());
			break;
		}
	case Log::Err:
		{
			LogWarn(subsystem.getSubsystem().c_str() << " " << message.c_str());
			break;

		}
	case Log::Crit:
		{
			LogCrit(subsystem.getSubsystem().c_str() << " " << message.c_str());
			break;
		}
	default:
		{
			throw;
		}
	}
	
	return false;

}




