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
#include "UACAppDialogSet.h"
#include "UACDialogUsageManager.h"
#include "UASDialogUsageManager.h"
#include "Profiler.h"




using namespace resip;
using namespace std;

namespace ivrworx
{



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
		IN Configuration &conf):
		LightweightProcess(pair,__FUNCTION__),
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


	ApiErrorCode
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
				_iwHandlesMap,
				*this));

			//
			// UAC
			//
			_dumUac = UACDialogUsageManagerPtr(new UACDialogUsageManager(
				_stack,
				_conf.IvrCnxInfo(),
				_iwHandlesMap,
				*this));

			_handleInterruptor = ResipInterruptorPtr(new ResipInterruptor());

			_inbound->HandleInterruptor(
				shared_dynamic_cast<WaitInterruptor>(_handleInterruptor));

			return API_SUCCESS;

		}
		catch (BaseException& e)
		{
			LogWarn("Caught >>exception<< while starting msg=[" << e.getMessage().c_str() << "]");
			return API_FAILURE;
		}

	}

	void 
		ProcSipStack::UponHangupCall(IwMessagePtr ptr)
	{
		FUNCTRACKER;

		IX_PROFILE_FUNCTION();

		shared_ptr<MsgHangupCallReq> hangup_msg = 
			dynamic_pointer_cast<MsgHangupCallReq>(ptr);

		IwStackHandle handle = hangup_msg->stack_call_handle;

		IwHandlesMap::iterator iter = _iwHandlesMap.find(handle);
		if (iter == _iwHandlesMap.end())
		{
			LogWarn("The call with ix handle =[" << hangup_msg->stack_call_handle << "] already hanged up.");
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;

		if (ctx_ptr->transaction_type == TX_TYPE_UAC)
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
		ProcSipStack::ShutDown(IwMessagePtr req)
	{
		FUNCTRACKER;

		ShutDown();

	}

	void
		ProcSipStack::UponMakeCall(IwMessagePtr ptr)
	{
		FUNCTRACKER;

		ApiErrorCode res = _dumUac->MakeCall(ptr);

		if (res != API_SUCCESS)
		{
			SendResponse(ptr, new MsgMakeCallNack());
		}

	}

	void
		ProcSipStack::UponStartRegistration(IwMessagePtr ptr)
	{
		// 	FUNCTRACKER;
		// 
		// 	MsgStartRegisterRequest *req  = 
		// 		boost::shared_dynamic_cast<MsgStartRegisterRequest>(ptr).get();
		// 
		// 	
		// 	NameAddr nameAddr(req->proxy_id.c_str());
		// 	LogInfo ("sipstack> send sip:register proxy=[" << nameAddr <<"]");
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
		ProcSipStack::UponCallOfferedAck(IwMessagePtr req)
	{
		FUNCTRACKER;

		IX_PROFILE_FUNCTION();

		_dumUas->UponCallOfferedAck(req);

	}

	void
		ProcSipStack::UponCallOfferedNack(IwMessagePtr req)
	{
		FUNCTRACKER;

		IX_PROFILE_FUNCTION();

		_dumUas->UponCallOfferedNack(req);
	}

	bool 
		ProcSipStack::ProcessIwMessages()
	{

		FUNCTRACKER;

		IX_PROFILE_FUNCTION();


		bool shutdown = false;
		while (InboundPending())
		{
			ApiErrorCode res;
			IwMessagePtr msg;

			IX_PROFILE_CODE(msg = GetInboundMessage(Seconds(0),res));
			if (IW_FAILURE(res))
			{
				throw;
			}

			switch (msg->message_id)
			{
			case MSG_MAKE_CALL_REQ:
				{
					UponMakeCall(msg);
					break;
				}
				// 		case CCU_MSG_START_REGISTRATION_REQUEST:
				// 			{
				// 				UponStartRegistration(msg);
				// 				break;
				// 			}
			case MSG_PROC_SHUTDOWN_REQ:
				{

					ShutDown(msg);
					shutdown = true;
					SendResponse(msg, new MsgShutdownAck());
					break;
				}
			case MSG_HANGUP_CALL_REQ:
				{
					UponHangupCall(msg);
					break;

				}
			case MSG_CALL_OFFERED_ACK:
				{
					UponCallOfferedAck(msg);
					break;
				}
			case MSG_CALL_OFFERED_NACK:
				{
					UponCallOfferedNack(msg);
					break;
				}
			default:
				{ 
					if (HandleOOBMessage(msg) == FALSE)
					{
						LogCrit(" Received unknown message " << msg->message_id_str);
						throw;
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

		if (Init() != API_SUCCESS)
		{
			return;
		}

		I_AM_READY;

		BOOL shutdown_flag = FALSE;
		while (shutdown_flag == FALSE)
		{
			IX_PROFILE_CHECK_INTERVAL(25000);

			FdSet fdset;
			_handleInterruptor->buildFdSet(fdset);
			_stack.buildFdSet(fdset);



			int ret = fdset.selectMilliSeconds(_stack.getTimeTillNextProcessMS());
			if (ret < 0)
			{
				LogCrit("Error while selecting in sip stack res=[" << ret << "].");
				throw;
			}

			IX_PROFILE_CODE(_handleInterruptor->process(fdset));
			IX_PROFILE_CODE(_stack.process(fdset));
			IX_PROFILE_CODE(while(_dumUas->process()));
			IX_PROFILE_CODE(while(_dumUac->process()));

			shutdown_flag = ProcessIwMessages();
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




}