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
#include "UASDialogUsageManager.h"






using namespace resip;
using namespace std;


namespace ivrworx
{
	typedef
	boost::char_separator<char> resip_conf_separator;

	typedef 
	boost::tokenizer<resip_conf_separator> resip_conf_tokenizer;

	ResipInterruptor::ResipInterruptor(IwDialogUsageManager *dum):
	_dumPtr(dum)
	{

		

	}

	void 
	ResipInterruptor::SignalDataIn()
	{
		
		boost::mutex::scoped_lock lock(_mutex);

		if (_dumPtr)
		{
			_dumPtr->mFifo.add(NULL, TimeLimitFifo<Message>::InternalElement);
		}
		
	}

	void 
	ResipInterruptor::SignalDataOut()
	{
		


	}

	void 
	ResipInterruptor::Destroy()
	{
		FUNCTRACKER;

		boost::mutex::scoped_lock lock(_mutex);

		_dumPtr = NULL;

	}

	 
	ResipInterruptor::~ResipInterruptor()
	{
		FUNCTRACKER;

	}

	ProcSipStack::ProcSipStack(IN LpHandlePair pair, 
		IN Configuration &conf):
		LightweightProcess(pair,SIP_STACK_Q,"SipStack"),
		_shutDownFlag(false),
		_conf(conf),
		_stack(NULL, resip::DnsStub::EmptyNameserverList, &_si),
		_stackThread(_stack,_si),
		_dumMngr(_stack),
		_dumUas(_conf,_iwHandlesMap,_resipHandlesMap,pair.outbound,_dumMngr),
		_dumUac(_conf,_iwHandlesMap,_resipHandlesMap,_dumMngr)

	{
		FUNCTRACKER;

		Log::initialize(Log::OnlyExternal, Log::Debug, NULL, _logger);
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
		const string &resip_conf = _conf.GetString("resip_log");

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
		
	}


	ApiErrorCode
	ProcSipStack::Init()
	{
		FUNCTRACKER;

		try 
		{
			
			
			//
			// Prepare SIP stack
			//
			const string ivr_host_str = _conf.GetString("ivr_sip_host");
			const int ivr_ip_int	= _conf.GetInt("ivr_sip_port" );


			CnxInfo ipAddr(
				convert_hname_to_addrin(ivr_host_str.c_str()),
				ivr_ip_int);
		
			_stack.addTransport(
				UDP,
				ipAddr.port_ho(),
				V4,
				StunDisabled,
				ipAddr.iptoa(),
				Data::Empty, // only used for TLS based stuff 
				Data::Empty,
				SecurityTypes::TLSv1 );


			_stack.addTransport(
				TCP,
				ipAddr.port_ho(),
				V4,
				StunDisabled,
				ipAddr.iptoa(),
				Data::Empty, // only used for TLS based stuff 
				Data::Empty,
				SecurityTypes::TLSv1 );


			//
			// Prepare interruptor
			//
			_dumInt = ResipInterruptorPtr(new ResipInterruptor(&_dumMngr));
			_inbound->HandleInterruptor(_dumInt);


			string uasUri = "sip:" + _conf.GetString("from_id") + "@" + ipAddr.ipporttos();
			NameAddr uasAor	(uasUri.c_str());


			_dumMngr.setMasterProfile(SharedPtr<MasterProfile>(new MasterProfile()));

			auto_ptr<ClientAuthManager> uasAuth (new ClientAuthManager());
			_dumMngr.setClientAuthManager(uasAuth);
			

			_dumMngr.getMasterProfile()->setDefaultFrom(uasAor);
			_dumMngr.getMasterProfile()->setDefaultRegistrationTime(70);

			_dumMngr.getMasterProfile()->addSupportedMethod(INFO);
			_dumMngr.getMasterProfile()->addSupportedMimeType(INFO,Mime("application","dtmf-relay"));
			_dumMngr.getMasterProfile()->addSupportedMimeType(INFO,Mime("application","mediaservercontrol+xml"));



			if (_conf.GetBool("sip_session_timer_enabled"))
			{
// 				auto_ptr<KeepAliveManager> keepAlive(new resip::KeepAliveManager());
// 				_dumMngr.setKeepAliveManager(keepAlive); 
// 				_dumMngr.getMasterProfile()->setKeepAliveTimeForDatagram(30);
// 				_dumMngr.getMasterProfile()->setKeepAliveTimeForStream(30);

				_dumMngr.getMasterProfile()->addSupportedOptionTag(Token(Symbols::Timer));

				_confSessionTimerModeMap["prefer_uac"]	  = Profile::PreferCallerRefreshes;
				_confSessionTimerModeMap["prefer_uas"]	  = Profile::PreferCalleeRefreshes;
				_confSessionTimerModeMap["prefer_local"]  = Profile::PreferLocalRefreshes;
				_confSessionTimerModeMap["prefer_remote"] = Profile::PreferRemoteRefreshes;

				ConfSessionTimerModeMap::iterator  i = _confSessionTimerModeMap.find(_conf.GetString("sip_refresh_mode"));
				if (i == _confSessionTimerModeMap.end())
				{
					LogInfo("Setting refresh mode to 'none'");
				}
				else
				{
					LogInfo("Setting refresh mode to " << i->first);
					_dumMngr.getMasterProfile()->setDefaultSessionTimerMode(i->second);

					int sip_default_session_time = _conf.GetInt("sip_default_session_time");
					sip_default_session_time = sip_default_session_time < 90 ? 90 : sip_default_session_time;
					LogInfo("sip_default_session_time:" << sip_default_session_time);

					_dumMngr.getMasterProfile()->setDefaultSessionTime(sip_default_session_time);

				}

			}

			_dumMngr.setClientRegistrationHandler(this);
			_dumMngr.setInviteSessionHandler(this);
			_dumMngr.addClientSubscriptionHandler("refer",this);
			_dumMngr.addOutOfDialogHandler(OPTIONS, this);
			


			auto_ptr<AppDialogSetFactory> uas_dsf(new UASAppDialogSetFactory());
			_dumMngr.setAppDialogSetFactory(uas_dsf);
			

			LogInfo("UAS started on " << ipAddr.ipporttos());


			_stackThread.run();

			return API_SUCCESS;

		}
		catch (BaseException& e)
		{
			LogWarn("Error while starting sip stack - " << e.getMessage().c_str());
			return API_FAILURE;
		}

	}

