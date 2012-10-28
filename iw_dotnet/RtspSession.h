#pragma once
#include "IvrWORX.h"
#include "AbstractOffer.h"


using namespace ivrworx;
using namespace interop;

namespace ivrworx
{
	namespace interop
	{
		public ref class RtspSession
		{
		public:

			RtspSession(IvrWORX ^threadCtx);

			virtual ~RtspSession(void);

			virtual ApiErrorCode Setup(IN  String ^rtsp_url, IN  AbstractOffer ^offer);

			virtual ApiErrorCode Play(double start_time, double duration, double scale);

			virtual ApiErrorCode Pause();

			virtual ApiErrorCode TearDown();

			virtual AbstractOffer^ RemoteOffer();

			virtual AbstractOffer^ LocalOffer();

		protected:

			ivrworx::RtspSession *_impl;

		};
	}
}