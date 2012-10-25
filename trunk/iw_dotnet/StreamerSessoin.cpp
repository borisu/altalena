#include "StdAfx.h"
#include "StreamerSession.h"

using namespace ivrworx;
using namespace interop;

namespace ivrworx
{
namespace interop
{




StreamerSession::StreamerSession(IvrWORX ^threadCtx):
_threadCtx(threadCtx)
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