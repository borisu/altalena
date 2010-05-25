#include "StdAfx.h"
#include "ProcOpalH323.h"

namespace ivrworx
{

#define	IW_DEFAULT_H323_TIMEOUT		60000 // 1 min

#define OPAL_EVENT 23145



class IwManager : public OpalManager
{
	PCLASSINFO(IwManager, OpalManager)
public:

	IwManager(IocpInterruptorPtr iocpPtr):
	_iocpPtr(iocpPtr)
	{
		
	}

	virtual PBoolean OnIncomingConnection(
		OpalConnection & connection,   ///<  Connection that is calling
		unsigned options,              ///<  options for new connection (can't use default as overrides will fail)
		OpalConnection::StringOptions * stringOptions
		)
	{
		
		
		OpalOverlapped *olap = 
			new OpalOverlapped();
		

		BOOL res = ::PostQueuedCompletionStatus(
			_iocpPtr->WinHandle(),	//A handle to an I/O completion port to which the I/O completion packet is to be posted.
			0,						//The value to be returned through the lpNumberOfBytesTransferred parameter of the GetQueuedCompletionStatus function.
			OPAL_EVENT,				//The value to be returned through the lpCompletionKey parameter of the GetQueuedCompletionStatus function.
			olap					//The value to be returned through the lpOverlapped parameter of the GetQueuedCompletionStatus function.
			);

		if (res == FALSE)
		{
			LogSysError("::PostQueuedCompletionStatus");
			throw;
		}

		return PTrue;
	}

	
	virtual ~IwManager()
	{
		ShutDownEndpoints();
	}

	IocpInterruptorPtr _iocpPtr;
		

};

class IwH323EndPoint : public H323EndPoint
{
	PCLASSINFO(IwH323EndPoint, H323EndPoint);
public:

	IwH323EndPoint(Configuration &conf, IwManager &manager, IocpInterruptorPtr iocpPtr):
	  H323EndPoint(manager),
	  _conf(conf),
	  _iocpPtr(iocpPtr)
	  {  
	  
	  }

	 

