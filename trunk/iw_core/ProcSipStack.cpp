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

#include "UASDialogUsageManager.h"
#include "Profiler.h"





using namespace resip;
using namespace std;


namespace ivrworx
{
	typedef
	boost::char_separator<char> resip_conf_separator;

	typedef 
	boost::tokenizer<resip_conf_separator> resip_conf_tokenizer;

	

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
		LightweightProcess(pair,SIP_STACK_Q,"SipStack"),
		_shutDownFlag(false),
		_conf(conf)
	{
		FUNCTRACKER;

		Log::initialize(Log::OnlyExternal, Log::Debug, NULL, _logger);
		_stack = SipStackPtr(new SipStack());

		SetResipLogLevel();
				
	}

    typedef
    map<string,Subsystem *> SubsystemsMap;

	typedef
	map<string,Log::Level> LogLevelMap;

	void
	ProcSipStack::SetResipLogLevel()
	{
		LogLevelMap levelsMap;
		levelsMap["OFF"] = Log::None;
		levelsMap["CRT"] = Log::Crit;
		levelsMap["WRN"] = Log::Warning;	
		levelsMap["INF"] = Log::Info;
		levelsMap["DBG"] = Log::Debug;

		SubsystemsMap subsystemMap;
		subsystemMap["APP"]	= &Subsystem::APP;
		subsystemMap["CONTENTS"] = &Subsystem::CONTENTS;
		subsystemMap["DNS"]	= &Subsystem::DNS;	
		subsystemMap["DUM"]	= &Subsystem::DUM;
		subsystemMap["NONE"] = &Subsystem::NONE; 
		subsystemMap["PRESENCE"] = &Subsystem::PRESENCE; 
		subsystemMap["SDP"] = &Subsystem::SDP;
		subsystemMap["SIP"] = &Subsystem::SIP;
		subsystemMap["TRANSPORT"] = &Subsystem::TRANSPORT;
		subsystemMap["STATS"] = &Subsystem::STATS;
		subsystemMap["REPRO"] = &Subsystem::REPRO;

		// firstly reset all log levels to none
		for (SubsystemsMap::iterator iter = subsystemMap.begin(); 
			iter != subsystemMap.end(); 
			iter++)
		{
			iter->second->setLevel(Log::None);
		}

		

		// dirty parsing
		#pragma warning (suppress: 4129)
		regex e("(APP|CONTENTS|DNS|DUM|NONE|PRESENCE|SDP|SIP|TRANSPORT|STATS|REPRO)\,(OFF|CRT|WRN|INF|DBG)");
		

		resip_conf_separator sep("|");
		const string &resip_conf = _conf.ResipLog();

		resip_conf_tokenizer tokens(resip_conf, sep);

		for (resip_conf_tokenizer::const_iterator  tok_iter = tokens.begin();
			tok_iter != tokens.end(); 
			++tok_iter)
		{
			boost::smatch what;
			if (true == boost::regex_match(*tok_iter, what, e, boost::match_extra))
			{
				const string &subsystem_str = what[1];
				const string &debug_level = what[2];

				if (subsystem_str.length() > 0	&&
					debug_level.length() > 0	&&
					subsystemMap.find(subsystem_str) != subsystemMap.end() &&
					levelsMap.find(debug_level)      != levelsMap.end())
				{
					LogInfo("Resiprocate log level of " << subsystem_str << " set to " << debug_level << ".");
					subsystemMap[subsystem_str]->setLevel(levelsMap[debug_level]);
				} 
				else
				{
					LogWarn("Error while parsing resip debug configuration string - " << resip_conf);
					break;
				}
			}

		}


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
			
			
			CnxInfo ipAddr = _conf.IvrCnxInfo();

			_stack->addTransport(
				UDP,
				ipAddr.port_ho(),
				V4,
				StunDisabled,
				ipAddr.iptoa(),
				Data::Empty, // only used for TLS based stuff 
				Data::Empty,
				SecurityTypes::TLSv1 );



			_stack->addTransport(
				TCP,
				ipAddr.port_ho(),
				V4,
				StunDisabled,
				ipAddr.iptoa(),
				Data::Empty, // only used for TLS based stuff 
				Data::Empty,
				SecurityTypes::TLSv1 );


			//
			// UAS
			//
			_dumUas = UASDialogUsageManagerPtr(new UASDialogUsageManager(
				_conf,
				*_stack,
				_iwHandlesMap,
				*this));

			//
			// UAC
			// 
			_dumUac = UACDialogUsageManagerPtr(new UACDialogUsageManager(
				_conf,
				*_stack,
				_iwHandlesMap,
				*this));

			_handleInterruptor = ResipInterruptorPtr(new ResipInterruptor());
			_inbound->HandleInterruptor(_handleInterruptor);

			return API_SUCCESS;

		}
		catch (BaseException& e)
		{
			LogWarn("Error while starting sip stack - " << e.getMessage().c_str());
			return API_FAILURE;
		}

	}

	void 
	ProcSipStack::UponBlindXferReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		IX_PROFILE_FUNCTION();

		_dumUas->UponBlindXferReq(req);

	}

	void 
	ProcSipStack::UponHangupCallReq(IwMessagePtr ptr)
	{
		FUNCTRACKER;

		IX_PROFILE_FUNCTION();

		shared_ptr<MsgHangupCallReq> hangup_msg = 
			dynamic_pointer_cast<MsgHangupCallReq>(ptr);

		IwStackHandle handle = hangup_msg->stack_call_handle;

		IwHandlesMap::iterator iter = _iwHandlesMap.find(handle);
		if (iter == _iwHandlesMap.end())
		{
			LogWarn("The call iwh:" << hangup_msg->stack_call_handle << " already hanged up.");
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;

		if (ctx_ptr->transaction_type == TXN_TYPE_UAC ) 
		{
			_dumUac->HangupCall(ctx_ptr);
			return;
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

 		if (_dumUac)
 		{
 			_dumUac->Shutdown();
 		}

		if (_dumUas)
		{
			_dumUas->forceShutdown(NULL);
		}

		_stack->shutdown();
	}

	void
	ProcSipStack::ShutDown(IwMessagePtr req)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		ShutDown();

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

	void
	ProcSipStack::UponMakeCallReq(IwMessagePtr req)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		_dumUac->UponMakeCallReq(req);
	}

	void
	ProcSipStack::UponMakeCallAckReq(IwMessagePtr req)
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		_dumUac->UponMakeCallAckReq(req);
	}

	bool 
	ProcSipStack::ProcessIwMessages()
	{

		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		bool shutdown = false;
		if (InboundPending())
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
			case MSG_CALL_BLIND_XFER_REQ:
				{
					UponBlindXferReq(msg);
					break;
				}
			case MSG_PROC_SHUTDOWN_REQ:
				{
					ShutDown(msg);
					shutdown = true;
					SendResponse(msg, new MsgShutdownAck());
					break;
				}
			case MSG_HANGUP_CALL_REQ:
				{
					UponHangupCallReq(msg);
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
			case MSG_MAKE_CALL_REQ:
				{
					UponMakeCallReq(msg);
					break;
				}
			case MSG_MAKE_CALL_ACK:
				{
					UponMakeCallAckReq(msg);
					break;
				}
			default:
				{ 
					if (HandleOOBMessage(msg) == FALSE)
					{
						LogCrit("Received unknown message " << msg->message_id_str);
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
			_stack->buildFdSet(fdset);



			int ret = fdset.selectMilliSeconds(_stack->getTimeTillNextProcessMS());
			if (ret < 0)
			{
				LogCrit("Error while selecting in sip stack res=[" << ret << "].");
				throw;
			}

			IX_PROFILE_CODE(_handleInterruptor->process(fdset));
			IX_PROFILE_CODE(_stack->process(fdset));
			IX_PROFILE_CODE(while(_dumUas->process()));
			IX_PROFILE_CODE(while(_dumUac->process()));

			shutdown_flag = ProcessIwMessages();
			if (shutdown_flag)
			{
				break;
			}

		}
	}


	IwResipLogger::~IwResipLogger()
	{

	}

	/** return true to also do default logging, false to suppress default logging. */
	bool 
	IwResipLogger::operator()(Log::Level level,
		const Subsystem& subsystem, 
		const Data& appName,
		const char* file,
		int line,
		const Data& message,
		const Data& messageWithHeaders)
	{

		if (subsystem.getLevel() == Log::None)
		{
			return false;
		}

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
				LogDebug(subsystem.getSubsystem().c_str() << " " << message.c_str());
			}
		}

		return false;

	}




}