	void 
	ProcSipStack::onSessionExpired(IN InviteSessionHandle is)
	{
		FUNCTRACKER;

		LogWarn("ProcSipStack::onSessionExpired - rsh:" << is.getId());
		ResipDialogHandlesMap::iterator iter  = _resipHandlesMap.find(is->getAppDialog());
		if (iter != _resipHandlesMap.end())
		{
			SipDialogContextPtr ctx = (*iter).second;
			LogDebug("ProcSipStack::onSessionExpired rsh:" << is.getId() << ", iwh:" << ctx->stack_handle);
			FinalizeContext(ctx);
		} 


	}

	void 
	ProcSipStack::UponInfoReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUac.UponInfoReq(req);

	}

	void 
	ProcSipStack::UponBlindXferReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUac.UponBlindXferReq(req);

	}

	void 
	ProcSipStack::UponHangupCallReq(IwMessagePtr ptr)
	{
		FUNCTRACKER;

		_dumUac.UponHangupReq(ptr);
		
		
	}

	void
	ProcSipStack::ShutdownStack()
	{
		FUNCTRACKER;

		if (_shutDownFlag)
		{
			return;
		}

		_shutDownFlag = true;

 		_dumMngr.forceShutdown(NULL);
		_stackThread.shutdown();
		_stackThread.join();
		_stack.shutdown();
		if (_dumInt)_dumInt->Destroy();
	}

	void
	ProcSipStack::UponShutDownReq(IwMessagePtr req)
	{
		FUNCTRACKER;

	}

	void
	ProcSipStack::UponCallOfferedAck(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUas.UponCallOfferedAck(req);

	}

	void
	ProcSipStack::UponCallOfferedNack(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUas.UponCallOfferedNack(req);

	}

	void
	ProcSipStack::UponMakeCallReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUac.UponMakeCallReq(req);
	}

	void
	ProcSipStack::UponMakeCallAckReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUac.UponMakeCallAckReq(req);
	}

	
	bool 
	ProcSipStack::ProcessApplicationMessages()
	{

		FUNCTRACKER;
		
		bool shutdown = false;
		if (InboundPending())
		{
			ApiErrorCode res;
			IwMessagePtr msg;

			msg = GetInboundMessage(Seconds(0),res);
			if (IW_FAILURE(res))
			{
				throw;
			}

			switch (msg->message_id)
			{
			case SIP_CALL_INFO_REQ:
				{
					UponInfoReq(msg);
					break;
				}
			case MSG_CALL_BLIND_XFER_REQ:
				{
					UponBlindXferReq(msg);
					break;
				}
			case MSG_PROC_SHUTDOWN_REQ:
				{
					UponShutDownReq(msg);
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

		ApiErrorCode res = Init();
		if (IW_FAILURE(res))
		{
			LogCrit("ProcSipStack::real_run - Cannot start sip stack res:" << res);
			return;
		}

		I_AM_READY;

		BOOL shutdown_flag = FALSE;
		while (shutdown_flag == FALSE)
		{
			try
			{
				// ***
				// IX_PROFILE_CHECK_INTERVAL(11000);
				// ***

				// taken from DumThread
				std::auto_ptr<Message> msg(_dumMngr.mFifo.getNext(60000));  // Only need to wake up to see if we are shutdown
				if (msg.get())
				{
					_dumMngr.internalProcess(msg);
					continue;
				} 

				if (InboundPending())
				{
					shutdown_flag = ProcessApplicationMessages();
					if (shutdown_flag)
					{
						break;
					}
				}
				else
				{
					LogInfo("Sip keep alive.");
				}
			} 
			catch (exception &e)
			{
				LogWarn("ProcSipStack::real_run exception in stack e:" << e.what());
				break;
			} // try-catch
		} // while

		for (IwHandlesMap::iterator iter = _iwHandlesMap.begin(); iter != _iwHandlesMap.end() ; ++iter)
		{
			FinalizeContext((*iter).second);
		}

		ShutdownStack();

	}

	// generic auto answer
	// generic offer
	void 
	ProcSipStack::onOffer(
		InviteSessionHandle h, 
		const SipMessage& msg, 
		const Contents& body)
	{
		FUNCTRACKER;
		InviteSessionHandler::onOffer(h,msg,body);
	}

	// generic answer 
	void 
	ProcSipStack::onAnswer(
		InviteSessionHandle h, 
		const SipMessage& msg, 
		const Contents& body)
	{
		FUNCTRACKER;

		// On answer is actually called when answer SDP is received
		// it may be in first OK sent to UAC, or in ACK response to 
		// empty invite sent by UAS, in this case it is not correct 
		// to send it to uac dum. This case is not handled.
		_dumUac.onAnswer(h,msg,body);
	}

	void 
	ProcSipStack::onEarlyMedia(
		ClientInviteSessionHandle h, 
		const SipMessage& msg, 
		const Contents& body)
	{
		FUNCTRACKER;
		InviteSessionHandler::onEarlyMedia(h,msg,body);
	}

	// generic 
	void 
	ProcSipStack::onRemoteAnswerChanged(
		InviteSessionHandle h, 
		const SipMessage& msg, 
		const Contents& body)
	{
		FUNCTRACKER;
		InviteSessionHandler::onRemoteAnswerChanged(h,msg,body);
	}


	void 
	ProcSipStack::onNewSession(
		IN ClientInviteSessionHandle s, 
		IN InviteSession::OfferAnswerType oat, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUac.onNewSession(s,oat,msg);
	}

	void 
	ProcSipStack::onConnected(
		IN ClientInviteSessionHandle is, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUac.onConnected(is,msg);
	}


	void 
	ProcSipStack::onNewSession(
		IN ServerInviteSessionHandle sis, 
		IN InviteSession::OfferAnswerType oat, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUas.onNewSession(sis,oat,msg);
	}

	void 
	ProcSipStack::onFailure(IN ClientInviteSessionHandle is, IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUac.onFailure(is,msg);

	}

	void 
	ProcSipStack::onOffer(
		IN InviteSessionHandle is, 
		IN const SipMessage& msg, 
		IN const SdpContents& sdp)
	{
		FUNCTRACKER;
		
		// On offer is actually called when first SDP is received
		// it may be in first invite sent to UAS, or in response to 
		// empty invite sent by UAC, in this case it is not correct 
		// to send it to uas dum. This case is not handled.
		_dumUas.onOffer(is,msg,sdp);
	}

	/// called when ACK (with out an answer) is received for initial invite (UAS)
	void 
	ProcSipStack::onConnectedConfirmed( 
		IN InviteSessionHandle is, 
		IN const SipMessage &msg)
	{

		FUNCTRACKER;
		_dumUas.onConnectedConfirmed(is,msg);
	}

	void 
	ProcSipStack::onReceivedRequest(
		IN ServerOutOfDialogReqHandle ood, 
		IN const SipMessage& request)
	{
		FUNCTRACKER;
		_dumUas.onReceivedRequest(ood,request);
	}

	void 
	ProcSipStack::FinalizeContext(SipDialogContextPtr ctx_ptr)
	{
		FUNCTRACKER;
		
		IwStackHandle ixhandle = ctx_ptr->stack_handle;
		ctx_ptr->call_handler_inbound->Send(new MsgCallHangupEvt());

		
		_iwHandlesMap.erase(ctx_ptr->stack_handle);

		if (ctx_ptr->uas_invite_handle.isValid())
		{
			_resipHandlesMap.erase(ctx_ptr->uas_invite_handle->getAppDialog());
		}
		if (ctx_ptr->uac_invite_handle.isValid())
		{
			_resipHandlesMap.erase(ctx_ptr->uac_invite_handle->getAppDialog());
		}
	}

	void 
	ProcSipStack::onTerminated(
		IN InviteSessionHandle is, 
		IN InviteSessionHandler::TerminatedReason reason, 
		IN const SipMessage* msg)
	{
		
		FUNCTRACKER;

		// this logic is not UAS/UAC specific
		LogDebug("ProcSipStack::onTerminated rsh:" << is.getId());
	
		ResipDialogHandlesMap::iterator iter  = _resipHandlesMap.find(is->getAppDialog());
		if (iter != _resipHandlesMap.end())
		{
			SipDialogContextPtr ctx = (*iter).second;
			LogDebug("ProcSipStack::onTerminated rsh:" << is.getId() << ", iwh:" << ctx->stack_handle);
			FinalizeContext(ctx);
		} 
	}

	void 
	ProcSipStack::onInfo(
		IN InviteSessionHandle is, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;

		LogDebug("ProcSipStack::onInfo rsh:" << is.getId());

		IwStackHandle ixhandle = IW_UNDEFINED;
		ResipDialogHandlesMap::iterator iter  = _resipHandlesMap.find(is->getAppDialog());
		if (iter != _resipHandlesMap.end())
		{
			if (!msg.exists(h_ContentType))
			{
				LogWarn("Cannot deduce content/type type");
				return;
			};

			// early termination
			SipDialogContextPtr ctx_ptr = (*iter).second;

			Mime mime = msg.getContents()->getType();

			if (ctx_ptr->generic_offer_answer)
			{
				MsgSipCallInfoReq *info_req = 
					new MsgSipCallInfoReq();

				info_req->free_body = msg.getContents()->getBodyData().c_str();
				info_req->body_type = msg.getContents()->getType().type().c_str();

				ctx_ptr->call_handler_inbound->Send(IwMessagePtr(info_req));
				is->acceptNIT();
				return;
			}

			if (mime.type() == "application" && 
				mime.subType() == "dtmf-relay")
			{
				Data data = msg.getContents()->getBodyData();


				std::istringstream iss(data.c_str());
				string signal,duration;

				std::getline(iss, signal,'=');
				string str_signal_value;
				int signal_value;

				// the value may be index
				// or symbol


				iss >> str_signal_value;

				if (str_signal_value == "*")
				{
					signal_value = 10;
				} else if (str_signal_value == "#")
				{
					signal_value = 11;
				} else 
				{
					
					signal_value = ::atoi(str_signal_value.c_str());

					if (signal_value < 0 || signal_value > 15)
					{
						is->rejectNIT();
						return;
					}
				}

				



				MsgCallDtmfEvt *dtmf_evt = 
					new MsgCallDtmfEvt();

				switch (signal_value)
				{
				case 0:
					{
						dtmf_evt->signal = "0";
						break;
					}
				case 1:
					{
						dtmf_evt->signal = "1";
						break;
					}
				case 2:
					{
						dtmf_evt->signal = "2";
						break;
					}
				case 3:
					{
						dtmf_evt->signal = "3";
						break;
					}
				case 4:
					{
						dtmf_evt->signal = "4";
						break;
					}
				case 5:
					{
						dtmf_evt->signal = "5";
						break;
					}
				case 6:
					{
						dtmf_evt->signal = "6";
						break;
					}
				case 7:
					{
						dtmf_evt->signal = "7";
						break;
					}
				case 8:
					{
						dtmf_evt->signal = "8";
						break;
					}
				case 9:
					{
						dtmf_evt->signal = "9";
						break;
					}
				case 10:
					{
						dtmf_evt->signal = "*";
						break;
					}
				case 11:
					{
						dtmf_evt->signal = "#";
						break;
					}
				default:
					{
						
						is->rejectNIT();
						return;
					}

				}

			
				ctx_ptr->call_handler_inbound->Send(IwMessagePtr(dtmf_evt));
				is->acceptNIT();
				return;
			} 

			is->rejectNIT();

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