	  ApiErrorCode
	  InitialiseH323EP(PBoolean secure)
	  {
		  
		  
		  DisableFastStart(_conf.HasOption("opal/f"));
		  DisableH245Tunneling(_conf.HasOption("opal/T"));
		  SetSendGRQ(!_conf.HasOption("opal/disable-grq"));


		  // Get local username, multiple uses of -u indicates additional aliases
		  if (_conf.HasOption("opal/u")) {
			  PStringArray aliases = PString(_conf.GetString("opal/u")).Lines();
			  SetLocalUserName(aliases[0]);
			  for (PINDEX i = 1; i < aliases.GetSize(); i++)
				  AddAliasName(aliases[i]);
		  }

		  if (_conf.HasOption("opal/b")) {
			  unsigned initialBandwidth =  PString(_conf.GetString("opal/b")).AsUnsigned()*100;
			  if (initialBandwidth == 0) {
				  LogWarn ("InitialiseH323EP - Illegal bandwidth specified.");
				  return API_FAILURE;
			  }
			  SetInitialBandwidth(initialBandwidth);
		  }

		  if (_conf.HasOption("opal/gk-token"))
			  SetGkAccessTokenOID(PString(_conf.GetString("opal/gk-token")));

		  PString prefix = GetPrefixName();

		  LogInfo ( prefix << " Local username: " << GetLocalUserName() << endl
			  << prefix << " FastConnect is " << (IsFastStartDisabled() ? "off" : "on") << endl
			  << prefix << " H245Tunnelling is " << (IsH245TunnelingDisabled() ? "off" : "on") << endl
			  << prefix << " gk Token OID is " << GetGkAccessTokenOID() );


		  // Start the listener thread for incoming calls.
		  PStringArray listeners = PString(_conf.GetString(secure ? "opal/h323s-listen" : "opal/h323-listen")).Lines();
		  PString &s = listeners[0];
		  LogInfo("s:" << (string)s );


		  if (!StartListeners(listeners)) {
			  LogWarn("Could not open any " << prefix << " listener from "
				  << setfill(',') << listeners) ;
			  return API_FAILURE;
		  }
		  LogInfo(prefix << " listeners: " << setfill(',') << GetListeners() << setfill(' '));


		  if (_conf.HasOption("opal/p"))
			  SetGatekeeperPassword(PString(_conf.GetString("opal/p")));

		  // Establish link with gatekeeper if required.
		  if (_conf.HasOption(secure ? "opal/h323s-gk" : "opal/gatekeeper")) {
			  PString gkHost      = _conf.GetString(secure ? "opal/h323s-gk" : "opal/gatekeeper");
			  if (gkHost == "*")
				  gkHost = PString::Empty();
			  PString gkIdentifer = _conf.GetString("opal/G");
			  PString gkInterface = _conf.GetString(secure ? "opal/h323s-listen" : "opal/h323-listen");

			  stringstream s;
			  s << ("Gatekeeper: ");
			  if (UseGatekeeper(gkHost, gkIdentifer, gkInterface))
			  {
				  LogInfo(GetGatekeeper())
			  }
			  else 
			  {
				  s << "none." << endl;
				  s << "Could not register with gatekeeper" << endl;
				  if (!gkIdentifer)
					  s << " id \"" << gkIdentifer << '"';
				  if (!gkHost)
					  s << "at \"" << gkHost << '"' ;
				  if (!gkInterface)
					  s << " on interface \"" << gkInterface << '"';
				  if (GetGatekeeper() != NULL) {
					  switch (GetGatekeeper()->GetRegistrationFailReason()) 
					  {
					  case H323Gatekeeper::InvalidListener :
						  s << " - Invalid listener";
						  break;
					  case H323Gatekeeper::DuplicateAlias :
						  s << " - Duplicate alias";
						  break;
					  case H323Gatekeeper::SecurityDenied :
						  s << " - Security denied";
						  break;
					  case H323Gatekeeper::TransportError :
						  s << " - Transport error";
						  break;
					  default :
						  s << " - Error code " << GetGatekeeper()->GetRegistrationFailReason();
					  } // switch
				  }// if
				  s << '.' << endl;
				  LogWarn(s.str());
				  if (_conf.HasOption("require-gatekeeper")) 
					  return API_FAILURE;
			  }
		  }

		  return API_SUCCESS;
	  }

	  // ***********************************************************************

	  void OnConnectionEstablished(H323Connection & connection, 
		  const PString & token)
	  {
		  PTRACE(1, "Connection established, token is " << token);
	  } 


	  // ***********************************************************************

	  void OnConnectionCleared(H323Connection &connection, 
		  const PString &token)
	  {
		  PTRACE(1, "Connection cleared, token is " << token);
	  } 


	  // ***********************************************************************

	  H323Connection::AnswerCallResponse OnAnswerCall( 
		  H323Connection & connection,
		  const PString & caller, 
		  const H323SignalPDU &, 
		  H323SignalPDU &)
	  {
		  PTRACE(1, "Accepting connection from " << caller);
		  return H323Connection::AnswerCallPending;
	  } 

	 
	  PBoolean OnStartLogicalChannel(H323Connection & connection, 
		  H323Channel & channel)
	  {
		  PString dir;
		  switch (channel.GetDirection())
		  {
		  case H323Channel::IsTransmitter :
			  dir = "sending";
			  break;
		  case H323Channel::IsReceiver :
			  dir = "receiving";
			  break;
		  default :
			  break;
		  }

		  PTRACE(1, "Started logical channel " << dir << " " 
			  << channel.GetCapability() );
		  return true;
	  } 


	  // ***********************************************************************



private:

	Configuration &_conf;

