#include "StdAfx.h"
#include "ProcOpalH323.h"

class MyManager : public OpalManager
{
	PCLASSINFO(MyManager, OpalManager)

public:
	virtual void OnClearedCall(OpalCall & call){}; // Callback override

	PSyncPoint m_completed;
};


namespace ivrworx
{

	ProcOpalH323::ProcOpalH323(Configuration &conf,LpHandlePair pair)
	:LightweightProcess(pair,"ProcOpalH323"),
	_conf(conf),
	_h323EP(NULL)
	{
		FUNCTRACKER;
		MyManager m;
	}

	ProcOpalH323::~ProcOpalH323(void)
	{
		FUNCTRACKER;
	}

	void
	ProcOpalH323::real_run()
	{
		FUNCTRACKER;

// 		if (!_conf.HasOption("no-h323")) {
// 			_h323EP = new H323EndPoint(*this);
// 			if (!InitialiseH323EP(args, false, h323EP))
// 				return PFalse;
// 		}

	}

}
