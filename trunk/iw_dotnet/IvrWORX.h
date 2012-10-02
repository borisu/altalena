#pragma once
#include "AutoNative.h"

namespace ivrworx
{
	public ref class IvrWORX
	{
	public:

		IvrWORX(String ^confName);

		~IvrWORX();

	private:

		long _threadId;

		CAutoNativePtr<ivrworx::ConfigurationPtr>  _configuration;

		CAutoNativePtr<shared_ptr<ScopedForking>>  _forking;

		CAutoNativePtr<shared_ptr<FactoryPtrList>> _factoryList;

		CAutoNativePtr<shared_ptr<LpHandlePair>>   _pair;

		CAutoNativePtr<shared_ptr<HandlePairList>> _procHandles;

		CAutoNativePtr<shared_ptr<HandlesVector>> _shutdownHandles;



	};

}



