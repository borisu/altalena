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
	_impl = new SipMediaCall(*threadCtx->Forking);
}

void SipCall::CleanDtmfBuffer()
{
	return _impl->CleanDtmfBuffer();

}

ApiErrorCode   
SipCall::WaitForDtmf(
						 OUT String ^&signal, 
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
SipCall::BlindXfer(IN const String ^destination_uri)
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
					  IN const String^						destinationUri, 
					  IN const AbstractOffer^				localOffer,
					  IN const Credentials^					credentials, 
					  IN OUT  Dictionary<String^,String^>^	keyValueMap,
					  IN Int32								ringTimeout)
{

	MapOfAny map;

	for each (String^ key in keyValueMap->Keys)
	{
		map[MarshalToString(key)] = MarshalToString(keyValueMap[key]);
	};

	localOffer->Body;

	MarshalToString((const String^)localOffer->Body);

	ivrworx::AbstractOffer offer(
		MarshalToString(localOffer->Body), 
		MarshalToString(localOffer->Type));

	ivrworx::ApiErrorCode res = 
		_impl->MakeCall(
			MarshalToString(destinationUri), 
			offer,
			ivrworx::Credentials(MarshalToString(credentials->User), MarshalToString(credentials->Password), MarshalToString(credentials->Realm)),
			map,
			MilliSeconds(ringTimeout)
			));

	return ivrworx::interop::ApiErrorCode(res);
}

ApiErrorCode   
SipCall::ReOffer(
					 IN const AbstractOffer^				localOffer,
					 IN OUT  Dictionary<String^,Object^>^	keyValueMap,
					 IN Int32							ringTimeout)
{

}

ApiErrorCode   
SipCall::Answer(
					IN const AbstractOffer^					 localOffer,
					IN const OUT  Dictionary<String^,Object^>^ keyValueMap,
					IN Int32	  ringTimeout)
{

}

String^ 
SipCall::DtmfBuffer()
{

}

ApiErrorCode   
SipCall::SendInfo(	IN	const AbstractOffer	^offer, 
					OUT AbstractOffer		^response, 
					bool async)
{

}

ApiErrorCode   
SipCall::WaitForInfo(OUT AbstractOffer ^offer)
{

}


void 
SipCall::CleanInfoBuffer()
{

}

ApiErrorCode   
SipCall::StartRegistration( IN const ListOfStrings	^contacts, 
							IN const String			^registrar,
							IN const Credentials	^credentials, 
							IN Int32				timeout)
{

}

ApiErrorCode   
SipCall::Subscribe(	IN const String			^eventserver,
					IN const ListOfStrings 	^contacts, 
					IN const Credentials	^credentials, 
					IN const AbstractOffer	^offer,
					IN const String			^eventsPackage,
					IN Int32			refreshInterval,
					IN Int32			subscriptionTime,
					IN Int32			timeout)
{

}

ApiErrorCode   
SipCall::WaitForNotify( OUT AbstractOffer		^offer)
{

}

void 
SipCall::CleanNotifyBuffer()
{

}

ApiErrorCode   
SipCall::MakeCall(	IN const String			^destination_uri, 
					IN const AbstractOffer	^offer,
					IN const Credentials		^credentials,
					IN OUT MapOfAnyInterop	^key_value_map,
					IN Int32					^ring_timeout)
{

}


}
}