	IocpInterruptorPtr _iocpPtr;

};

ProcOpalH323::ProcOpalH323(Configuration &conf,LpHandlePair pair)
:LightweightProcess(pair,"ProcOpalH323"),
_conf(conf)
{
	FUNCTRACKER;

	ServiceId(_conf.GetString(("opal/uri")));
}

ProcOpalH323::~ProcOpalH323(void)
{
	FUNCTRACKER;
}

void
ProcOpalH323::real_run()
{
	
	FUNCTRACKER;

	// this is regular PTLib initialization code
	// see PCREATE_PROCESS macro for more info
	PreInitialise(NULL, NULL, NULL);
	int terminationValue = InternalMain(); 
	

}


void
ProcOpalH323::Main()
{
	FUNCTRACKER;

	_iocpPtr = IocpInterruptorPtr(new IocpInterruptor());
	_inbound->HandleInterruptor(_iocpPtr);

	IwManager myMngr(_iocpPtr);
	IwH323EndPoint *h323EP  = new IwH323EndPoint(_conf, myMngr,_iocpPtr);

	if (IW_FAILURE(h323EP ->InitialiseH323EP(false)))
	{
		h323EP   = NULL;
		LogWarn("ProcOpalH323::Main - H.323 Initialization failure");

		return;
	}

	I_AM_READY;	

	BOOL shutdown_flag = FALSE;
	while (shutdown_flag == FALSE)
	{

		DWORD number_of_bytes    = 0;
		ULONG_PTR completion_key = 0;
		LPOVERLAPPED lpOverlapped = NULL;

		BOOL res = ::GetQueuedCompletionStatus(
			_iocpPtr->WinHandle(),		// A handle to the completion port. To create a completion port, use the CreateIoCompletionPort function.
			&number_of_bytes,		// A pointer to a variable that receives the number of bytes transferred during an I/O operation that has completed.
			&completion_key,		// A pointer to a variable that receives the completion key value associated with the file handle whose I/O operation has completed. A completion key is a per-file key that is specified in a call to CreateIoCompletionPort.
			&lpOverlapped,			// A pointer to a variable that receives the address of the OVERLAPPED structure that was specified when the completed I/O operation was started. 
			IW_DEFAULT_H323_TIMEOUT // The number of milliseconds that the caller is willing to wait for a completion packet to appear at the completion port. If a completion packet does not appear within the specified time, the function times out, returns FALSE, and sets *lpOverlapped to NULL.
			);

		// error during overlapped I/O?
		int last_err = ::GetLastError();
		if (res == FALSE)
		{
			if (last_err != WAIT_TIMEOUT)
			{
				LogSysError("::GetQueuedCompletionStatus");
				throw;
			}
			else 
			{
				LogInfo("Opal H.323 keep alive.");
				continue;
			}
		}

	
		switch (completion_key)
		{
		case OPAL_EVENT:
			{
				break;
			}
		default:
			{
				if (completion_key!=IOCP_UNIQUE_COMPLETION_KEY)
				{
					LogCrit("Unknown overlapped structure received");
					throw;
				}
			}
		}


		ProcessApplicationMessages();

	}// while

}

bool 
ProcOpalH323::ProcessApplicationMessages()
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
				SendResponse(msg, new MsgCallSubscribeAck());
				break;
			}
		case MSG_CALL_BLIND_XFER_REQ:
			{
				SendResponse(msg, new MsgCallBlindXferNack());
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
ProcOpalH323::UponMakeCallReq(IN IwMessagePtr req)
{
	FUNCTRACKER;

}

void 
ProcOpalH323::UponMakeCallAckReq(IN IwMessagePtr req)
{
	FUNCTRACKER;

}

void 
ProcOpalH323::UponHangupCallReq(IN IwMessagePtr req)
{
	FUNCTRACKER;

}

void 
ProcOpalH323::UponShutDownReq(IN IwMessagePtr req)
{
	FUNCTRACKER;

}

void 
ProcOpalH323::UponCallOfferedAck(IN IwMessagePtr req)
{
	FUNCTRACKER;

}

void 
ProcOpalH323::UponCallOfferedNack(IN IwMessagePtr req)
{
	FUNCTRACKER;

}

}
