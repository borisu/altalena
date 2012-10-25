
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
#include "AbstractOffer.h"


using namespace ivrworx;
using namespace interop;

namespace ivrworx
{
	namespace interop
	{
		public ref class RtpProxySession
		{

		public:
			RtpProxySession(IvrWORX ^threadCtx);

			virtual ~RtpProxySession(void);

			virtual ApiErrorCode Allocate(AbstractOffer ^remoteOffer);

			virtual ApiErrorCode TearDown();

			virtual ApiErrorCode Modify(AbstractOffer ^remoteOffer);

			virtual ApiErrorCode Bridge(IN  RtpProxySession ^dest, bool fullDuplex);

			virtual AbstractOffer^ LocalOffer();

			virtual AbstractOffer^ RemoteOffer();

			virtual void CleanDtmfBuffer();

			virtual ApiErrorCode WaitForDtmf(
				OUT String ^&signal, 
				IN Int32 timeout);

			virtual String^ DtmfBuffer();

		private:

			IvrWORX ^_threadCtx;

			ivrworx::RtpProxySession *_impl;

		};

	}

}