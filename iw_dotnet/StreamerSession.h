#pragma once
#include "IvrWORX.h"
#include "AbstractOffer.h"


namespace ivrworx
{
	namespace interop
	{
		public ref class StreamerSession
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

		private:

			IvrWORX ^_threadCtx;

			ivrworx::StreamingSession *_impl;

		};

	}

}


