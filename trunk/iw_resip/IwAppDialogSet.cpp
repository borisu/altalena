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
#include "IwAppDialogSet.h"

using namespace resip;
using namespace std;

namespace ivrworx
{
	IwAppDialogSet::IwAppDialogSet(
		IN DialogUsageManager& dum,
		IN SipDialogContextPtr ptr,
		IN IwMessagePtr origRequest):
		AppDialogSet(dum),
		dialog_ctx(ptr)
	{
		// dialog opening request
		switch (origRequest->message_id)
		{
			case SIP_CALL_SUBSCRIBE_REQ:
				{
					last_subscribe_req = origRequest;
					break;
				}
			case MSG_MAKE_CALL_REQ:
				{
					last_makecall_req = origRequest;
					break;
				}
			default:
				{
					throw std::exception("unknown request");
				}
		}
	}

	SharedPtr<UserProfile> 
	IwAppDialogSet::getUserProfile()
	{
		if (dialog_ctx->user_profile)
			return dialog_ctx->user_profile;
		else
			return AppDialogSet::getUserProfile();
	}

	IwAppDialogSet::IwAppDialogSet(IN DialogUsageManager& dum):
	AppDialogSet(dum)
	{
	}

	IwAppDialogSet::~IwAppDialogSet(void)
	{
	}
}

