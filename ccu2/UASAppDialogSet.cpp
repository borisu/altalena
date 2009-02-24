#include "StdAfx.h"
#include "UASAppDialogSet.h"
#include "UASAppDialog.h"
#include "Message.h"
#include "Logger.h"

namespace ivrworx
{
	UASAppDialogSet::UASAppDialogSet(DialogUsageManager& dum, Data SampleAppData): 
	AppDialogSet(dum)
	{
		FUNCTRACKER;;
	}

	UASAppDialogSet::~UASAppDialogSet(void)
	{
		FUNCTRACKER;;
	}

	AppDialog* 
	UASAppDialogSet::createAppDialog(const SipMessage& msg) 
	{  
		FUNCTRACKER;;
		return new UASAppDialog(mDum);  
	}

	SharedPtr<UserProfile> 
	UASAppDialogSet::selectUASUserProfile(const SipMessage& msg) 
	{ 
		FUNCTRACKER;;
		return mDum.getMasterUserProfile(); 
	}

}

