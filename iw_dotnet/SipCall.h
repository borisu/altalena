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
#include "GenericOfferAnswerSession.h"
#include "..\iw_telephony\SipCall.h"

namespace ivrworx
{
	namespace interop 
	{

		public ref class SipCall : public IGenericOfferAnswerSession
		{
		public:
			SipCall(IvrWORX ^threadCtx);

			virtual void CleanDtmfBuffer();

			virtual ApiErrorCode WaitForDtmf(
				OUT String ^signal, 
				IN  Int32 timeout);

			virtual ApiErrorCode RejectCall();

			virtual ApiErrorCode HangupCall();

			virtual ApiErrorCode BlindXfer(
				IN const String ^destination_uri);

			virtual void WaitTillHangup();

			virtual String^ Dnis();

			virtual String^ Ani();

			virtual AbstractOffer^ LocalOffer();

			virtual AbstractOffer^ RemoteOffer();

			virtual ApiErrorCode MakeCall(
				IN const String^					destinationUri, 
				IN const AbstractOffer^				localOffer,
				IN const Credentials^				credentials, 
				IN OUT  Dictionary<String^,Object^>^	keyValueMap,
				IN Int32							ringTimeout);

			virtual ApiErrorCode ReOffer(
				IN const AbstractOffer^				localOffer,
				IN OUT  Dictionary<String^,Object^>^	keyValueMap,
				IN Int32							ringTimeout);

			virtual ApiErrorCode Answer(
				IN const AbstractOffer^					 localOffer,
				IN const OUT  Dictionary<String^,Object^>^ keyValueMap,
				IN Int32	  ringTimeout);

			virtual String^ DtmfBuffer();

			virtual ApiErrorCode SendInfo(
				IN	const AbstractOffer	^offer, 
				OUT AbstractOffer		^response, 
				IN	bool async);

			virtual ApiErrorCode WaitForInfo(OUT AbstractOffer ^offer);

			virtual void CleanInfoBuffer();

			virtual ApiErrorCode StartRegistration(
				IN const ListOfStrings ^contacts, 
				IN const String^		registrar,
				IN const Credentials^   credentials, 
				IN Int32				timeout);

			virtual ApiErrorCode Subscribe(
				IN const String^		eventserver,
				IN const ListOfStrings^ contacts, 
				IN const Credentials^	credentials, 
				IN const AbstractOffer^	offer,
				IN const String^		eventsPackage,
				IN Int32			refreshInterval,
				IN Int32			subscriptionTime,
				IN Int32			timeout);

			virtual ApiErrorCode WaitForNotify(
				OUT AbstractOffer		^offer);

			virtual void CleanNotifyBuffer();

			virtual ApiErrorCode MakeCall(
				IN const String			^destination_uri, 
				IN const AbstractOffer	^offer,
				IN const Credentials	^credentials,
				IN OUT MapOfAnyInterop	^key_value_map,
				IN Int32				^ring_timeout);

		private:

			IvrWORX ^_threadCtx;

			ivrworx::SipMediaCall *_impl;

		};

	

	}
}
