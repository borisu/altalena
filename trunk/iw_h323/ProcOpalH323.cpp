#include "StdAfx.h"
#include "ProcOpalH323.h"

namespace ivrworx
{


	class IwManager : public OpalManager
	{
		PCLASSINFO(IwManager, OpalManager)
	public:

		IwManager()
		{
			
		}

		virtual ~IwManager()
		{
			ShutDownEndpoints();
		}
		

	};

	class IwH323EndPoint : public H323EndPoint
	{
		PCLASSINFO(IwH323EndPoint, H323EndPoint);
	public:

		IwH323EndPoint(Configuration &conf, IwManager &manager):
		  H323EndPoint(manager),
		  _conf(conf)
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

			  GetManager().AddRouteEntry("h323:.* = pots:<du>");

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

	private:
		Configuration &_conf;
	
	};


	class IwH323Connection : public H323Connection
	{
		PCLASSINFO(IwH323Connection, H323Connection);

	public:
	};



	

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

		shared_ptr<IwManager> _myMngr (new IwManager());
		IwH323EndPoint *_h323EP  = new IwH323EndPoint(_conf, *_myMngr);

		if (IW_FAILURE(_h323EP ->InitialiseH323EP(false)))
		{
			_h323EP   = NULL;
			LogWarn("ProcOpalH323::Main - H.323 Initialization failure");

			return;
		}

		I_AM_READY;	

		::Sleep(INFINITE);



		
		
		
		

	}

}
