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

#include "stdafx.h"
#include "RtspSession.h"

using namespace ivrworx;
using namespace interop;

namespace ivrworx
{
namespace interop
{
		

RtspSession::RtspSession(IvrWORX ^threadCtx)
{
	HandleId service_handle_id = IW_UNDEFINED;
	service_handle_id  = threadCtx->GetServiceHandle(gcnew String("ivr/rtsp_service"));

	if (service_handle_id ==  IW_UNDEFINED) 
	{
		throw gcnew Exception("Service not found");
	}

	_impl = new ivrworx::RtspSession(*threadCtx->Forking,service_handle_id);


}

RtspSession::~RtspSession(void)
{
	if (_impl)
		_impl->TearDown();

}

ApiErrorCode 
RtspSession::Setup(IN  String ^rtsp_url, IN  AbstractOffer ^offer)
{
 DECLARE_OFFER_FROM_MANAGED(offer_, offer);
 
 ivrworx::ApiErrorCode res = 
	 _impl->Setup(MarshalToString(rtsp_url),offer_);

 return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode 
RtspSession::Play(double start_time , double duration , double scale )
 {
	 ivrworx::ApiErrorCode res = 
		 _impl->Play(start_time,duration,scale);

	 return ivrworx::interop::ApiErrorCode(res);

 }

ApiErrorCode 
RtspSession::Pause()
{
 ivrworx::ApiErrorCode res = 
	 _impl->Pause();

 return ivrworx::interop::ApiErrorCode(res);

}

 ApiErrorCode 
RtspSession::TearDown()
{
 ivrworx::ApiErrorCode res = 
	 _impl->TearDown();

 return ivrworx::interop::ApiErrorCode(res);

}

AbstractOffer^
RtspSession::RemoteOffer()
{
	return gcnew AbstractOffer(_impl->RemoteOffer());

}

AbstractOffer^ 
RtspSession::LocalOffer()
{
	return gcnew AbstractOffer(_impl->LocalOffer());

}


	
}
}