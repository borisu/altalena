#pragma once
#include "ResipCall.h"

using namespace std;

namespace ivrworx
{
	
	class ResipMscmlCall:
		public ResipMediaCall
	{
	public:

		ResipMscmlCall(IN ScopedForking &forking, IN HandleId handle_id);

		virtual ~ResipMscmlCall(void);

		virtual ApiErrorCode MakeCall(
			IN const string &destination_uri, 
			IN const CnxInfo &local_media, 
			IN csp::Time ring_timeout)
		{
			throw new exception("unsupported");
		};

	private:
		

	};

	typedef shared_ptr<ResipMscmlCall>
	MscmlCallPtr;

}

