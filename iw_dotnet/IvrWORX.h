#pragma once
#include "AutoNative.h"

namespace ivrworx
{
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



