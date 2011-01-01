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

#include "IwBase.h"
#include "LpHandle.h"



using namespace boost;
using namespace resip;
using namespace std;

namespace ivrworx
{



	typedef 
	shared_ptr<NameAddr> NameAddrPtr;

	typedef
	shared_ptr<Data> DataPtr;

	typedef
	shared_ptr<HeaderFieldValue> HeaderFieldValuePtr;

	typedef
	shared_ptr<SdpContents> SdpContentsPtr;

	ostream& 
	operator <<(ostream &ostream, const Data &data);

	ostream& 
	operator <<(ostream& strm,  const Message& msg);

	typedef 
	int IwStackHandle;

	struct SipDialogContext :
		public noncopyable
	{
		SipDialogContext();

		InviteSessionHandle invite_handle;

		ServerInviteSessionHandle uas_invite_handle;

		ClientInviteSessionHandle uac_invite_handle;

		LpHandlePtr call_handler_inbound;

		IwStackHandle stack_handle;

		AbstractOffer local_media_offer;

		AbstractOffer remote_media_offer;

	};


	typedef 
	shared_ptr<SipDialogContext> SipDialogContextPtr;

	typedef 
	map<IwStackHandle,SipDialogContextPtr> IwHandlesMap;

	typedef 
	map<AppDialogHandle,SipDialogContextPtr> ResipDialogHandlesMap;


	template<class T> 
	string LogHandleState(SipDialogContextPtr ptr, T is)
	{

		stringstream stream;
		stream << "iwh:" << (ptr.get() == NULL ? -1 : ptr->stack_handle) << ", rsh:"  << is.getId() <<  ", callid:" << is->getCallId().c_str() << ", state:";

		if (is->isAccepted())
		{
			stream << "accepted;";
		}
		if (is->isConnected())
		{
			stream << "connected;";
		}
		if (is->isEarly())
		{
			stream << "early;";
		}
		if (is->isTerminated())
		{
			stream << "terminated;";
		}

		return stream.str();

	}

}





