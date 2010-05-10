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
#include "TelephonyProvider.h"

namespace ivrworx
{



TelephonyProviderRepository::TelephonyProviderRepository(void)
{
}

TelephonyProviderRepository::~TelephonyProviderRepository(void)
{
}

void
TelephonyProviderRepository::SetMediaCallProvider(IMediaCallProviderPtr provider)
{
	_providersProtocolMap[provider->protocolId()] = provider;
	_providersIdMap[provider->protocolId()] = provider;

}


IMediaCallProviderPtr  
TelephonyProviderRepository::GetMediaCallProviderByProtocol(const string& protocol)
{

	if (_providersProtocolMap.find(protocol) == _providersProtocolMap.end())
	{
		return IMediaCallProviderPtr();
	}
	else 
	{
		return _providersProtocolMap[protocol]; 
	}

}

IMediaCallProviderPtr  
TelephonyProviderRepository::GetMediaCallProviderByProviderId(const string& protocol)
{

	if (_providersIdMap.find(protocol) == _providersIdMap.end())
	{
		return IMediaCallProviderPtr();
	}
	else 
	{
		return _providersIdMap[protocol]; 
	}

}

}