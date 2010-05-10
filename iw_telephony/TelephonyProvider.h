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

#include "Call.h"

using namespace std;

namespace ivrworx
{

typedef shared_ptr<IMediaCall>
IMediaCallPtr;

class IMediaCallProvider
{
public:

	virtual IMediaCallPtr 
	CreateCall(ScopedForking &forking, shared_ptr<MsgCallOfferedReq> msg) = 0;

	virtual IMediaCallPtr
	CreateCall(ScopedForking &forking) = 0;

	virtual const string& 
	protocolId() = 0;

	virtual const string& 
	providerId() = 0;

};

typedef shared_ptr<IMediaCallProvider> 
IMediaCallProviderPtr;

class TelephonyProviderRepository
{
public:
	TelephonyProviderRepository(void);
	virtual ~TelephonyProviderRepository(void);

	void 
	SetMediaCallProvider(IMediaCallProviderPtr provider);

	IMediaCallProviderPtr  
	GetMediaCallProviderByProtocol(const string& protocol);

	IMediaCallProviderPtr  
	GetMediaCallProviderByProviderId(const string& protocol);

private:

	map<string,IMediaCallProviderPtr> _providersProtocolMap;

	map<string,IMediaCallProviderPtr> _providersIdMap;

};

}

