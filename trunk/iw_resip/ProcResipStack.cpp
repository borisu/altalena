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
#include "ProcResipStack.h"
#include "IwAppDialogSetFactory.h"
#include "UASDialogUsageManager.h"
#include "FreeContent.h"
#include "IwAppDialogSet.h"






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

	ProcResipStack::ProcResipStack(IN LpHandlePair pair, 
		IN ConfigurationPtr conf):
		LightweightProcess(pair,"SipStack"),
		_shutDownFlag(false),
		_conf(conf),
		_stack(NULL, resip::DnsStub::EmptyNameserverList, &_si),
		_stackThread(_stack,_si)
	{
		FUNCTRACKER;

		this->ServiceId(_conf->GetString("resip/uri"));

		Log::initialize(Log::OnlyExternal, Log::Debug, NULL, _logger);
		SetResipLogLevel();

		
				
	}

    typedef
    map<string,Subsystem *> SubsystemsMap;

	typedef
	map<string,Log::Level> LogLevelMap;

	void
	ProcResipStack::SetResipLogLevel()
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
		const string &resip_conf = _conf->GetString("resip/resip_log");

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


	ProcResipStack::~ProcResipStack(void)
	{
		FUNCTRACKER;
		
	}


	ApiErrorCode
	ProcResipStack::Init()
	{
		FUNCTRACKER;

		try 
		{

			_dumMngr.reset( new IwDialogUsageManager(_stack));
			_dumUas.reset( new UASDialogUsageManager(_conf,_iwHandlesMap,*_dumMngr));
			_dumUac.reset( new UACDialogUsageManager(_conf,_iwHandlesMap,*_dumMngr));

			
			//
			// Prepare SIP stack
			//
			const string ivr_host_str = _conf->GetString("resip/sip_host");
			const int ivr_ip_int	= _conf->GetInt("resip/sip_port" );


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
			_dumInt = ResipInterruptorPtr(new ResipInterruptor(_dumMngr.get()));
			_inbound->HandleInterruptor(_dumInt);


			string uasUri = "sip:" + _conf->GetString("resip/from_id") + "@" + ipAddr.ipporttos();
			NameAddr uasAor	(uasUri.c_str());


			_dumMngr->setMasterProfile(SharedPtr<MasterProfile>(new MasterProfile()));

			auto_ptr<ClientAuthManager> uasAuth (new ClientAuthManager());
			_dumMngr->setClientAuthManager(uasAuth);

			if (_conf->HasOption("resip/outbound_proxy"))
			{
				_dumMngr->getMasterProfile()->setOutboundProxy(Uri(_conf->GetString("resip/outbound_proxy").c_str()));
			}


			_dumMngr->getMasterProfile()->setDefaultFrom(uasAor);
			_dumMngr->getMasterProfile()->setDefaultRegistrationTime(70);

			_dumMngr->getMasterProfile()->addSupportedMethod(NOTIFY);
			_dumMngr->getMasterProfile()->addSupportedMimeType(NOTIFY,Mime("application","pidf+xml"));

			_dumMngr->getMasterProfile()->addSupportedMethod(INFO);
			_dumMngr->getMasterProfile()->addSupportedMimeType(INFO,Mime("application","dtmf-relay"));
			_dumMngr->getMasterProfile()->addSupportedMimeType(INFO,Mime("application","mediaservercontrol+xml"));
			_dumMngr->addClientSubscriptionHandler(Symbols::Presence, this);



			

			if (_conf->GetBool("resip/sip_session_timer_enabled"))
			{
// 				auto_ptr<KeepAliveManager> keepAlive(new resip::KeepAliveManager());
// 				_dumMngr->setKeepAliveManager(keepAlive); 
// 				_dumMngr->getMasterProfile()->setKeepAliveTimeForDatagram(30);
// 				_dumMngr->getMasterProfile()->setKeepAliveTimeForStream(30);

				_dumMngr->getMasterProfile()->addSupportedOptionTag(Token(Symbols::Timer));

				_confSessionTimerModeMap["prefer_uac"]	  = Profile::PreferCallerRefreshes;
				_confSessionTimerModeMap["prefer_uas"]	  = Profile::PreferCalleeRefreshes;
				_confSessionTimerModeMap["prefer_local"]  = Profile::PreferLocalRefreshes;
				_confSessionTimerModeMap["prefer_remote"] = Profile::PreferRemoteRefreshes;

				ConfSessionTimerModeMap::iterator  i = _confSessionTimerModeMap.find(_conf->GetString("resip/sip_refresh_mode"));
				if (i == _confSessionTimerModeMap.end())
				{
					LogInfo("Setting refresh mode to 'none'");
				}
				else
				{
					LogInfo("Setting refresh mode to " << i->first);
					_dumMngr->getMasterProfile()->setDefaultSessionTimerMode(i->second);

					int sip_default_session_time = _conf->GetInt("resip/sip_default_session_time");
					sip_default_session_time = sip_default_session_time < 90 ? 90 : sip_default_session_time;
					LogInfo("sip_default_session_time:" << sip_default_session_time);

					_dumMngr->getMasterProfile()->setDefaultSessionTime(sip_default_session_time);

				}

			}

			_dumMngr->setClientRegistrationHandler(this);
			_dumMngr->setInviteSessionHandler(this);
			_dumMngr->addClientSubscriptionHandler("refer",this);
			_dumMngr->addOutOfDialogHandler(OPTIONS, this);
			


			auto_ptr<AppDialogSetFactory> uas_dsf(new IwAppDialogSetFactory());
			_dumMngr->setAppDialogSetFactory(uas_dsf);
			

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
	ProcResipStack::onSessionExpired(IN InviteSessionHandle is)
	{
		FUNCTRACKER;

		LogWarn("ProcResipStack::onSessionExpired - rsh:" << is.getId());
		FinalizeContext(IWDIAGSET(is)->dialog_ctx);

	}

	void 
	ProcResipStack::UponSubscribeToIncomingReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUas->UponSubscribeToIncomingReq(req);

	}

	void 
	ProcResipStack::UponInfoReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUac->UponInfoReq(req);

	}

	void 
	ProcResipStack::UponBlindXferReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUac->UponBlindXferReq(req);

	}

	void 
	ProcResipStack::UponReofferReq(IN IwMessagePtr req)
	{
		FUNCTRACKER;

		shared_ptr<MsgCallReofferReq> roff_req = 
			dynamic_pointer_cast<MsgCallReofferReq>(req);

		IwHandlesMap::iterator iter = _iwHandlesMap.find(roff_req->stack_call_handle);
		if (iter == _iwHandlesMap.end())
		{
			LogWarn("ProcResipStack::UponReofferReq - iwh:" << roff_req->stack_call_handle << " not found. Has caller disconnected already?");
			GetCurrRunningContext()->SendResponse(
				req,
				new MsgCallReofferNack());
			return;
		}

		SipDialogContextPtr ctx_ptr = (*iter).second;

		ctx_ptr->last_reoffer_req = roff_req;

		Data free_data(roff_req->localOffer.body);

		FreeContent fc(roff_req->localOffer.body,roff_req->localOffer.type);

		
		ctx_ptr->invite_handle->provideOffer(fc);


	}

	void 
	ProcResipStack::UponHangupCallReq(IwMessagePtr ptr)
	{
		FUNCTRACKER;

		_dumUac->UponHangupReq(ptr);
		
		
	}

	void
	ProcResipStack::ShutdownStack()
	{
		FUNCTRACKER;

		if (_shutDownFlag)
		{
			return;
		}

		_shutDownFlag = true;

		_iwHandlesMap.clear();

 		_dumMngr->forceShutdown(NULL);
		if (_dumInt)_dumInt->Destroy();

		_dumUac.reset();
		_dumUas.reset();
		_dumMngr.reset();

		_stackThread.shutdown();
		_stackThread.join();
		_stack.shutdown();
		

	}

	void
	ProcResipStack::UponShutDownReq(IwMessagePtr req)
	{
		FUNCTRACKER;

	}

	void 
	ProcResipStack::UponCallConnected( 
		IN IwMessagePtr req)
	{

		FUNCTRACKER;
		_dumUac->UponCallConnected(req);
	}


	void
	ProcResipStack::UponCallOfferedAck(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUas->UponCallOfferedAck(req);

	}

	void
	ProcResipStack::UponCallOfferedNack(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUas->UponCallOfferedNack(req);

	}

	void
	ProcResipStack::UponMakeCallReq(IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUac->UponMakeCallReq(req);
	}

	void
	ProcResipStack::UponRegisterReq(IN IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUac->UponRegisterReq(req);
	}

	void
	ProcResipStack::UponUnRegisterReq(IN IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUac->UponUnRegisterReq(req);
	}

	void 
	ProcResipStack::UponSubscribeReq(IN IwMessagePtr req)
	{
		FUNCTRACKER;

		_dumUac->UponSubscribeReq(req);
	}


	void 
	ProcResipStack::onUpdatePending(
		IN ClientSubscriptionHandle h, 
		IN const SipMessage& notify, 
		IN bool outOfOrder)
	{
		FUNCTRACKER;

		_dumUac->onUpdatePending(h,notify,outOfOrder);

	}

	void 
	ProcResipStack::onUpdateActive(
		IN ClientSubscriptionHandle h, 
		IN const SipMessage& notify, 
		IN bool outOfOrder)
	{
		FUNCTRACKER;

		_dumUac->onUpdateActive(h,notify,outOfOrder);

	}

	//unknown Subscription-State value
	void 
	ProcResipStack::onUpdateExtension(
		IN ClientSubscriptionHandle h, 
		IN const SipMessage& notify, 
		IN bool outOfOrder)
	{
		FUNCTRACKER;

		_dumUac->onUpdateExtension(h,notify,outOfOrder);

	}


	bool 
	ProcResipStack::ProcessApplicationMessages()
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
			case MSG_CALL_SUBSCRIBE_REQ:
				{
					UponSubscribeToIncomingReq(msg);
					break;
				}
			case MSG_CALL_CONNECTED:
				{
					UponCallConnected(msg);
					break;
				}
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

			case SIP_CALL_REGISTER_REQ:
				{
					UponRegisterReq(msg);
					break;
				}
			case SIP_CALL_UNREGISTER_REQ:
				{
					UponUnRegisterReq(msg);
					break;
				}
			case MSG_CALL_REOFFER_REQ:
				{
					UponReofferReq(msg);
					break;
				}

			case SIP_CALL_SUBSCRIBE_REQ:
				{
					UponSubscribeReq(msg);
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
	ProcResipStack::real_run()
	{
		FUNCTRACKER;

		ApiErrorCode res = Init();
		if (IW_FAILURE(res))
		{
			LogCrit("ProcResipStack::real_run - Cannot start sip stack res:" << res);
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
				std::auto_ptr<Message> msg(_dumMngr->mFifo.getNext(60000));  // Only need to wake up to see if we are shutdown
				if (msg.get())
				{
					_dumMngr->internalProcess(msg);
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
				LogWarn("ProcResipStack::real_run exception in stack e:" << e.what());
				break;
			} // try-catch
		} // while

		IwHandlesMap::iterator iter = _iwHandlesMap.begin();
		while (iter != _iwHandlesMap.end())
		{
			// cannot use iterators in usual manner 
			// as FinalizeContext removes the context 
			// from the collection
			FinalizeContext((*iter).second);
			iter = _iwHandlesMap.begin();
		}

		ShutdownStack();

	}

	// You should only override the following method if genericOfferAnswer is true
	void 
	ProcResipStack::onOffer(
		InviteSessionHandle h, 
		const SipMessage& msg, 
		const Contents& body)
	{
		FUNCTRACKER;

		
		SipDialogContextPtr ctx = IWDIAGSET(h)->dialog_ctx;

		if (ctx->uac_invite_handle.isValid())
		{
			_dumUac->onOffer(h,msg,body);
		}
		else
		{
			_dumUas->onOffer(h,msg,body);
		}

		
	}

	/// called when an answer is received - has nothing to do with user
	/// answering the call 
	// ...	
	// You should only override the following method if genericOfferAnswer is true 
	void 
	ProcResipStack::onAnswer(
		InviteSessionHandle h, 
		const SipMessage& msg, 
		const Contents& body)
	{
		FUNCTRACKER;

		// On answer is actually called when answer SDP is received
		// it may be in first OK sent to UAC, or in ACK response to 
		// empty invite sent by UAS, in this case it is not correct 
		// to send it to uac dum. This case is not handled.
		_dumUac->onAnswer(h,msg,body);
	}

	void 
	ProcResipStack::onEarlyMedia(
		ClientInviteSessionHandle h, 
		const SipMessage& msg, 
		const Contents& body)
	{
		FUNCTRACKER;
		InviteSessionHandler::onEarlyMedia(h,msg,body);
	}

	// generic 
	void 
	ProcResipStack::onRemoteAnswerChanged(
		InviteSessionHandle h, 
		const SipMessage& msg, 
		const Contents& body)
	{
		FUNCTRACKER;
		InviteSessionHandler::onRemoteAnswerChanged(h,msg,body);
	}


	void 
	ProcResipStack::onNewSession(
		IN ClientInviteSessionHandle s, 
		IN InviteSession::OfferAnswerType oat, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUac->onNewSession(s,oat,msg);
	}

	void 
	ProcResipStack::onConnected(
		IN ClientInviteSessionHandle is, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUac->onConnected(is,msg);
	}


	void 
	ProcResipStack::onNewSession(
		IN ServerInviteSessionHandle sis, 
		IN InviteSession::OfferAnswerType oat, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUas->onNewSession(sis,oat,msg);
	}

	void 
	ProcResipStack::onFailure(
		IN ClientRegistrationHandle h, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUac->onFailure(h,msg);

	}

	void 
	ProcResipStack::onSuccess(
		IN ClientRegistrationHandle h, 
		IN const SipMessage& response)
	{
		FUNCTRACKER;
		_dumUac->onSuccess(h,response);
	}

	void 
	ProcResipStack::onFailure(
		IN ClientOutOfDialogReqHandle h, 
		IN const SipMessage& errorResponse)
	{
		FUNCTRACKER;
		_dumUac->onFailure(h,errorResponse);
	}

	void 
	ProcResipStack::onFailure(IN ClientInviteSessionHandle is, IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUac->onFailure(is,msg);

	}

	/// called when ACK (with out an answer) is received for initial invite (UAS)
	void 
	ProcResipStack::onConnectedConfirmed( 
		IN InviteSessionHandle is, 
		IN const SipMessage &msg)
	{

		FUNCTRACKER;
		_dumUas->onConnectedConfirmed(is,msg);
	}


	void 
	ProcResipStack::onReceivedRequest(
		IN ServerOutOfDialogReqHandle ood, 
		IN const SipMessage& request)
	{
		FUNCTRACKER;
		_dumUas->onReceivedRequest(ood,request);
	}

	//subscription can be ended through a notify or a failure response.
	void ProcResipStack::onTerminated(
		IN ClientSubscriptionHandle h, 
		IN const SipMessage* msg)
	{
		FUNCTRACKER;
		_dumUac->onTerminated(h,msg);

	}

	//not sure if this has any value.
	void ProcResipStack::onNewSubscription(
		IN ClientSubscriptionHandle h, 
		IN const SipMessage& notify)
	{
		FUNCTRACKER;
		_dumUac->onNewSubscription(h,notify);

	}

	void 
	ProcResipStack::FinalizeContext(SipDialogContextPtr ctx_ptr)
	{
		FUNCTRACKER;
		
		IwStackHandle ixhandle = ctx_ptr->stack_handle;

		if (ctx_ptr && ctx_ptr->call_handler_inbound)
			ctx_ptr->call_handler_inbound->Send(new MsgCallHangupEvt());

		_iwHandlesMap.erase(ctx_ptr->stack_handle);

	}

	void 
	ProcResipStack::onTerminated(
		IN InviteSessionHandle is, 
		IN InviteSessionHandler::TerminatedReason reason, 
		IN const SipMessage* msg)
	{
		
		FUNCTRACKER;

		// this logic is not UAS/UAC specific
		LogDebug("ProcResipStack::onTerminated rsh:" << is.getId());
		FinalizeContext(IWDIAGSET(is)->dialog_ctx);
		
	}

	/// called when response to INFO message is received 
	void 
	ProcResipStack::onInfoSuccess(
		IN InviteSessionHandle is, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUac->onInfoSuccess(is,msg);
	}

	int ProcResipStack::onRequestRetry(
		ClientSubscriptionHandle, 
		int retrySeconds, 
		const SipMessage& notify)
	{
		FUNCTRACKER;
		return -1;
	}


	void 
	ProcResipStack::onInfoFailure(
		IN InviteSessionHandle is, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;
		_dumUac->onInfoFailure(is,msg);
	}

	void 
	ProcResipStack::onInfo(
		IN InviteSessionHandle is, 
		IN const SipMessage& msg)
	{
		FUNCTRACKER;

		LogDebug("ProcResipStack::onInfo rsh:" << is.getId());

		
		if (!msg.exists(h_ContentType))
		{
			LogWarn("Cannot deduce content/type type");
			is->rejectNIT();
			return;
		};

		// early termination
		SipDialogContextPtr ctx_ptr = IWDIAGSET(is)->dialog_ctx;

		Mime mime = msg.getContents()->getType();

		
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
		} 
		else
		{
			MsgSipCallInfoReq *info_req = 
				new MsgSipCallInfoReq();

			info_req->remoteOffer.body = msg.getContents()->getBodyData().c_str();
			info_req->remoteOffer.type = string("") + msg.getContents()->getType().type().c_str() + "/" + msg.getContents()->getType().subType().c_str();;

			ctx_ptr->call_handler_inbound->Send(IwMessagePtr(info_req));
			is->acceptNIT();
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