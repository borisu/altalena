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
#include "SipCall.h"

namespace ivrworx{
namespace interop{


SipCall::SipCall(IvrWORX ^threadCtx):
_threadCtx(threadCtx)
{
	

	
	HandleId service_handle_id = IW_UNDEFINED;
	service_handle_id  = threadCtx->GetServiceHandle(gcnew String("ivr/sip_service"));

	 if (service_handle_id ==  IW_UNDEFINED) 
	 {
		 throw gcnew Exception("Service not found");
	 }
	
	_impl = new SipMediaCall(*threadCtx->Forking,service_handle_id);
}

SipCall::~SipCall()
{
	if (_impl)
		_impl->HangupCall();
}


void SipCall::CleanDtmfBuffer()
{
	return _impl->CleanDtmfBuffer();

}

ApiErrorCode   
SipCall::WaitForDtmf(OUT String ^%signal, 
					 IN  Int32 timeout)
{
	string my_signal;

	ivrworx::ApiErrorCode  
		res= _impl->WaitForDtmf(my_signal, MilliSeconds((int)timeout));

	signal = gcnew String(my_signal.c_str());

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode   
SipCall::RejectCall()
{
	ivrworx::ApiErrorCode  
		res= _impl->RejectCall();

	return ivrworx::interop::ApiErrorCode(res);
}

ApiErrorCode   
SipCall::HangupCall()
{
	ivrworx::ApiErrorCode  
		res= _impl->HangupCall();

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode   
SipCall::BlindXfer(IN String ^destination_uri)
{
	

	ivrworx::ApiErrorCode  
		res= _impl->BlindXfer(MarshalToString(destination_uri));

	return ivrworx::interop::ApiErrorCode(res);

}

void 
SipCall::WaitTillHangup()
{
	
	_impl->WaitTillHangup();

}

String^ 
SipCall::Dnis()
{
	
	return gcnew String(_impl->Dnis().c_str());

}

String^ 
SipCall::Ani()
{
	return gcnew String(_impl->Ani().c_str());

}

AbstractOffer^ 
SipCall::LocalOffer()
{
	return gcnew AbstractOffer(_impl->LocalOffer());
}

AbstractOffer^ 
SipCall::RemoteOffer()
{
	return gcnew AbstractOffer(_impl->RemoteOffer());
}

ApiErrorCode   
SipCall::MakeCall(
					  IN  String^						destinationUri, 
					  IN  AbstractOffer^				localOffer,
					  IN  Credentials^					credentials, 
					  IN OUT  MapOfAnyInterop^			keyValueMap,
					  IN Int32							ringTimeout)
{

	DECLARE_MAPOFANY_FROM_MANAGED(keyValueMap_,keyValueMap);
	DECLARE_OFFER_FROM_MANAGED(localOffer_, localOffer);
	DECLARE_CREDENTIALS_FROM_MANAGED(credentials_,credentials);
	

	ivrworx::ApiErrorCode res = 
		_impl->MakeCall(
			MarshalToString(destinationUri), 
			localOffer_,
			credentials_,
			keyValueMap_,
			MilliSeconds(ringTimeout)
			);

	if (keyValueMap != nullptr)
		keyValueMap->Clear();

	FillManagedTable(keyValueMap,keyValueMap_);

	return ivrworx::interop::ApiErrorCode(res);
}

ApiErrorCode   
SipCall::ReOffer(
					 IN  AbstractOffer^			localOffer,
					 IN OUT MapOfAnyInterop^	keyValueMap,
					 IN Int32					ringTimeout)
{
	DECLARE_MAPOFANY_FROM_MANAGED(keyValueMap_,keyValueMap);
	DECLARE_OFFER_FROM_MANAGED(offer, localOffer);

	ivrworx::ApiErrorCode res = 
		_impl->ReOffer(offer,keyValueMap_,MilliSeconds(ringTimeout));

	if (keyValueMap != nullptr)
		keyValueMap->Clear();
	FillManagedTable(keyValueMap,keyValueMap_);

	
	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode   
SipCall::Answer(	IN  AbstractOffer^					 localOffer,
					IN  OUT MapOfAnyInterop^ keyValueMap,
					IN Int32	  ringTimeout)
{
	DECLARE_MAPOFANY_FROM_MANAGED(keyValueMap_,keyValueMap);
	DECLARE_OFFER_FROM_MANAGED(localOffer_, localOffer);

	ivrworx::ApiErrorCode res = 
		_impl->Answer(localOffer_,keyValueMap_,MilliSeconds(ringTimeout));

	if (keyValueMap != nullptr)
		keyValueMap->Clear();

	FillManagedTable(keyValueMap,keyValueMap_);

	return ivrworx::interop::ApiErrorCode(res);


}

String^ 
SipCall::DtmfBuffer()
{

	return gcnew String(_impl->DtmfBuffer().c_str());

}

ApiErrorCode   
SipCall::SendInfo(	IN	 AbstractOffer	^offer, 
					OUT AbstractOffer	^response, 
					bool async)
{
	DECLARE_OFFER_FROM_MANAGED(offer_, offer);
	DECLARE_OFFER_FROM_MANAGED(response_,response);

	ivrworx::ApiErrorCode res = 
		_impl->SendInfo(offer_,response_,async);

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode   
SipCall::WaitForInfo(OUT AbstractOffer ^offer)
{
	DECLARE_OFFER_FROM_MANAGED(offer_, offer);

	ivrworx::ApiErrorCode res = 
		_impl->WaitForInfo(offer_);

	return ivrworx::interop::ApiErrorCode(res);

}


void 
SipCall::CleanInfoBuffer()
{
	_impl->CleanDtmfBuffer();

}

ApiErrorCode   
SipCall::StartRegistration( IN ListOfStrings	^contacts, 
							IN String			^registrar,
							IN Credentials		^credentials, 
							IN Int32			 timeout)
{

	DECLARE_CONTACTSLIST_FROM_MANAGED(contacts_, contacts);
	DECLARE_CREDENTIALS_FROM_MANAGED(credentials_, credentials);

	ivrworx::ApiErrorCode res = 
		_impl->StartRegistration(
		contacts_, 
		MarshalToString(registrar),
		credentials_, 
		MilliSeconds(timeout));

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode   
SipCall::Subscribe(	IN  String			^eventserver,
					IN  ListOfStrings 	^contacts, 
					IN  Credentials		^credentials, 
					IN  AbstractOffer	^offer,
					IN  String			^eventsPackage,
					IN  Int32			 refreshInterval,
					IN  Int32			 subscriptionTime,
					IN  Int32			 timeout)
{
	DECLARE_CONTACTSLIST_FROM_MANAGED(contacts_, contacts);
	DECLARE_CREDENTIALS_FROM_MANAGED(credentials_, credentials);
	DECLARE_OFFER_FROM_MANAGED(offer_, offer);

	ivrworx::ApiErrorCode res = 
		_impl->Subscribe(
			MarshalToString(eventserver),
			contacts_, 
			credentials_, 
			offer_,
			MarshalToString(eventsPackage),
			refreshInterval,
			subscriptionTime,
			MilliSeconds(timeout)
		);

	return ivrworx::interop::ApiErrorCode(res);


}

ApiErrorCode   
SipCall::WaitForNotify(OUT AbstractOffer ^offer)
{
	DECLARE_OFFER_FROM_MANAGED(offer_, offer);

	ivrworx::ApiErrorCode res = 
		_impl->WaitForNotify(offer_);

	return ivrworx::interop::ApiErrorCode(res);
}

void 
SipCall::CleanNotifyBuffer()
{
	_impl->CleanNotifyBuffer();

}


}
}