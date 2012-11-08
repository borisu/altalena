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
#include "RtpProxySession.h"

using namespace ivrworx;
using namespace interop;

namespace ivrworx
{
namespace interop
{

RtpProxySession::RtpProxySession(IvrWORX ^threadCtx):
_impl(NULL)
{
	HandleId service_handle_id = IW_UNDEFINED;
	service_handle_id  = threadCtx->GetServiceHandle(gcnew String("ivr/rtpproxy_service"));

	if (service_handle_id ==  IW_UNDEFINED) 
	{
		throw gcnew Exception("Service not found");
	}

	_impl = new ivrworx::RtpProxySession(*threadCtx->Forking,service_handle_id);

}

RtpProxySession::~RtpProxySession(void)
{
	if (_impl)
		_impl->TearDown();

	delete _impl;

}

ApiErrorCode 
RtpProxySession::Allocate( AbstractOffer ^remoteOffer)
{
	DECLARE_OFFER_FROM_MANAGED(remoteOffer_, remoteOffer);

	ivrworx::ApiErrorCode res = 
		_impl->Allocate(remoteOffer_);

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode 
RtpProxySession::TearDown()
{
	ivrworx::ApiErrorCode res = 
		_impl->TearDown();

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode 
RtpProxySession::Modify( AbstractOffer ^remoteOffer)
{
	DECLARE_OFFER_FROM_MANAGED(remoteOffer_, remoteOffer);

	ivrworx::ApiErrorCode res = 
		_impl->Modify(remoteOffer_);

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode 
RtpProxySession::Bridge(IN  RtpProxySession ^dest, bool fullDuplex)
{
	ivrworx::ApiErrorCode res = 
		_impl->Bridge(*dest->_impl, fullDuplex);

	return ivrworx::interop::ApiErrorCode(res);

}

AbstractOffer^ 
RtpProxySession::LocalOffer()
{
	return gcnew AbstractOffer(_impl->LocalOffer());
}

AbstractOffer^ 
RtpProxySession::RemoteOffer()
{
   return gcnew AbstractOffer(_impl->RemoteOffer());
}


void 
RtpProxySession::CleanDtmfBuffer()
{
	_impl->CleanDtmfBuffer();
}

ApiErrorCode 
RtpProxySession::WaitForDtmf(
	OUT String ^&signal, 
	IN Int32 timeout)
{
	
   string dtmf;

   ivrworx::ApiErrorCode res = 
	 _impl->WaitForDtmf(dtmf, MilliSeconds(timeout));


   signal = gcnew String(dtmf.c_str());

   return ivrworx::interop::ApiErrorCode(res);

}

String^ 
RtpProxySession::DtmfBuffer()
{
	return gcnew String(_impl->DtmfBuffer().c_str());
}



}
}