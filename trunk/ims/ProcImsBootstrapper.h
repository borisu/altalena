#pragma once
#include "procccufacade.h"

//
// THIS CLASS IS ADDED TO HIDE ADDITIONAL IMS 
// DEPENDENCIES SO WHOEVER INCLUDES IMS DOES 
// NOT HAVE TO INCLUDE ALL ITS LIBRARY DEPENDENCIES
//
// !!! DO NOT PUT INCLUDES HERE !!!
//

class ProcImsBootstrapper :
	public ProcCcuFacade
{
public:
	ProcImsBootstrapper(LpHandlePair pair);

	void real_run();

	virtual ~ProcImsBootstrapper(void);
};
