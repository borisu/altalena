#include "StdAfx.h"
#include "RtpProxySession.h"

using namespace ivrworx;
using namespace interop;

namespace ivrworx
{
namespace interop
{

RtpProxySession::RtpProxySession(IvrWORX ^threadCtx)
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