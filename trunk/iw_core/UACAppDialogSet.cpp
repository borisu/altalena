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
#include "UACAppDialogSet.h"
#include "UACAppDialog.h"
#include "CcuLogger.h"



UACAppDialogSet::UACAppDialogSet(DialogUsageManager& dum, SipDialogContextPtr ptr): 
AppDialogSet(dum),
_ptr(ptr)
{
	FUNCTRACKER;;
}

UACAppDialogSet::~UACAppDialogSet(void)
{
	FUNCTRACKER;;
}

AppDialog* 
UACAppDialogSet::createAppDialog(const SipMessage& msg) 
{  
	FUNCTRACKER;;
	return new UACAppDialog(mDum);  
}

SharedPtr<UserProfile> 
UACAppDialogSet::selectUACUserProfile(const SipMessage& msg) 
{ 
	FUNCTRACKER;;
	return mDum.getMasterUserProfile(); 
}