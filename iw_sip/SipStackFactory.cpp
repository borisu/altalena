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
#include "SipStackFactory.h"
#include "ProcSipStack.h"

namespace ivrworx
{
	IMediaCallPtr 
	ResiprocateProvider::CreateCall(ScopedForking &forking, shared_ptr<MsgCallOfferedReq> msg)
	{
		return IMediaCallPtr(new SipMediaCall(forking,msg));

	}

	IMediaCallPtr 
	ResiprocateProvider::CreateCall(ScopedForking &forking)
	{
		return IMediaCallPtr(new SipMediaCall(forking));

	}

	const string& 
	ResiprocateProvider::protocolId()
	{
		static string protocol_id = "sip";
		return protocol_id;
	}

	SipStackFactory::SipStackFactory(void)
	{
	}

	SipStackFactory::~SipStackFactory(void)
	{
	}

	LightweightProcess*
	SipStackFactory::Create(IN LpHandlePair stack_pair, IN Configuration &conf)
	{

		return new ProcSipStack(
			stack_pair, 
			conf);

	}


}

