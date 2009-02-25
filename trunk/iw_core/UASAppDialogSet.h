#pragma once

using namespace resip;
using namespace std;

namespace ivrworx
{
	class UASAppDialogSet : public AppDialogSet
	{
	public:

		UASAppDialogSet(DialogUsageManager& dum, Data SampleAppData);

		virtual ~UASAppDialogSet(void);

		virtual AppDialog* createAppDialog(const SipMessage& msg);

		virtual SharedPtr<UserProfile> selectUASUserProfile(const SipMessage& msg);

	};

}




