#include "StdAfx.h"
#include "ProcOpalH323.h"

namespace ivrworx
{


	class MyManager : public OpalManager
	{
		PCLASSINFO(MyManager, OpalManager)

	public:

		virtual PBoolean 
		OnIncomingConnection(
			IN OpalConnection & connection,   ///<  Connection that is calling
			IN unsigned options,              ///<  options for new connection (can't use default as overrides will fail)
			IN OpalConnection::StringOptions * stringOptions)
		{
			return PTrue;
		};

		virtual void OnClearedCall(OpalCall & call){}; // Callback override

		PSyncPoint m_completed;
	};


	ApiErrorCode
	InitialiseH323EP(Configuration &args, PBoolean secure, H323EndPoint *h323EP)
	{
		h323EP->DisableFastStart(args.HasOption("opal/f"));
		h323EP->DisableH245Tunneling(args.HasOption("opal/T"));
		h323EP->SetSendGRQ(!args.HasOption("opal/disable-grq"));


		// Get local username, multiple uses of -u indicates additional aliases
		if (args.HasOption("opal/u")) {
			PStringArray aliases = PString(args.GetString("opal/u")).Lines();
			h323EP->SetLocalUserName(aliases[0]);
			for (PINDEX i = 1; i < aliases.GetSize(); i++)
				h323EP->AddAliasName(aliases[i]);
		}

		if (args.HasOption("opal/b")) {
			unsigned initialBandwidth =  PString(args.GetString("opal/b")).AsUnsigned()*100;
			if (initialBandwidth == 0) {
				LogWarn ("InitialiseH323EP - Illegal bandwidth specified.");
				return API_FAILURE;
			}
			h323EP->SetInitialBandwidth(initialBandwidth);
		}

		if (args.HasOption("opal/gk-token"))
			h323EP->SetGkAccessTokenOID(PString(args.GetString("opal/gk-token")));

		PString prefix = h323EP->GetPrefixName();

		LogInfo ( prefix << " Local username: " << h323EP->GetLocalUserName() << endl
			<< prefix << " FastConnect is " << (h323EP->IsFastStartDisabled() ? "off" : "on") << endl
			<< prefix << " H245Tunnelling is " << (h323EP->IsH245TunnelingDisabled() ? "off" : "on") << endl
			<< prefix << " gk Token OID is " << h323EP->GetGkAccessTokenOID() );


		// Start the listener thread for incoming calls.
		PStringArray listeners = PString(args.GetString(secure ? "opal/h323s-listen" : "opal/h323-listen")).Lines();
		PString &s = listeners[0];
		LogInfo("s:" << (string)s );


		if (!h323EP->StartListeners(listeners)) {
			LogWarn("Could not open any " << prefix << " listener from "
				<< setfill(',') << listeners) ;
			return API_FAILURE;
		}
		LogInfo(prefix << " listeners: " << setfill(',') << h323EP->GetListeners() << setfill(' '));


		if (args.HasOption("opal/p"))
			h323EP->SetGatekeeperPassword(PString(args.GetString("opal/p")));

		// Establish link with gatekeeper if required.
		if (args.HasOption(secure ? "opal/h323s-gk" : "opal/gatekeeper")) {
			PString gkHost      = args.GetString(secure ? "opal/h323s-gk" : "opal/gatekeeper");
			if (gkHost == "*")
				gkHost = PString::Empty();
			PString gkIdentifer = args.GetString("opal/G");
			PString gkInterface = args.GetString(secure ? "opal/h323s-listen" : "opal/h323-listen");

			stringstream s;
			s << ("Gatekeeper: ");
			if (h323EP->UseGatekeeper(gkHost, gkIdentifer, gkInterface))
			{
				LogInfo(h323EP->GetGatekeeper())
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
				if (h323EP->GetGatekeeper() != NULL) {
					switch (h323EP->GetGatekeeper()->GetRegistrationFailReason()) 
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
							s << " - Error code " << h323EP->GetGatekeeper()->GetRegistrationFailReason();
					} // switch
				}// if
				s << '.' << endl;
				LogWarn(s.str());
				if (args.HasOption("require-gatekeeper")) 
					return API_FAILURE;
				}
			}
		return API_SUCCESS;
	}


	ProcOpalH323::ProcOpalH323(Configuration &conf,LpHandlePair pair)
	:LightweightProcess(pair,"ProcOpalH323"),
	_conf(conf)
	{
		FUNCTRACKER;
	}

	ProcOpalH323::~ProcOpalH323(void)
	{
		FUNCTRACKER;
	}

	void
	ProcOpalH323::real_run()
	{
		
		// this is regular PTLib initialization code
		// see PCREATE_PROCESS macro for more info
		PreInitialise(NULL, NULL, NULL);
		int terminationValue = InternalMain(); 
		

	}


	void
	ProcOpalH323::Main()
	{
		FUNCTRACKER;

		
		shared_ptr<MyManager> _myMngr (new MyManager());
		H323EndPoint *_h323EP  = new H323EndPoint(*_myMngr);

		if (IW_FAILURE(InitialiseH323EP(_conf, false, _h323EP)))
		{
			_myMngr->ShutDownEndpoints();
			_h323EP   = NULL;

			LogWarn("ProcOpalH323::Main - H.323 Initialization failure");
			return;
		}

		I_AM_READY;	

		::Sleep(INFINITE);

		
		

	}

}
