#pragma once
#include "SipCall.h"

using namespace std;

namespace ivrworx
{
	
	class MscmlCall:
		public SipMediaCall
	{
	public:

		MscmlCall(IN ScopedForking &forking, IN HandleId handle_id);

		virtual ~MscmlCall(void);

		virtual ApiErrorCode MakeCall(
			IN const string &destination_uri, 
			IN const CnxInfo &local_media, 
			IN csp::Time ring_timeout)
		{
			throw new exception("unsupported");
		};

	private:
		

	};

	typedef shared_ptr<MscmlCall>
	MscmlCallPtr;

}

