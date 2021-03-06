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
#pragma once

#include "ResipCommon.h"
#include "SipSessionHandlerAdapter.h"


using namespace resip;
using namespace std;

namespace ivrworx
{

class IwAppDialogSetFactory :
	public AppDialogSetFactory
{
public:
	IwAppDialogSetFactory(void);
	virtual ~IwAppDialogSetFactory(void);

	// For a UAS the testAppDialogSet will be created by DUM using this function.  If you want to set 
	// Application Data, then one approach is to wait for onNewSession(ServerInviteSessionHandle ...) 
	// to be called, then use the ServerInviteSessionHandle to get at the AppDialogSet or AppDialog,
	// then cast to your derived class and set the desired application data.
	virtual AppDialogSet* createAppDialogSet(DialogUsageManager& dum, const SipMessage& msg);


};

}
