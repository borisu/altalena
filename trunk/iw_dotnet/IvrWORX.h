#pragma once
#include "AutoNative.h"

using namespace ivrworx;

namespace ivrworx
{
namespace interop
{
	public enum ApiErrorCode
	{
		API_SUCCESS = 0,
		API_FAILURE,
		API_SOCKET_INIT_FAILURE,
		API_TIMER_INIT_FAILURE,
		API_SERVER_FAILURE,
		API_TIMEOUT,
		API_WRONG_PARAMETER,
		API_WRONG_STATE,
		API_HANGUP,
		API_UNKNOWN_DESTINATION,
		API_FEATURE_DISABLED,
		API_UNKNOWN_RESPONSE,
		API_PENDING_OPERATION
	};

	public ref class IvrWORX 
	{
	public:

		IvrWORX();

		void Init(String ^confName);

		~IvrWORX();

	private:

		long _threadId;

		ConfigurationPtr  *_conf;

		FactoryPtrList * _factoriesList;

		HandlePairList * _procHandles ;

		HandlesVector  * _shutdownHandles ;

		RunningContext *_ctx;

		ScopedForking  *_forking;

		bool _isDisposed;


	};

}
}



