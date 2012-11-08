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
		public ref class StreamerSession : public IActiveObject
		{
		public:
			enum struct SndDeviceType
			{
				SND_DEVICE_TYPE_FILE,
				SND_DEVICE_TYPE_SND_CARD_MIC,
				SND_DEVICE_TYPE_SND_CARD_LINE
			};

			enum struct RcvDeviceType
			{
				RCV_DEVICE_NONE,
				RCV_DEVICE_FILE_REC_ID,
				RCV_DEVICE_WINSND_WRITE
			};

		public:
			StreamerSession(IvrWORX ^threadCtx);

			virtual ~StreamerSession(void);

			virtual ApiErrorCode	Allocate(
				IN  AbstractOffer ^remoteOffer,
				IN RcvDeviceType rcvDeviceType,
				IN SndDeviceType sndDeviceType);

			virtual ApiErrorCode	ModifyConnection(
				IN  AbstractOffer ^remoteOffer);

			virtual ApiErrorCode	StopPlay();

			virtual ApiErrorCode PlayFile(
				IN  String ^fileName,
				IN bool sync ,
				IN bool loop );

			virtual void TearDown();

			virtual AbstractOffer^ LocalOffer();

			virtual AbstractOffer^ RemoteOffer();

			virtual ivrworx::ActiveObject *GetOpaqueObject()
			{
				return _impl;
			}

		private:

			IvrWORX ^_threadCtx;

			ivrworx::StreamingSession *_impl;

		};

	}

}


