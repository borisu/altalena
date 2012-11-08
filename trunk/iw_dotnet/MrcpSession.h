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
#include "IvrWORX.h"
#include "IActiveObject.h"
#include "AbstractOffer.h"


namespace ivrworx
{
	namespace interop
	{
		public enum struct MrcpResource
		{
			UKNOWN_MRCP_RESOURCE,
			SYNTHESIZER,
			RECOGNIZER
		};

		typedef MapOfAnyInterop 
		MrcpParams;
		
		public ref class MrcpSession : 
			public IActiveObject
		{
		public:
			MrcpSession (IN IvrWORX ^forking);

			virtual ~MrcpSession (void);

			virtual ApiErrorCode	Allocate(IN MrcpResource rsc);

			virtual ApiErrorCode	Allocate(
				IN MrcpResource rsc,
				IN AbstractOffer ^localOffer, 
				IN Int32 timeout);

			virtual ApiErrorCode	ModifySession(
				IN AbstractOffer ^remote_offer, 
				IN Int32 timeout);

			virtual ApiErrorCode	StopSpeak();

			virtual ApiErrorCode Speak(
				IN  MrcpParams ^rsc,
				IN  String ^rawBody,
				IN  bool sync );

			virtual ApiErrorCode SimpleSpeak(
				IN  MrcpParams ^rsc,
				IN  String ^sentence,
				IN  bool sync );

			virtual ApiErrorCode Recognize(
				IN  MrcpParams ^p,
				IN  String ^body,
				IN  Int32 timeout, 
				IN  bool sync, 
				[Out] String^%answer);

			virtual ApiErrorCode WaitForRecogResult(
				IN  Int32 timeout,
				[Out] String^%answer);

			virtual ApiErrorCode DefineGrammar(
				IN  MrcpParams ^p,
				IN  String ^body,
				IN  Int32 timeout);

			virtual void TearDown();

			AbstractOffer^ RemoteOffer(MrcpResource rsc);

			AbstractOffer^ LocalOffer(MrcpResource rsc);

			virtual ivrworx::ActiveObject *GetOpaqueObject()
			{
				return _impl;
			}

		private:

			ivrworx::MrcpSession *_impl;
		};
	}
}
