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
#include "StreamerSession.h"

using namespace ivrworx;
using namespace interop;

namespace ivrworx
{
namespace interop
{




StreamerSession::StreamerSession(IvrWORX ^threadCtx):
_threadCtx(threadCtx),
_impl(NULL)
{
	HandleId service_handle_id = IW_UNDEFINED;
	service_handle_id  = threadCtx->GetServiceHandle(gcnew String("ivr/stream_service"));

	if (service_handle_id ==  IW_UNDEFINED) 
	{
		throw gcnew Exception("Service not found");
	}

	_impl = new StreamingSession(*threadCtx->Forking,service_handle_id);
}

StreamerSession::~StreamerSession(void)
{
	if (_impl)
		_impl->TearDown();

	delete _impl;
}


ApiErrorCode	
StreamerSession::Allocate(IN  AbstractOffer ^remoteOffer,
				IN RcvDeviceType rcvDeviceType,
				IN SndDeviceType sndDeviceType)
{
	DECLARE_OFFER_FROM_MANAGED(remoteOffer_, remoteOffer);

	ivrworx::ApiErrorCode res = 
		_impl->Allocate(remoteOffer_, 
		(ivrworx::RcvDeviceType) rcvDeviceType,
		(ivrworx::SndDeviceType) sndDeviceType);

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode	
StreamerSession::ModifyConnection(
	IN  AbstractOffer ^remoteOffer)
{
	DECLARE_OFFER_FROM_MANAGED(remoteOffer_, remoteOffer);

	ivrworx::ApiErrorCode res = 
		_impl->ModifyConnection(remoteOffer_);

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode	
StreamerSession::StopPlay()
{
	ivrworx::ApiErrorCode res = 
		_impl->StopPlay();

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode 
StreamerSession::PlayFile(IN  String ^fileName,
					IN bool sync,
					IN bool loop)
{
	ivrworx::ApiErrorCode res = 
		_impl->PlayFile(MarshalToString(fileName),sync,loop);

	return ivrworx::interop::ApiErrorCode(res);

}

void 
StreamerSession::TearDown()
{
	_impl->TearDown();
}

AbstractOffer^ 
StreamerSession::LocalOffer()
{
	return gcnew AbstractOffer(_impl->LocalOffer());

}

AbstractOffer^ 
StreamerSession::RemoteOffer()
{
	return gcnew AbstractOffer(_impl->RemoteOffer());

}

}
}